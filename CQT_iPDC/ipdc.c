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
				if(!strncmp(d1,"DDS2",3)) {// Super PDC running DDS 2 // Need to set a flag

					DDS = 2;
					SPDCApp = true;					
					IamSPDC = true;
					printf("I am SPDC \n");
					buildAppHashTables(); //Added by KK on 20/11/2013:19.53 

				} 
				 
			} else {

				d1 = strtok (NULL,","); 
				if(!strncmp(d1,"DDS2",3)) {// Need to set a flag
					
					DDS = 2;
					LPDCApp = true;
					IamLPDC = true;
					printf("I am LPDC \n");
					buildAppHashTables(); //Added by KK on 20/11/2013:19.53 
				}				
			}

			pthread_mutex_lock(&mutex_Analysis);
			// For Analysis of Aggregated Data dispatch and Distributed Partial Computation
			first_arrival_lpdc = 9999999;
			Min_pmutolpdc = 9999999;
			Min_pmutolpdcdelay_sum = 0;
			count_patial_computation = 0;
			computation_time_sum = 0;
			appComputeTime = 0;
			dataFrameCreateTime = 0;
			flagApps = 0;
			
			pthread_mutex_unlock(&mutex_Analysis);

			
			/* setup() call to stablish the connections at iPDC restart */
			setup();		
			
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
						else
							d1 = strtok (l1,","); 

						d2 = strtok (NULL,","); 

						d3 = strtok (NULL,","); 

						d4 = strtok (NULL,","); 

						/* call add_PMU() to actual add pmu/pdc and start communication */ 
						ret = add_PMU(d1,d2,d3,d4);

/*						if(ret == 0)
							printf("Source Device Successfully Added. For ID: %s, IP: %s, PORT: %s, and Protocol:%s.\n",d1,d2,d3,d4);
						else
							printf("Device details already exists! For ID: %s, IP: %s, PORT: %s, and Protocol:%s.\n",d1,d2,d3,d4);
*/					}
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

/*						if(ret == 0)
							printf("Destination Device Successfully Added. For IP: %s, and Protocol:%s.\n",d1,d2);
						else
							printf("Device details already exists! For IP: %s, and Protocol:%s.\n",d1,d2);
*/					}
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
//		printf("\niPDC CSV file is not present in the system! iPDC Exiting...\n\n");		
	}
	
	pthread_join(UDP_thread, NULL);
	pthread_join(TCP_thread, NULL);
	pthread_join(p_thread, NULL);

	close(UL_UDP_sockfd);
	close(UL_TCP_sockfd);
	
	return 0;
}


/**************************************** End of File *******************************************************/
