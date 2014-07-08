#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <pthread.h>
#include <errno.h>
#include <math.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/signal.h>
#include <unistd.h>
#include "connection.h"
#include "function.h"
#include "iPMU.h"

void* sender(void * info)
{
	struct sender_data* inf = (struct sender_data*)info;
	int index, start, end;
	index = inf->index;
	start = inf->start;
	end = inf->end;
	
	long esoc,efsec;
	int indx;
	unsigned char temp4[4];
	int key;
	struct timeval tm;
	
	while(1){
/*	printf("here1\n");*/
	
		pthread_cond_wait(&sending[index],&mutex_sending[index]);
/*	printf("here2\n");*/
/*int count = 0;*/
		for(key = start; key < end; key++){
			gettimeofday(&tm, NULL);
			esoc = tm.tv_sec;
			efsec = tm.tv_usec;
			
			indx = data_frame_size[key];
			
			li2c(esoc,temp4);
			B_copy(data_frame[key],temp4,indx,4);
			indx = indx + 4;
		     	
			li2c(efsec,temp4);
			B_copy(data_frame[key],temp4,indx,4);
			int k;
	
/*			printf("%d %d\n",pmuids[key],candidates_count[key]);*/
	
			for(k=0; k<candidates_count[key]; k++){
				if (sendto(UDP_sockfd[key],data_frame[key],data_frame_size[key]+8, 0,(struct sockaddr *)&UDP_addresses[key][k],sizeof(UDP_addresses[key][k])) == -1){
					perror("sendto");
				} else {
				//printf("Send Successfully.. %d Realbytes = %d\n",pmuids[key],data_frame_size[key]);
//printf("%ld %ld\n",esoc,efsec);
/*count ++;*/
				}
			}
			
/*			if(candidates_count[key]>1)*/
/*				printf("%d\n",key+1);*/
		}
/*		printf("%d\n",count);*/
/*		break;*/

/*while(1);*/
/*		gettimeofday(&tm, NULL);*/
/*		*/
/*		printf("%ld %ld\n",tm.tv_sec,tm.tv_usec);		*/
/*		printf("%d %d\n",start,end);*/
/*		for(key = start; key< end; key++)*/
/*			generate_data_frame(key);*/

//break;
	}

}

