/* ----------------------------------------------------------------------------- 
 * connections.c
 *
 * iPDC - Phasor Data Concentrator
 *
 * Copyright (C) 2011-2012 Nitesh Pandit
 * Copyright (C) 2011-2012 Kedar V. Khandeparkar
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * Authors: 
 *		Nitesh Pandit <panditnitesh@gmail.com>
 *		Kedar V. Khandeparkar <kedar.khandeparkar@gmail.com>			
 *
 * ----------------------------------------------------------------------------- */

#define _GNU_SOURCE
 
#include  <stdio.h>
#include  <stdlib.h>
#include  <unistd.h>
#include  <errno.h>
#include  <string.h>
#include  <sys/types.h>
#include  <sys/socket.h>
#include  <netinet/in.h>
#include  <arpa/inet.h>
#include  <sys/wait.h>
#include  <signal.h>
#include  <pthread.h>

#include  "connections.h"
#include  "parser.h"
#include  "global.h"
#include  "new_pmu_or_pdc.h"
#include "apps.h"
#include "applications.h"

/* ---------------------------------------------------------------------*/
/*                  Functions defined in connections.c          	*/
/* ---------------------------------------------------------------------*/

/*                 1.  void  setup()           	    	      		*/
/*                 2.  void* UL_udp()          	    	      		*/
/*                 3.  void* UL_tcp()            	    		*/
/*                 4.  void* UL_tcp_connection()	 		*/
/*                 5.  void  PMU_process_UDP()		               	*/
/*                 6.  void  PMU_process_TCP()                     	*/
/*                 7.  void  sigchld_handler()       	              	*/

/* -------------------------------------------------------------------- */


/* ---------------------------------------------------------------- */
/*                         global variables                         */
/* ---------------------------------------------------------------- */

int yes = 1; 	/* argument to setsockopt */
char display_buf[200];


/* ----------------------------------------------------------------------------	*/
/* FUNCTION  setup():                                	     			*/
/* It creates two threads by calling tcp() and udp() in each thread. 		*/
/* ----------------------------------------------------------------------------	*/


