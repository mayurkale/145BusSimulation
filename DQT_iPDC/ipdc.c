/* ----------------------------------------------------------------------------- 
 * ipdc.c
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
#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include <search.h>
#include "global.h"
#include "apps.h"
 

/* ---------------------------------------------------------------- */
/*                   main program starts here                       */
/* ---------------------------------------------------------------- */

int main(int argc, char **argv)
{
	if (argc != 2 )
	{
		printf("Passing arguments does not match with the iPDC inputs! Try Again?\n");
		exit(EXIT_SUCCESS);
	}

	int id,i,port,ret;
	int surcCount,destCount;
	char *ptr1,buff[20];
	char *l1,*d1,*d2,*d3,*d4;

	FILE *fp;
	size_t l2=0;
	ssize_t result;

	id = atoi(argv[1]);
    	ptr1 = malloc(30*sizeof(char));
    	memset(ptr1, '\0', 30);

	strcat(ptr1, "iPDC");
	sprintf(buff,"%d",id);
	strcat(ptr1,buff);
	strcat(ptr1, ".csv");
	
	/* Open the iPDC CSV Setup File */
	fp = fopen (ptr1,"r");

	if (fp != NULL)
	{
//		printf("\niPDC CSV file Found - %s\n",ptr1);
		
		/* For The CSV Headers */
		getdelim (&l1, &l2, ('\n'), fp); 
		
		if ((result = getdelim (&l1, &l2, ('\n'), fp)) >0)
		{

			/* For The First column CSV Header */
			d1 = strtok (l1,","); 

			d1 = strtok (NULL,","); 

			PDC_IDCODE = atoi(d1);
			//printf("PDC_ID: %d\n",PDC_IDCODE);
			d1 = strtok (NULL,","); 
			UDPPORT = atoi(d1);

			d1 = strtok (NULL,","); 
			TCPPORT = atoi(d1);

			d1 = strtok (NULL,",");
			memset(dbserver_ip, '\0', 20);
			strcpy(dbserver_ip, (char *)d1);
//			if(!checkip(dbserver_ip))
//				exit(1);
			
			d1 = strtok (NULL,","); 
			surcCount = atoi(d1);
			
			d1 = strtok (NULL,","); 
			destCount = atoi(d1);

		// Commented by Kedar for future use on 12-07-13		
			d1 = strtok (NULL,","); 
			int waitTime = atoi(d1);

			d1 = strtok (NULL,","); 
			if(!strncmp(d1,"YES",3))
			{				
				printf("%s***\n", d1);	
				d1 = strtok (NULL,","); 
				if(!strncmp(d1,"DDS3",3)) {// Super PDC running DDS 3 // Need to set a flag

/*					DDS = 3;*/
					SPDCApp = true;
					//printf("Yeh...++\n");
				}
				else 
				{SPDCApp = false;
					printf("bingo++\n");
				}
			} else {

				d1 = strtok (NULL,","); 
				if(!strncmp(d1,"DDS3",3)) {// Need to set a flag
					
/*					DDS = 3;*/
					LPDCApp = true;
				}
				else
					printf("bingo--%s\n",d1);
			}
			
			
			/* setup() call to stablish the connections at iPDC restart */
			setup();		
			
			// For DSE
			pthread_mutex_init(&mutex_app2_Analysis, NULL);
			pthread_mutex_lock(&mutex_app2_Analysis);
			app2_first_arrival_lpdc = 9999999;
			app2_Min_pmutolpdc = 9999999;
			app2_Min_pmutolpdcdelay_sum = 0;
			app2_DSE_count = 0;
			app2_DSE_time_sum = 0;
			flagapp2 = 0;
			fake=0;
			app2_prviousTS = -1,app2_currentTS = -1;
			app2_sequence = -1;
			memset(&hash_app2latencies, 0, sizeof(hash_app2latencies));
			if( hcreate_r(Datarate, &hash_app2latencies) == 0 ) {
				perror("hcreate_r");
			}
			int k;
			int jump = 1000*1000/Datarate;
			char fseckey[100];
			for(k=0;k<Datarate;k++){
				long int TS = k*jump;
/*				printf("%ld\n",TS);*/
				sprintf(fseckey,"%ld",TS);
				ENTRY item;
				ENTRY * ret;
				struct app2latencies* elmt = (struct app2latencies*)malloc(sizeof(struct app2latencies));
				elmt->first_arrival_lpdc = -1;
				elmt->Min_pmutolpdc = -1;
	
				item.key = strdup(fseckey);
				item.data = elmt;  

				if( hsearch_r(item, ENTER, &ret, &hash_app2latencies) == 0 ) {
					perror("hsearch_r");
					exit(1);
				}
			}
			
			
			pthread_mutex_unlock(&mutex_app2_Analysis);
			
			// For Analysis of Aggregated Data dispatch and Distributed Partial Computation
			pthread_mutex_init(&mutex_Analysis, NULL);
			pthread_mutex_lock(&mutex_Analysis);
			first_arrival_lpdc = 9999999;
			Min_pmutolpdc = 9999999;
			Min_pmutolpdcdelay_sum = 0;
			count_patial_computation = 0;
			computation_time_sum = 0;
			flagapp1 = 0;
			pthread_mutex_unlock(&mutex_Analysis);

			// For Analysis of DQT			
			pthread_mutex_init(&mutex_App6Analysis, NULL);
			pthread_mutex_lock(&mutex_App6Analysis);
			lpdctospdc_latencysum = 0;
			lpdctospdc_pktcount = 0;
			total_pkt_arrivals = 0;

			total_local_violation = 0;
			total_globle_violation = 0;
			total_filtered = 0;
			app6PmuCount = 0;
			app6CurrentPmuCount = 0;
			flagapp6 = 0;
			app6Pmuids = (int*)malloc(surcCount*sizeof(int));
			memset(&hashForAnalysis, 0, sizeof(hashForAnalysis));
			if( hcreate_r(surcCount, &hashForAnalysis) == 0 ) {
				perror("hcreate_r");
			}
			pthread_mutex_unlock(&mutex_App6Analysis);
			
			
			//Mayur change for aplication 7
			pthread_mutex_init(&mutex_App7Analysis, NULL);
			pthread_mutex_lock(&mutex_App7Analysis);
			total_pkt_arrivals7 = 0;//for application 7
			ALL_DATA=false;
			first_arrival_lpdc7=9999999;
			Min_pmutolpdc7=9999999;
			Min_pmutolpdcdelay_sum7=0;
			count_patial_computation7=0;
			//t_cnt=0;
			lpdctospdc_latencysum7=0 ;
			lpdctospdc_pktcount7=0;
			flagapp7=0;
			//app7pktsent=0;
			dataFrameCreateTime=0;
			Min_lpdctospdc7=9999999;
			Min_lpdctospdcdelay_sum7=0;
			pthread_mutex_unlock(&mutex_App7Analysis);

/*			lpdctospdcdelay_sum = 0;*/
			
							
			if (surcCount > 0)
			{
				/* For The CSV Headers */
				getdelim (&l1, &l2, ('\n'), fp); 
				getdelim (&l1, &l2, ('\n'), fp); 
			
				for (i=0; i<surcCount; i++)
				{
					if ((result = getdelim (&l1, &l2, ('\n'), fp)) >0)
					{
						/* For The First column Header */
						if(i == 0)
						{
							d1 = strtok (l1,","); 
							d1 = strtok (NULL,","); 
						}	
						else{
							d1 = strtok (l1,","); 
						}
						

						d2 = strtok (NULL,","); 

						d3 = strtok (NULL,","); 

						d4 = strtok (NULL,","); 

						/* call add_PMU() to actual add pmu/pdc and start communication */ 
						ret = add_PMU(d1,d2,d3,d4);

//						if(ret == 0)
//							printf("Source Device Successfully Added. For ID: %s, IP: %s, PORT: %s, and Protocol:%s.\n",d1,d2,d3,d4);
//						else
//							printf("Device details already exists! For ID: %s, IP: %s, PORT: %s, and Protocol:%s.\n",d1,d2,d3,d4);
				}
				}	
			}
			else
			{
//				printf("\nNo Source Devices mentioned in CSV!\n");
			}

			if (destCount > 0)
			{
				/* For The CSV Headers */
				getdelim (&l1, &l2, ('\n'), fp); 
				getdelim (&l1, &l2, ('\n'), fp); 
			
				for (i=0; i<destCount; i++)
				{
					if ((result = getdelim (&l1, &l2, ('\n'), fp)) >0)
					{
						/* For The First column Header */
						if(i == 0)
						{
							d1 = strtok (l1,","); 
							d1 = strtok (NULL,","); 
						}	
						else
							d1 = strtok (l1,","); 

						d2 = strtok (NULL,","); 

						/* call add_PDC() to add a pdc and start */ 
						ret = add_PDC(d1,d2);
/*
					if(ret == 0)
							printf("Destination Device Successfully Added. For IP: %s, and Protocol:%s.\n",d1,d2);
						else
							printf("Device details already exists! For IP: %s, and Protocol:%s.\n",d1,d2);
			*/	}
				}	
			}
			else
			{
//				printf("\nNo Destination Devices mentioned in CSV!\n");
			}
		}
		else
			exit(1);

		/* Close the iPDC file */
		fclose(fp);		
	}
	else
	{
		printf("\niPDC CSV file is not present in the system! iPDC Exiting...\n\n");		
	}
	
	pthread_join(UDP_thread, NULL);
	pthread_join(TCP_thread, NULL);
	pthread_join(p_thread, NULL);

	close(UL_UDP_sockfd);
	close(UL_TCP_sockfd);
	
	return 0;
}


/**************************************** End of File *******************************************************/