void* SEND_DATA()
{
	int swadeshcount = 0;
	printf("Starting Data Transmission...\n");
     /* Calculate the waiting time during sending data frames */
	int data_waiting, i=0;
	struct timeval tm;

	data_waiting = 1e9/cfg_infos[0]->cfg_dataRate; // Assume all PMU have same datarate

   	struct timespec *cal_timeSpec, *cal_timeSpec1;
   	cal_timeSpec = malloc(sizeof(struct timespec));
   	cal_timeSpec1 = malloc(sizeof(struct timespec));

     /* int clock_gettime(clockid_t clk_id, struct timespec *tp);
        The functions clock_gettime() and retrieve and set the time of the specified clock clk_id.
        CLOCK_REALTIME : System-wide realtime clock. Setting this clock requires appropriate privileges.
     */
	
	clock_gettime(CLOCK_REALTIME, cal_timeSpec);
/*gettimeofday(&tm, NULL);*/
/*printf("swa..1 %ld %ld\n",tm.tv_sec,tm.tv_usec);*/

    	while(1)
    	{
        	clock_gettime(CLOCK_REALTIME, cal_timeSpec1);
        	usleep(1);
        	clock_gettime(CLOCK_REALTIME, cal_timeSpec);
		
        	if (cal_timeSpec->tv_sec > cal_timeSpec1->tv_sec)
        	{
            		fsecNum = 1;
            		break;
        	}
    	} // End of while-1
    	
/*gettimeofday(&tm, NULL);*/
/*printf("swa..2 %ld %ld\n",tm.tv_sec,tm.tv_usec);*/

	
	while(1)
	{
        	if (i != 0)
        	{
               		cal_timeSpec->tv_nsec += data_waiting;
        	}
        	else
        	{
            		cal_timeSpec->tv_nsec = data_waiting;
        	}
        	if ((cal_timeSpec->tv_nsec) >= 1e9)
        	{
		    	cal_timeSpec->tv_sec++;
		    	cal_timeSpec->tv_nsec-=1e9;
        	}
		
		/* Call the function generate_data_frame() to create a fresh new Data Frame */
	
		usleep(10000);
		int key;
		for(key = 0; key< pmu_count; key++)
				generate_data_frame(key);
/*printf("here\n");*/
/*		if(swadeshcount == 1)*/
/*			while(1);*/


/*gettimeofday(&tm, NULL);*/
/*long int s = tm.tv_sec;*/
/*long int f = tm.tv_usec;*/
/*		*/
/*		*/

/*gettimeofday(&tm, NULL);*/
/*printf("now %ld %ld\n",tm.tv_sec,tm.tv_usec);*/

          /* int clock_nanosleep(clockid_t clock_id, int flags, const struct timespec *request, struct timespec *remain);
              clock_nanosleep - high resolution sleep with specifiable clock (ADVANCED REALTIME). Link with -lrt.
              The suspension time caused by this function may be longer than requested because the argument value is rounded
              up to an integer multiple of the sleep resolution, or because of the scheduling of other activity by the system.
          */
		clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, cal_timeSpec, cal_timeSpec1);
/*gettimeofday(&tm, NULL);*/
/*printf("after %ld %ld\n",tm.tv_sec,tm.tv_usec);*/
/*gettimeofday(&tim, NULL);*/
/*long esoc = tim.tv_sec;*/
/*long efsec = tim.tv_usec;*/
/*printf("After : SOC =  %ld, FRACSEC =  %ld\n",esoc,efsec);*/

		int k;
		for(k=0; k<div_count; k++)
			pthread_cond_signal(&sending[k]);

/*long esoc,efsec;*/
/*	int indx;*/
/*	unsigned char temp4[4];*/
/*	struct timeval tm;*/

/*			for(key = 0; key < pmu_count; key++){*/
/*			gettimeofday(&tm, NULL);*/
/*			esoc = tm.tv_sec;*/
/*			efsec = tm.tv_usec;*/
/*			*/
/*			indx = data_frame_size[key];*/
/*			*/
/*			li2c(esoc,temp4);*/
/*			B_copy(data_frame[key],temp4,indx,4);*/
/*			indx = indx + 4;*/
/*		     	*/
/*			li2c(efsec,temp4);*/
/*			B_copy(data_frame[key],temp4,indx,4);*/
/*			int k;*/
/*	*/
/*	*/
/*			for(k=0; k<candidates_count[key]; k++){*/
/*				if (sendto(UDP_sockfd[key],data_frame[key],data_frame_size[key]+8, 0,(struct sockaddr *)&UDP_addresses[key][k],sizeof(UDP_addresses[key][k])) == -1){*/
/*					perror("sendto");*/
/*				} else {*/
/*//					printf("Send Successfully.. %d Realbytes = %d\n",pmuids[key],data_frame_size[key]);*/
/*///printf("%ld %ld\n",esoc,efsec);*/
/*				}*/
/*			}*/
/*		}*/


/*gettimeofday(&tm, NULL);*/
/*		printf("%ld %ld\n",tm.tv_sec,tm.tv_usec);		*/

/*		swadeshcount++;*/
		i++;
		clock_gettime(CLOCK_REALTIME, cal_timeSpec1);

	} // End of while-2
} // End of SEND_DATA