void setup(){

	/* ---------------------------------------------------------------- */
	/*        Initialize Global Mutex Variable from global.h            */
	/* ---------------------------------------------------------------- */

	pthread_mutex_init(&mutex_cfg, NULL);
	pthread_mutex_init(&mutex_status_change, NULL);
	pthread_mutex_init(&mutex_Lower_Layer_Details, NULL);
	pthread_mutex_init(&mutex_Upper_Layer_Details, NULL);
	pthread_mutex_init(&mutex_on_TSB, NULL);
	pthread_mutex_init(&mutex_on_thread,NULL);

	CMDSYNC[0] = 0xaa;
	CMDSYNC[1] = 0x41;
	CMDSYNC[2] = '\0';

	CMDCFGSEND[0] = 0x00;
	CMDCFGSEND[1] = 0x05;
	CMDCFGSEND[2] = '\0'; 

	CMDDATASEND[0] = 0x00;
	CMDDATASEND[1] = 0x02;
	CMDDATASEND[2] = '\0'; 

	CMDDATAOFF[0] = 0x00;
	CMDDATAOFF[1] = 0x01;
	CMDDATAOFF[2] = '\0'; 

	DATASYNC[0] = 0xaa;
	DATASYNC[1] = 0x01;
	DATASYNC[2] = '\0';

	CFGSYNC[0] = 0xaa;
	CFGSYNC[1] = 0x31;
	CFGSYNC[2] = '\0';

	finalFramFlag = 0;
	
	cfgframe = dataframe = NULL;
	cfgfirst = cfgfirst_DDS2 = NULL;

	int err;

	/* Create UDP socket and bind to port */

	if ((UL_UDP_sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {

		perror("socket");
		exit(1);

	} else {

//		printf("UDP Socket:Sucessfully created\n");

	} 	

	if (setsockopt(UL_UDP_sockfd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int)) == -1) {
		perror("setsockopt");
		exit(1);
	}

	UDP_my_addr.sin_family = AF_INET;            // host byte order
	UDP_my_addr.sin_port = htons(UDPPORT);       // short, network byte order
	UDP_my_addr.sin_addr.s_addr = INADDR_ANY;    // automatically fill with my IP
	memset(&(UDP_my_addr.sin_zero),'\0', 8);     // zero the rest of the struct

	if (bind(UL_UDP_sockfd, (struct sockaddr *)&UDP_my_addr,
			sizeof(struct sockaddr)) == -1) {
		perror("bind");
		exit(1);
	} else {

//		printf("UDP Socket Bind :Sucessfull\n");
	} 


	/* Created socket and bound to port */
	/* Create TCP socket and bind and listen on port */

	if ((UL_TCP_sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		exit(1);
	} else {

//		printf("TCP Socket:Sucessfully created\n");
	}

	if (setsockopt(UL_TCP_sockfd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int)) == -1) {
		perror("setsockopt");
		exit(1);
	}

	TCP_my_addr.sin_family = AF_INET;          // host byte order
	TCP_my_addr.sin_port = htons(TCPPORT);     // short, network byte order
	TCP_my_addr.sin_addr.s_addr = INADDR_ANY;  // automatically fill with my IP
	memset(&(TCP_my_addr.sin_zero), '\0', 8);  // zero the rest of the struct

	if (bind(UL_TCP_sockfd, (struct sockaddr *)&TCP_my_addr, sizeof(struct sockaddr))
			== -1) {
		perror("bind");
		exit(1);

	} else {

//		printf("TCP Socket Bind :Sucessfull\n");
	}

	if (listen(UL_TCP_sockfd, BACKLOG) == -1) {

		perror("listen");
		exit(1);

	} else {

//		printf("TCP Listen :Sucessfull\n");
	}

	sa.sa_handler = sigchld_handler; // reap all dead processes
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}

	/* TCP created socket and is litening for connections */

	/* Create UDP socket for DB Server and bind to DBPORT */
	if ((DB_sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {

		perror("socket");
		exit(1);

	} else {

		//printf("DB Socket:Sucessfully created\n");

	} 	
	if (setsockopt(DB_sockfd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int)) == -1) {
		perror("setsockopt");
		exit(1);
	}

	bzero(&DB_Server_addr,sizeof(DB_Server_addr));
	DB_Server_addr.sin_family = AF_INET;            // host byte order
	DB_Server_addr.sin_port = htons(DBPORT);       // short, network byte order
	DB_Server_addr.sin_addr.s_addr = inet_addr(dbserver_ip);    // automatically fill with my IP
	memset(&(DB_Server_addr.sin_zero),'\0', 8);     // zero the rest of the struct


	//printf("\nUDP Listening on port %d for command frames from Upper PDC\n",UDPPORT);
	//printf("\nTCP Listening on port %d for command frames from Upper PDC\n",TCPPORT);
	//printf("\nPort %d for Sending the data frames for archival from iPDC\n\n",DBPORT);

	UL_TCP_sin_size = sizeof(struct sockaddr_in);
	UL_UDP_addr_len = sizeof(struct sockaddr);
	//DB_addr_len = sizeof(struct sockaddr);


	/* Threads are created for UDP and TCP to listen on port 6001 and 6000 respectively in default attr mode*/
	if((err = pthread_create(&UDP_thread,NULL,UL_udp,NULL))) { 

		perror(strerror(err));
		exit(1);	
	}

	if((err = pthread_create(&TCP_thread,NULL,UL_tcp,NULL))) {

		perror(strerror(err));
		exit(1);	
	}  
	
	if(SPDCApp == true){
		/* Thread created for APIs() function */
		pthread_t t;
		if((err = pthread_create(&t,NULL,APIs,NULL))) { 
			perror(strerror(err));
			exit(1);	
		}   
	}
	else if(LPDCApp == true){
		/* Thread created for APIs() function */
		pthread_t t;
		if((err = pthread_create(&t,NULL,LPDC_APIs,NULL))) { 
			perror(strerror(err));
			exit(1);	
		}   
	}
}


/* ----------------------------------------------------------------------------	*/
/* FUNCTION  UL_udp():                                	     			*/
/* Handles upper layer PDC command frames			 		*/
/* ----------------------------------------------------------------------------	*/

void* UL_udp(){

	// KK Design

	/* 1. Design and handle the requests from SPDCs and Peer PDCs that indicate model updates and model violations respectively
	   2. see if the updated model from SPDC is received. Accordingly set the flags and update in the MapPMUToPhasor the alpha and averagefreq.
	   3. See if the peer PDC sends the model violation trigger
	   4. Maintain soc and fracsec of the latest PMU frames dispatched that can identify if the data is in the database
	   5. Create data frame from the data fetched from in - Memory db if the data has already been dispatched from iPDC
	*/

	/* UDP data Received */
	while(1) {
		unsigned char* UL_udp_command = (unsigned char*)malloc(MAXBUFLEN* sizeof(unsigned char)); //My Change

		memset(UL_udp_command,'\0',MAXBUFLEN); //My Change
		
/*		memset(UL_udp_command,'\0',19);*/
		memset(display_buf,'\0',200);

		if ((numbytes = recvfrom(UL_UDP_sockfd,UL_udp_command,MAXBUFLEN-1, 0,(struct sockaddr *)&UL_UDP_addr, (socklen_t *)&UL_UDP_addr_len)) == -1) { 
			// Main if
			perror("recvfrom");
			exit(1);

		} else { /* New datagram has been received */
/*printf("here\n");*/
			int pdc_flag = 0;
			pthread_mutex_lock(&mutex_Upper_Layer_Details);
			struct Upper_Layer_Details *temp_pdc = ULfirst;	
			
			if(ULfirst == NULL) {
/*printf("here1\n");*/
				pdc_flag = 0;				

			} else  {

				while(temp_pdc != NULL ) {
/*printf("here2\n");*/
					if((!strcmp(temp_pdc->ip,inet_ntoa(UL_UDP_addr.sin_addr))) && 
							(!strncasecmp(temp_pdc->protocol,"UDP",3)) && (temp_pdc->port == UDPPORT)) {

						pdc_flag = 1;		
						break;
					} else {

						temp_pdc = temp_pdc->next;
					}									
				}  												
			}

			if(pdc_flag){ 
/*printf("here3\n");*/
				unsigned char c = UL_udp_command[1];
				c <<= 1;
				c >>= 5;	
				temp_pdc->sockfd = UL_UDP_sockfd;

				if(c  == 0x04) { /* Check if it is a command frame from Upper PDC */ 
/*printf("here4\n");*/
					//printf("\nCommand frame Received at iPDC.\n");
					c = UL_udp_command[15];
/*					printf("c = %x\n",c);*/
					if((c & 0x06) == 0x06){ // Received a Query
						if(LPDCApp == true){
							
							struct PMUQueryInfo* temp_query = (struct PMUQueryInfo*)malloc(sizeof(struct PMUQueryInfo));
							unsigned char * index = (unsigned char *)UL_udp_command;
							index += 2; // Skip SYNC
						
							unsigned char framesize[3];
							copy_cbyc(framesize,index,2);
							index += 2; // Skip Framesize
				
							framesize[2] = '\0';
							int frsize = to_intconvertor((unsigned char*)framesize);
						
							unsigned char mpmuid[3];
							copy_cbyc(mpmuid,index,2);
							index += 2;
							mpmuid[2] = '\0';
							temp_query->pmuid = to_intconvertor((unsigned char*)mpmuid);
	
							index += 4; // Skip soc
							index += 4; // Skip fracsec
							index += 2; // Skip Cmdcode
						
							unsigned char apiid[3];
							copy_cbyc(apiid,index,2);
							index += 2;
							apiid[2] = '\0';
							temp_query->api_id = to_intconvertor((unsigned char*)apiid);
						
							unsigned char qid[3];
							copy_cbyc(qid,index,2);
							index += 2;
							qid[2] = '\0';
							temp_query->query_id = to_intconvertor((unsigned char*)qid);
							
/*printf("pmuid = %d, apiid = %d, queryid = %d\n",temp_query->pmuid,temp_query->api_id,temp_query->query_id);				*/
							
							unsigned char invoke_soc[5];
							copy_cbyc(invoke_soc,index,4);
							index += 4;
							invoke_soc[4] = '\0';
							temp_query->invoke_soc = to_long_int_convertor((unsigned char*)invoke_soc);
						
							unsigned char invoke_fracsec[5];
							copy_cbyc(invoke_fracsec,index,4);
							index += 4;
							invoke_fracsec[4] = '\0';
							temp_query->invoke_fracsec = to_long_int_convertor((unsigned char*)invoke_fracsec);
							
							copy_cbyc(temp_query->phasorname,index,16);
							index += 16;
							(temp_query->phasorname)[16] = '\0';
											
							temp_query->phasorNumber = -1;
								
							struct cfg_frame *temp_cfg = cfgfirst;
	
							// Check for the IDCODE in Configuration Frame
							while(temp_cfg != NULL){
/*printf("idcode = %d\n",to_intconvertor(temp_cfg->idcode));*/
								if(temp_query->pmuid == to_intconvertor(temp_cfg->idcode)) {
/*printf("here\n");	*/
									int phnmr = to_intconvertor(temp_cfg->pmu[0]->phnmr);
/*printf("phnmr = %d\n",phnmr);	*/
									int i;
									for(i = 0;i<phnmr;i++){
/*printf("(%s) == (%s)\n",temp_cfg->pmu[0]->cnext->phnames[i],temp_query->phasorname);*/
										if(!strcmp(temp_cfg->pmu[0]->cnext->phnames[i],temp_query->phasorname)){
/*printf("inside\n");*/
											temp_query->phasorNumber = i;
											break;
										}
									}
									break;
								} else {

									temp_cfg = temp_cfg->cfgnext;
								}
							}
						
							temp_query->spdc = temp_pdc;
							
							c = UL_udp_command[14];

							if((c & 0x01) == 0x01){ // Received a common Query for result
/*								printf("Received a common Query for result\n");*/
								
/*								printf("fsize = %d, numbyte = %d, Monitored PMUID = %d, SOC = %ld, FRACSEC = %ld, APIid = %d, QueryId = %d, Phasorname = %s, PhasorNo. = %d. \n",frsize,numbytes, temp_query->pmuid,temp_query->invoke_soc,temp_query->invoke_fracsec,temp_query->api_id,temp_query->query_id,temp_query->phasorname,temp_query->phasorNumber);*/
								
/*								writeTimeToLog(5,temp_query->pmuid,temp_query->invoke_soc,temp_query->invoke_fracsec);*/
								pthread_attr_t attr;
								pthread_attr_init(&attr);
								int err;
								/* In  the detached state, the thread resources are immediately freed when it terminates, but
								   pthread_join(3) cannot be used to synchronize on the thread termination. */
								if((err = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED))) {

									perror(strerror(err));
									exit(1);
								}																																										     	  
								/* Shed policy = SCHED_FIFO (realtime, first-in first-out) */
								if((err = pthread_attr_setschedpolicy(&attr,SCHED_FIFO))) {

									perror(strerror(err));		     
									exit(1);
								}  

								pthread_t t;
								if((err = pthread_create(&t,&attr,execute_query,(void *)temp_query))) {
									perror(strerror(err));		     
									exit(1);
								}
							
							}
							else{	// Received a filter Query
/*								printf("Received a Filter Query\n");*/
														
								unsigned char slope[5];
								copy_cbyc(slope,index,4);
								index += 4;
								slope[4] = '\0';
								temp_query->bounds_slope = c2f_ieee((unsigned char*)slope);
						
								unsigned char dlb[5];
								copy_cbyc(dlb,index,4);
								index += 4;
								dlb[4] = '\0';
								temp_query->displace_lb = c2f_ieee((unsigned char*)dlb);

								unsigned char dub[5];
								copy_cbyc(dub,index,4);
								index += 4;
								dub[4] = '\0';
								temp_query->displace_ub = c2f_ieee((unsigned char*)dub);
												
								char key[16];
								sprintf(key,"%d",temp_query->pmuid);
/*	printf("second pmuid = %d, apiid = %d, queryid = %d\n",temp_query->pmuid,temp_query->api_id,temp_query->query_id);*/
/*	printf("fsize = %d, numbyte = %d, Monitored PMUID = %d, SOC = %ld, FRACSEC = %ld, APIid = %d, QueryId = %d, Phasorname = %s, PhasorNo. = %d, boundslope= %f, displace_lb= %f, displace_ub=%f. \n",frsize,numbytes, temp_query->pmuid,temp_query->invoke_soc,temp_query->invoke_fracsec,temp_query->api_id,temp_query->query_id,temp_query->phasorname,temp_query->phasorNumber,temp_query->bounds_slope,temp_query->displace_lb,temp_query->displace_ub);*/
	
								updateQueryList(key ,temp_query, &hashForPMUQuery);
		
						
						

							}
						}
					}
						
					else if((c & 0x05) == 0x05){ //Send CFg frame to PDC
/*printf("here5\n");*/
						printf("\nCommand frame for CFG Received\n");

						while(root_pmuid != NULL); // Wait till all the status change has been cleared

/*						printf("sockfd = %d,ipaddress = %s\n",temp_pdc->sockfd,inet_ntoa(temp_pdc->pdc_addr.sin_addr));*/

						if(temp_pdc->address_set == 0) {
/*printf("here6\n");*/
							memcpy(&temp_pdc->pdc_addr,&UL_UDP_addr,sizeof(UL_UDP_addr));

						}
						//numbytes = create_cfgframe(cfgfirst);
						//if(angleDiffApp == true && DDS == 3 && LPDCApp == true) {
						if(LPDCApp == true) {
/*printf("here7\n");*/
							numbytes = create_cfgframe(cfgfirst);
						}

						if ((numbytes = sendto (temp_pdc->sockfd,cfgframe, numbytes, 0,
								(struct sockaddr *)&temp_pdc->pdc_addr,sizeof(temp_pdc->pdc_addr)) == -1)) {
/*printf("here8\n");*/
							perror("sendto");

						} else {

							printf("Sent iPDC Configuration Frame\n");
						}
						free(cfgframe);

						temp_pdc->UL_upper_pdc_cfgsent = 1;
						temp_pdc->config_change = 0;

					} else if((c & 0x02) == 0x02) { // if data frame 

						if(temp_pdc->UL_upper_pdc_cfgsent == 1) { // Only if cfg is sent send the data	
/*printf("here9\n");*/
							temp_pdc->UL_data_transmission_off = 0;

						} else {

							printf("Data cannot be sent as CMD for CFG not received\n");

						}			

					} else if ((c & 0x01) == 0x01){

						temp_pdc->UL_data_transmission_off = 1;

					}				  

				} else { /* If it is a frame other than command frame */

					printf("Not a command frame\n");						
				}


			} else { /* If the command frame is not from authentic PDC*/

				printf("Command frame from un-authentic PDC\n");
			}

		} // Main if ends	
		
		free(UL_udp_command);//My change
		pthread_mutex_unlock(&mutex_Upper_Layer_Details);
	} // while ends		
} 

/* ----------------------------------------------------------------------------	*/
/* FUNCTION  PMU_process_UDP():                                	     		*/
/* This function processes the frames as per their type( data, config).	 	*/
/* The received frames are from Lower Layer PMU/PDC on UDP.		 	*/
/* ----------------------------------------------------------------------------	*/

void PMU_process_UDP(unsigned char *udp_buffer,struct sockaddr_in PMU_addr,int sockfd){

	int stat_status;
	unsigned int id;
	unsigned char id_char[2];

	id_char[0] = udp_buffer[4];
	id_char[1] = udp_buffer[5];
	id = to_intconvertor(id_char);

	unsigned char c = udp_buffer[1];
	c <<= 1;
	c >>= 5;
/*	if(c == 0x03)
	printf("Config recvd in conn\n");
	else
		("Data recvd in conn\n");

	*/
	if(c == 0x00){ 							/* If data frame */
		if(LPDCApp == true){
			stat_status = dataparser(udp_buffer);

			/* Change in cfg frame is handled */
			if((stat_status == 10)||(stat_status == 14)) {

				unsigned char *cmdframe = malloc(19);
				cmdframe[18] = '\0';
				create_command_frame(1,id,(char *)cmdframe);

				if (sendto(sockfd,cmdframe,18, 0,
						(struct sockaddr *)&PMU_addr,sizeof(PMU_addr)) == -1)
					perror("sendto");
				free(cmdframe);						

			} else if (stat_status == 15) { 			/* Data Invalid */

				printf("Data Invalid\n");

			}
		}else if(SPDCApp == true){
			c = udp_buffer[14];						/* If filtered data frame */
//printf("c = [%x] [%x]\n", udp_buffer[14],udp_buffer[15]);
/*printf("I Got\n");*/
			if(c == 0x07){
				//printf("I Got some thing\n");
				c = udp_buffer[15];
				if(c & 0x01 == 0x01){
					//printf("Sent modal creator request %d\n",fake++);
					model_creater(udp_buffer); // Get required data from second LPDC
/*					pthread_mutex_unlock(&mutex_on_thread);*/
				}
				else{
					initiate_create_model(udp_buffer); // Voilation occure on one LPDC
				//printf("Out of initiate\n");
/*					pthread_mutex_unlock(&mutex_on_thread);*/
				}
			} else if(c == 0x08){
				//printf("Partial Compute result..\n");
				partial_computation_aggregate(udp_buffer);
			} else if(c == 0x09){
				//printf("DSE Partial Compute result..\n");
				DSE_aggregate(udp_buffer);
			}
			else if(c == 0x00){
				//printf("my app4\n");

				struct timeval tm;
				gettimeofday(&tm, NULL);

				Analysing(73,0,tm.tv_sec,tm.tv_usec);

			}
			

		
		}
	} else if(c == 0x03) { 						/* If configuration frame */

		//printf("\nConfiguration frame received for ID: %d conn.c.\n",id);
		cfgparser(udp_buffer);

		unsigned char *cmdframe = malloc(19);
		cmdframe[18] = '\0';
		create_command_frame(2,id,(char *)cmdframe);
/*		printf("\nReturn from create_command_frame\n");*/

		/* Command frame sent to send the data frames */
		
		if (sendto(sockfd,cmdframe, 18, 0,
				(struct sockaddr *)&PMU_addr,sizeof(PMU_addr)) == -1)
			perror("sendto");
		free(cmdframe);
/*		pthread_mutex_unlock(&mutex_on_thread); // Added by KK on 19-Oct-2013*/

	} else {	

		printf("Erroneous frame\n");

	}	
	fflush(stdout);
} 


