void* UDP_PMU(void* i)
{
	unsigned char c;
	char udp_command[18];
	
	int * key = (int *)i;

	int index = *key;
	free(i);
	int numbytes;
	
/*	UDP_addr_len[index] = sizeof(UDP_addr[index]);*/

     /* This while is always in listening mode to receiving frames from a PDC and their respective reply */
	while(1)	
	{
		memset(udp_command,'\0',18);

          /* New datagram has been received */
		
		if ((numbytes = recvfrom(UDP_sockfd[index], udp_command, 18, 0, (struct sockaddr *)&(UDP_addresses[index][candidates_count[index]]), (socklen_t *)&(UDP_addr_lengths[index][candidates_count[index]]))) == -1)
		{ 
			perror("recvfrom");
			exit(1);
		}
		else{	
			c = udp_command[1];
			c <<= 1;
			c >>= 5;

			if(c  == 0x04) 		/* Check if it is a command frame from PDC? */ 
			{
				c = udp_command[15];

				if(((c & 0x05) == 0x05) || ((c & 0x04) == 0x04))		/* If Command frame for Configuration Frame-2/1? */
				{
					//printf("\nCommand Frame Received : Configuration Frame-2/1.\n"); 

					//printf("from port %d and address %s\n",ntohs(UDP_addr[index].sin_port), inet_ntoa(UDP_addr[index].sin_addr));

					if (sendto(UDP_sockfd[index],cfg_frame[index],cfg_frame_size[index], 0,(struct sockaddr *)&UDP_addresses[index][candidates_count[index]],sizeof(UDP_addresses[index][candidates_count[index]])) == -1) {

						perror("sendto");
					}
					candidates_count[index] ++;
					
					//udp_send_status = 0;
					//printf("\niPMU CFG-2/1 frame [of %d Bytes] is sent to the PDC.\n", cfg_frame_size[index]);
					
					
				}
				else if((c & 0x02) == 0x02)		/* Command frame for Turn ON transmission request from PDC */
				{ 				
					//printf("\nCommand Frame Received : Turn ON data transmission.");

				     	pthread_mutex_lock(&mutex_data);
						send_cfg_count++;
						printf("send_cfg_count = %d\n",send_cfg_count);
						if(send_cfg_count == dup_total_buses)
							pthread_cond_signal(&cond);
						pthread_mutex_unlock(&mutex_data);

					//printf(" --> Data is ON.\n");
/*					pthread_exit(NULL);*/
				}
				else
				{
					printf("\nCan't recognize the received Command Frame!\n");						
					continue;				
				}	
			}	
			else
			{
				printf("\nReceived Frame is not a command frame! Try Again.\n");						
				continue;				
			}	
		}
	}

	
} // End of UDP_PMU

/* ----------------------------------------------------------------------------	*/
/* void server(int id, int uport, int tport);							*/
/* ----------------------------------------------------------------------------	*/

void server(int id, int uport, int tport, int index)
{

	int err;
   	int udp_port = uport;
   	int tcp_port = tport;
	int yes = 1; 	/* argument to setsockopt */

//	printf("\n\t\t|-------------------------------------------------------|\n");      
//	printf("\t\t|\t\t Welcom to iPMU SERVER\t\t\t|\n");      
//	printf("\t\t|-------------------------------------------------------|\n");      

/*	printf("iPMU ID Code : %d\n", id);*/

	/* Create UDP socket and bind to port */
	if ((UDP_sockfd[index] = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {

		perror("socket");
		exit(1);

	} else {

	//	printf("\nUDP Socket\t\t: Sucessfully Created\n");
	} 	

	if (setsockopt(UDP_sockfd[index],SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int)) == -1) {

		perror("setsockopt");
		exit(1);
	}

	UDP_my_addr[index].sin_family = AF_INET;            	// host byte order
	UDP_my_addr[index].sin_port = htons(udp_port);       	// short, network byte order
	UDP_my_addr[index].sin_addr.s_addr = htonl(INADDR_ANY);    	// automatically fill with my IP
	memset(&(UDP_my_addr[index].sin_zero),'\0', 8);     	// zero the rest of the struct

	if (bind(UDP_sockfd[index], (struct sockaddr *)&UDP_my_addr[index], sizeof(struct sockaddr)) == -1) {

		perror("bind");
		exit(1);

	} else {

	//	printf("UDP Socket Bind\t\t: Sucessfull\n");
	} 

	/* UDP created socket and is litening for connections */
//	printf("UDP Listening Port\t: %d\n\n",udp_port);


	//printf("\niPMU Server now listening for new connection:\n");

/*	UDP_addr_len[index] = sizeof(struct sockaddr);*/
	
	
	
	int k;
	for(k=0; k<Max_dup_allowed; k++){
		UDP_addr_lengths[index][k] = sizeof(struct sockaddr);
	}
	
	
	
	
	

	/* Threads are created for UDP and TCP to listen on ports given by user */
	pthread_t UDP_thread;
	
	pthread_attr_t attr;
	pthread_attr_init(&attr);

	if((err = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED))) {

		perror(strerror(err));
		exit(1);
	}																																										     	  
	if((err = pthread_attr_setschedpolicy(&attr,SCHED_FIFO))) {

		perror(strerror(err));		     
		exit(1);
	}
			  		
	int* ind = (int*)malloc(sizeof(int));
	*ind = index;

	if((err = pthread_create(&UDP_thread,&attr,UDP_PMU,(void*)ind))) 		{
		perror(strerror(err));
		exit(1);	
	}


	pthread_join(UDP_thread, NULL);


} /* end of main */


/*************************************** End of Program ***********************************************/ 