/*************** TCP Case Didn't handled yet. **************/

/* ----------------------------------------------------------------------------	*/
/* FUNCTION  UL_tcp():                                	     			*/
/* It Handles Upper Layer PDC connections.					*/
/* ----------------------------------------------------------------------------	*/

void* UL_tcp() {

	int err;

	// A new thread is created for each TCP connection in 'detached' mode. Thus allowing any number of threads to be created. 
	pthread_attr_t attr;
	pthread_attr_init(&attr);

	if((err = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED))) { // In  the detached state, the thread resources are

		// immediately freed when it terminates, but 
		perror(strerror(err));	                                        // pthread_join(3) cannot be used to synchronize
		exit(1);							//  on the thread termination.	       
	}																																		    

	if((err = pthread_attr_setschedpolicy(&attr,SCHED_FIFO))) { // Shed policy = SCHED_FIFO (realtime, first-in first-out)

		perror(strerror(err));		     
		exit(1);
	}  

	int sin_size,new_fd,pdc_flag = 0;

	while (1) {

		sin_size = sizeof(struct sockaddr_in);

		if (((new_fd = accept(UL_TCP_sockfd, (struct sockaddr *)&UL_TCP_addr,
				(socklen_t *)&sin_size)) == -1)) { // main if starts
			perror("accept");

		} else { /* New TCP connection has been received*/ 

			pthread_mutex_lock(&mutex_Upper_Layer_Details);							
			struct Upper_Layer_Details *temp_pdc = ULfirst;
			if(ULfirst == NULL) {

				pdc_flag = 0;				

			} else {

				while(temp_pdc != NULL ) {

					if((!strcmp(temp_pdc->ip,inet_ntoa(UL_TCP_addr.sin_addr))) && 
							(!strncasecmp(temp_pdc->protocol,"TCP",3)) && (temp_pdc->port == TCPPORT)) {

						pdc_flag = 1;		
						break;
					} else {

						temp_pdc = temp_pdc->next;
					}									
				}  												
			}
			if(pdc_flag) {

				temp_pdc->sockfd = new_fd;				
				pthread_t t;

				/* PDC is authentic. Send the command frame for cfg frame */
				printf("server: got connection from %s\n",
						inet_ntoa(temp_pdc->pdc_addr.sin_addr));

				/* Creates a new thread for each TCP connection. */
				if((err = pthread_create(&t,&attr,UL_tcp_connection,(void *)temp_pdc))) {

					perror(strerror(err));		     
					exit(1);
				}				

			} else { /* If PMU ip is not in the ipdcINFO.bin */

				printf("Request from %s TCP which is un-authentic\n",
						inet_ntoa(UL_TCP_addr.sin_addr));			
			}	
			pthread_mutex_unlock(&mutex_Upper_Layer_Details);									

		} // main if ends	

	} // While ends

	pthread_attr_destroy(&attr);
}


/* ----------------------------------------------------------------------------	*/
/* FUNCTION  UL_tcp_connection():                                     		*/
/* It handles command frames from upper layer PDC on TCP	.		*/
/* ----------------------------------------------------------------------------	*/

void* UL_tcp_connection(void * temp_pdc) {

	struct Upper_Layer_Details *udetails = (struct Upper_Layer_Details *) temp_pdc;
	int UL_new_fd = udetails->sockfd;
	udetails->thread_id = pthread_self();

	while(1) {

		memset(UL_tcp_command,19,0);	
		int bytes_read = recv(UL_new_fd,UL_tcp_command,18,0);
		if(bytes_read == -1) {

			perror("recv");
			udetails->tcpup = 0;
			pthread_exit(NULL);

		} else if(bytes_read == 0){

			printf("The Client connection exit.\n");
			udetails->tcpup = 0;
			pthread_exit(NULL);

		} else {

			pthread_mutex_lock(&mutex_Upper_Layer_Details);							
			unsigned char c = UL_tcp_command[1];
			c <<= 1;
			c >>= 5;

			if(c  == 0x04) {	/* Check if it is a command frame from Upper PDC*/			

//				printf("Command frame Received\n"); // Need to further check if the command is for cfg or data
				c = UL_tcp_command[15];

				if((c & 0x05) == 0x05){ //Send CFg frame to PDC

					while(root_pmuid != NULL); // Wait till the staus chage list becomes empty
					numbytes = create_cfgframe(cfgfirst);
					
					udetails->tcpup = 1;

					if (send(UL_new_fd,cfgframe,numbytes, 0)== -1)
						perror("send");						
					free(cfgframe);

					udetails->UL_upper_pdc_cfgsent = 1;
					udetails->config_change = 0;

				} else if((c & 0x02) == 0x02) {

					if(udetails->UL_upper_pdc_cfgsent == 1) { // Only if cfg is sent send the data

						udetails->UL_data_transmission_off = 0;

					} else {

						printf("Data cannot be sent as CMD for CFG not received\n");

					}			

				} else if ((c & 0x01) == 0x01){ // Put the data transmission off

					udetails->UL_data_transmission_off = 1;
				}
			} else { /* If it is a frame other than command frame */

				printf("Not a command frame\n");						
			}

			pthread_mutex_unlock(&mutex_Upper_Layer_Details);							
		}        		

	} // while
	close(UL_new_fd);
	pthread_exit(NULL);
}

/* ----------------------------------------------------------------------------	*/
/* FUNCTION  PMU_process_TCP():                                	     		*/
/* This function processes the frames as per their type( data, config).	 	*/
/* The received frames are from Lower Layer PMU/PDC on TCP.		 	*/
/* ----------------------------------------------------------------------------	*/

void PMU_process_TCP(unsigned char tcp_buffer[],int sockfd) {

	int stat_status;
	unsigned int id;
	unsigned char id_char[2];

	id_char[0] = tcp_buffer[4];
	id_char[1] = tcp_buffer[5];
	id = to_intconvertor(id_char);

	unsigned char c = tcp_buffer[1];
	c <<= 1;
	c >>= 5;

	if(c == 0x00){ 							/* If data frame */

		stat_status = dataparser(tcp_buffer);

		/* Handle the Stat word */
		if((stat_status == 10)||(stat_status == 14)) {

			unsigned char *cmdframe = malloc(19);
			cmdframe[18] = '\0';
			create_command_frame(1,id,(char *)cmdframe);

			if (send(sockfd,cmdframe,18, 0)== -1)
				perror("send");	
			free(cmdframe);					

		} else if (stat_status == 15) { 			/* Data Invalid */

			printf("Data Invalid\n");
		}

	} else if(c == 0x03) { 						/* If configuration frame */

//		printf("\nConfiguration frame received.\n");
		cfgparser(tcp_buffer);
		unsigned char *cmdframe = malloc(19);
		cmdframe[18] = '\0';
		create_command_frame(2,id,(char *)cmdframe);
//		printf("Return from create_command_frame().\n");

		/* Command frame sent to send the data frames */
		if (send(sockfd,cmdframe,18, 0)== -1)
			perror("send");
		free(cmdframe);

	} else {	

		printf("\nErroneous frame\n");
	}	
	fflush(stdout);
} 


/* ----------------------------------------------------------------------------	*/
/* FUNCTION  sigchld_handler():                                	     		*/
/* ----------------------------------------------------------------------------	*/

void sigchld_handler(int s) {
	while(wait(NULL) > 0);
}

/**************************************** End of File *******************************************************/
