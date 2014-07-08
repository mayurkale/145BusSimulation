/* ----------------------------------------------------------------------------- 
 * parser.c
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


/* ------------------------------------------------------------------------------------ */
/*                       Functions defined in parser.c          	   	 	*/
/* -------------------------------------------------------------------------------------*/

/*	1. void cfgparser(char [])           		    	      				*/
/*	3. void dataparser(char[])          	   		     	      			*/
/*	4. int  check_statword(char stat[])      	         	      			*/
/*	5. void add_id_to_status_change_list(unsigned char idcode[]); 				*/
/*	6. void remove_id_from_status_change_list(unsigned char idcode[]) 			*/
/*	7. unsigned int to_intconvertor(unsigned char [])          				*/
/*	8. void long_int_to_ascii_convertor (long int n,unsigned char hex[]);			*/					
/*	9. void int_to_ascii_convertor(unsigned int n,unsigned char hex[]);			*/
/*	10.void copy_cbyc(unsigned char dst[],unsigned char *s,int size)			*/
/*	11.int  ncmp_cbyc(unsigned char dst[],unsigned char src[],int size)			*/
/*	12.void byte_by_byte_copy(unsigned char dst[],unsigned char src[],int index,int n)	*/
/*	13.unsigned long int to_long_int_convertor(unsigned char array[])			*/
/*	14.uint16_t compute_CRC(unsigned char *message,int length)				*/

/* ------------------------------------------------------------------------------------ */

#define _GNU_SOURCE

#include  <stdio.h>
#include  <string.h>
#include  <stdlib.h> 
#include  <stdint.h>
#include  <math.h>
#include <assert.h>
#include  <float.h>
#include <search.h> // Hash search
#include  "parser.h"
#include  "global.h" 
#include  "dallocate.h" 
#include  "connections.h"
#include "apps.h"
#include "dds.h"

/* ----------------------------------------------------------------------------	*/
/* FUNCTION  cfgparser():                                	     		*/
/* It creates configuration objects for the received configuration frames.	*/
/* Configuration frame is also written in the file `cfg.bin`.			*/
/* If the object is already present, it will replace in cfg_frame LL and	*/
/* also in the file `cfg.bin` by calling 					*/
/* ----------------------------------------------------------------------------	*/

void cfgparser(unsigned char st[]){ 

	pthread_mutex_lock(&mutex_cfg);
	
	unsigned char *s;
	unsigned char sync[3];
	unsigned int num_pmu,phn,ann,dgn;
	int i,j,dgchannels,match = 0;

	struct cfg_frame *cfg;
	struct channel_names *cn;
	unsigned int dataFrameSize = 0;

	/******************** PARSING BEGINGS *******************/

	cfg = malloc(sizeof(struct cfg_frame));

	if(!cfg) {

		printf("No enough memory for cfg\n");
	}

//	printf("Inside cfgparser()\n");
	s = st;

	/* Memory Allocation Begins - Allocate memory to framesize */
	cfg->framesize = malloc(3*sizeof(unsigned char));
	if(!cfg->framesize) {

		printf("No enough memory for cfg->framesize\n");
	}

	// Allocate memory to idcode
	cfg->idcode = malloc(3*sizeof(unsigned char));
	if(!cfg->idcode) {
		printf("No enough memory for cfg->idcode\n");
	}

	// Allocate memory to soc
	cfg->soc = malloc(5*sizeof(unsigned char));
	if(!cfg->soc) {
		printf("Not enough memory for cfg->soc\n");
	}

	// Allocate memory to fracsec
	cfg->fracsec = malloc(5*sizeof(unsigned char));
	if(!cfg->fracsec) {
		printf("Not enough memory for cfg->fracsec\n");
	}

	// Allocate memory to time_base
	cfg->time_base = malloc(5*sizeof(unsigned char));
	if(!cfg->time_base) {
		printf("Not enough memory for cfg->time_base\n");
	}

	// Allocate memory to time_base
	cfg->time_base = malloc(5*sizeof(unsigned char));
	if(!cfg->time_base) {
		printf("No enough memory for cfg->time_base\n");
	}

	// Allocate memory to num_pmu
	cfg->num_pmu = malloc(3*sizeof(unsigned char));
	if(!cfg->num_pmu) {
		printf("No enough memory for cfg->num_pmu\n");
	}

	// Allocate memory to data_rate
	cfg->data_rate = malloc(3*sizeof(unsigned char));
	if(!cfg->data_rate) {
		printf("No enough memory for cfg->data_rate\n");
	}

	//Copy sync word to file
	copy_cbyc(sync,(unsigned char *)s,2);
	sync[2] = '\0';
	s = s + 2;

	// Separate the FRAME SIZE
	copy_cbyc(cfg->framesize,(unsigned char *)s,2);
	cfg->framesize[2] = '\0';
	unsigned int framesize;
	framesize = to_intconvertor(cfg->framesize);
	s = s + 2;	

	//SEPARATE IDCODE
	copy_cbyc(cfg->idcode,(unsigned char *)s,2);
	cfg->idcode[2] = '\0';
	int id = to_intconvertor(cfg->idcode);
//	printf("ID Code %d\n",id);
	s = s + 2;

	/**** Remove the id from the list of Stat change if it is present ****/
	remove_id_from_status_change_list(cfg->idcode);

	//SEPARATE SOC	
	copy_cbyc(cfg->soc,(unsigned char *)s,4);
	cfg->soc[4] = '\0';
	s =s + 4;

	//SEPARATE FRACSEC	
	copy_cbyc(cfg->fracsec,(unsigned char *)s,4);
	cfg->fracsec[4] = '\0';
	s = s + 4;

	//SEPARATE TIMEBASE
	copy_cbyc (cfg->time_base,(unsigned char *)s,4);
	cfg->time_base[4]='\0';
	s = s + 4;

	//SEPARATE PMU NUM
	copy_cbyc (cfg->num_pmu,(unsigned char *)s,2);
	cfg->num_pmu[2] = '\0';
	s = s + 2;

	num_pmu = to_intconvertor(cfg->num_pmu);
	pmuCount++;
//	printf("Number of PMU's %d\n",num_pmu);

	// Allocate Memeory For Each PMU
	cfg->pmu = malloc(num_pmu* sizeof(struct for_each_pmu *));
	if(!cfg->pmu) {
		printf("Not enough memory for pmu[][]\n");
		exit(1);
	}

	for (i = 0; i < num_pmu; i++) {
		cfg->pmu[i] = malloc(sizeof(struct for_each_pmu));
	}

	j = 0;

	///WHILE EACH PMU IS HANDLED
	while(j<num_pmu) {

		// Memory Allocation for stn
		cfg->pmu[j]->stn = malloc(17*sizeof(unsigned char));
		if(!cfg->pmu[j]->stn) {
			printf("Not enough memory cfg->pmu[j]->stn\n");
			exit(1);
		}

		// Memory Allocation for idcode
		cfg->pmu[j]->idcode = malloc(3*sizeof(unsigned char));
		if(!cfg->pmu[j]->idcode) {
			printf("Not enough memory cfg->pmu[j]->idcode\n");
			exit(1);
		}

		// Memory Allocation for format
		cfg->pmu[j]->data_format = malloc(3*sizeof(unsigned char));
		if(!cfg->pmu[j]->data_format) {
			printf("Not enough memory cfg->pmu[j]->data_format\n");
			exit(1);
		}

		// Memory Allocation for phnmr
		cfg->pmu[j]->phnmr = malloc(3*sizeof(unsigned char));
		if(!cfg->pmu[j]->phnmr) {
			printf("Not enough memory cfg->pmu[j]->phnmr\n");
			exit(1);
		}

		// Memory Allocation for annmr
		cfg->pmu[j]->annmr = malloc(3*sizeof(unsigned char));
		if(!cfg->pmu[j]->annmr) {
			printf("Not enough memory cfg->pmu[j]->annmr\n");
			exit(1);
		}

		// Memory Allocation for dgnmr
		cfg->pmu[j]->dgnmr = malloc(3*sizeof(unsigned char));
		if(!cfg->pmu[j]->dgnmr) {
			printf("Not enough memory cfg->pmu[j]->dgnmr\n");
			exit(1);
		}

		// Memory Allocation for fnom
		cfg->pmu[j]->fnom = malloc(3*sizeof(unsigned char));
		if(!cfg->pmu[j]->fnom) {
			printf("Not enough memory cfg->pmu[j]->fnom\n");
			exit(1);
		}

		// Memory Allocation for cfg_cnt
		cfg->pmu[j]->cfg_cnt = malloc(3*sizeof(unsigned char));
		if(!cfg->pmu[j]->cfg_cnt) {
			printf("Not enough memory cfg->pmu[j]->cfg_cnt\n");
			exit(1);
		}

		//SEPARATE STATION NAME
		copy_cbyc (cfg->pmu[j]->stn,(unsigned char *)s,16);
		cfg->pmu[j]->stn[16] = '\0';
		s = s + 16;

		//SEPARATE IDCODE		
		copy_cbyc (cfg->pmu[j]->idcode,(unsigned char *)s,2);
		cfg->pmu[j]->idcode[2] = '\0';
		//printf("ID = %d\n",to_intconvertor(cfg->pmu[j]->idcode));
		s = s + 2;

		//SEPARATE DATA FORMAT		
		copy_cbyc (cfg->pmu[j]->data_format,(unsigned char *)s,2);
		cfg->pmu[j]->data_format[2]='\0';
		s = s + 2;

		//SEPARATE PHASORS	
		copy_cbyc (cfg->pmu[j]->phnmr,(unsigned char *)s,2);
		cfg->pmu[j]->phnmr[2]='\0';
		phn = to_intconvertor(cfg->pmu[j]->phnmr);
		s = s + 2;

		//SEPARATE ANALOGS			
		copy_cbyc (cfg->pmu[j]->annmr,(unsigned char *)s,2);
		cfg->pmu[j]->annmr[2]='\0';
		ann = to_intconvertor(cfg->pmu[j]->annmr);
		s = s + 2;

		//SEPARATE DIGITALS			
		copy_cbyc (cfg->pmu[j]->dgnmr,(unsigned char *)s,2);
		cfg->pmu[j]->dgnmr[2]='\0';
		dgn = to_intconvertor(cfg->pmu[j]->dgnmr);
		s = s + 2; 


		unsigned char hex = cfg->pmu[j]->data_format[1];
		hex <<= 4;

		// Extra field has been added to identify polar,rectangular,floating/fixed point	
		dataFrameSize += 2; // For stat word

		cfg->pmu[j]->fmt = malloc(sizeof(struct format));
		
		if((hex & 0x80) == 0x80) {
		
			cfg->pmu[j]->fmt->freq = '1'; 
			dataFrameSize += 8; // freq and dfreq
		
		} else {
		
			cfg->pmu[j]->fmt->freq = '0';
			dataFrameSize += 4;  //freq and dfreq
		}
		
		if((hex & 0x40) == 0x40 ) {
		
			cfg->pmu[j]->fmt->analog = '1'; 
			dataFrameSize += ann*4;

		} else {
		
			cfg->pmu[j]->fmt->analog = '0';
			dataFrameSize += ann*2;
		}
		
		if((hex & 0x20) == 0x20) {
		
			cfg->pmu[j]->fmt->phasor = '1'; 
			dataFrameSize += phn*8;
		
		} else {

			cfg->pmu[j]->fmt->phasor = '0';
			dataFrameSize += phn*4;
		}
		
		if((hex & 0x10) == 0x10) {
		
			cfg->pmu[j]->fmt->polar =  '1'; 		

		} else {
			
			cfg->pmu[j]->fmt->polar = '0';
		}

		dataFrameSize += dgn*2;

		cn = malloc(sizeof(struct channel_names));
		cn->first = NULL;

		ENTRY entry;
		ENTRY *found1=NULL,*found2 = NULL;			

		////SEPARATE PHASOR NAMES 
		if(phn != 0){

			cn->phnames = malloc(phn*sizeof(unsigned char*));

			if(!cn->phnames) {
				printf("Not enough memory cfg->pmu[j]->cn->phnames[][]\n");
				exit(1);
			}

			for (i = 0; i < phn; i++) {

				cn->phnames[i] = malloc(17*sizeof(unsigned char));
			}

			i = 0;	//Index for PHNAMES

			int idd = to_intconvertor(cfg->pmu[j]->idcode);
			if(angleDiffApp == true) {

				memset(pmuInfoKey,'\0',50);

				sprintf(pmuInfoKey,"%d",idd);
        		pmuKey = pmuInfoKey;
				entry.key = pmuKey;				
				hsearch_r( entry,FIND, &found1,&hashForMapPMUToPhasor);								
			}

			bool bothSame = false;

			if(coherencyOfGen && IamLPDC) {

				memset(pmuInfoKey,'\0',50);
				sprintf(pmuInfoKey,"%d",idd);
        		pmuKey = pmuInfoKey;
				entry.key = pmuKey;				
				hsearch_r( entry,FIND, &found2,&cggHashForPhasorIndex);				
				/*if(found2 == NULL)
					printf("NULL HU id = %s\n",pmuKey);*/
			}

			if(found1 != NULL) {

				((struct MapPMUToPhasor *)found1->data)->format = cfg->pmu[j]->data_format[1];
				((struct MapPMUToPhasor *)found1->data)->visited = false;									
				((struct MapPMUToPhasor *)found1->data)->Idcode = to_intconvertor(cfg->pmu[j]->idcode);
					
				while(i<phn){

					copy_cbyc (cn->phnames[i],(unsigned char *)s,16);
					cn->phnames[i][16]='\0';       
					
					int nn = ((struct MapPMUToPhasor *)found1->data)->numOfPhasors;
					int ll = 0;
					while(ll < nn) {

						char *ee = ((struct MapPMUToPhasor *)found1->data)->PhasorName[ll];
						if(!ncmp_cbyc(cn->phnames[i],ee,2)) {

//							printf("match phasor\n");
							
							((struct MapPMUToPhasor *)found1->data)->DDS2_Phasor_Index[ll] = i;
//							printf("bos1\n");	
							((struct MapPMUToPhasor *)found1->data)->PhasorIndex[ll] = i;
//							printf("bos2\n");	
							if(found2 != NULL && IamLPDC) {
//								printf("sos1\n");	
								if (((struct MapCGGPMUID2PhasorIndex *)found2->data)->index == i)
									bothSame = true;
							}	
//							printf("sos2\n");	
							break;
						}
						ll++;
					} 

					s = s + 16;  
					i++;
				}

				if(found2 != NULL && bothSame == false && IamLPDC) {

//					printf("dingo2\n");
					((struct MapCGGPMUID2PhasorIndex *)found2->data)->format = cfg->pmu[j]->data_format[1];

				}
			} else {
				
				while(i<phn){

					copy_cbyc (cn->phnames[i],(unsigned char *)s,16);
					cn->phnames[i][16]='\0';        		
					s = s + 16;  
					i++;
				}
			}
		} 		

		//SEPARATE ANALOG NAMES
		if(ann != 0){

			cn->angnames = malloc(ann*sizeof(unsigned char*));

			if(!cn->angnames) {

				printf("Not enough memory cfg->pmu[j]->cn->phnames[][]\n");
				exit(1);
			}

			for (i = 0; i < ann; i++) {

				cn->angnames[i] = malloc(17*sizeof(unsigned char));
			}


			i=0;	//Index for ANGNAMES

			while(i<ann){

				copy_cbyc (cn->angnames[i],(unsigned char *)s,16);
				cn->angnames[i][16]='\0';
//				printf("ANGNAMES %s\n",cn->angnames[i]);
				s = s + 16;  
				i++;
			}
		}

        int di;
		struct dgnames *q;
		i = 0;	//Index for number of dgwords

		while(i < dgn) {

			struct dgnames *temp1 = malloc(sizeof(struct dgnames));

			temp1->dgn = malloc(16*sizeof(unsigned char *));	
			if(!temp1->dgn) {

				printf("Not enough memory temp1->dgn\n");
				exit(1);
			}

			for (di = 0; di < 16; di++) {

				temp1->dgn[di] = malloc(17*sizeof(unsigned char));
			}

			temp1->dg_next = NULL;

			for(dgchannels = 0;dgchannels < 16; dgchannels++){

				copy_cbyc (temp1->dgn[dgchannels],(unsigned char *)s,16);
				temp1->dgn[dgchannels][16]='\0';
				s += 16;
//				printf("%s\n",temp1->dgn[dgchannels]);
			}

			if(cn->first == NULL){

				cn->first = q = temp1;

			} else {

				while(q->dg_next!=NULL){

					q = q->dg_next;
				}
				q->dg_next = temp1;			       
			}  

			i++;  
		} //DGWORD WHILE ENDS

		cfg->pmu[j]->cnext = cn;//Assign to pointers

		///PHASOR FACTORS
		if(phn != 0){

			cfg->pmu[j]->phunit = malloc(phn*sizeof(unsigned char*));

			if(!cfg->pmu[j]->phunit) {

				printf("Not enough memory cfg->pmu[j]->phunit[][]\n");
				exit(1);
			}

			for (i = 0; i < phn; i++) {

				cfg->pmu[j]->phunit[i] = malloc(5);
			}

			i = 0;

			while(i<phn){ //Separate the Phasor conversion factors

				copy_cbyc (cfg->pmu[j]->phunit[i],(unsigned char *)s,4);
				cfg->pmu[j]->phunit[i][4] = '\0';
				s = s + 4;
				i++;
			}
		}//if for PHASOR Factors ends

		//ANALOG FACTORS
		if(ann != 0){

			cfg->pmu[j]->anunit = malloc(ann*sizeof(unsigned char*));

			if(!cfg->pmu[j]->anunit) {

				printf("Not enough memory cfg->pmu[j]->anunit[][]\n");
				exit(1);
			}

			for (i = 0; i < ann; i++) {

				cfg->pmu[j]->anunit[i] = malloc(5);
			}

			i = 0;

			while(i<ann){ //Separate the Phasor conversion factors

				copy_cbyc (cfg->pmu[j]->anunit[i],(unsigned char *)s,4);
				cfg->pmu[j]->anunit[i][4] = '\0';
				s = s + 4;
				i++;
			}
		} // if for ANALOG Factors ends

		if(dgn != 0){

			cfg->pmu[j]->dgunit = malloc(dgn*sizeof(unsigned char*));

			if(!cfg->pmu[j]->dgunit) {

				printf("Not enough memory cfg->pmu[j]->dgunit[][]\n");
				exit(1);
			}

			for (i = 0; i < dgn; i++) {

				cfg->pmu[j]->dgunit[i] = malloc(5);
			}

			i=0;

			while(i<dgn){ //Separate the Phasor conversion factors

				copy_cbyc (cfg->pmu[j]->dgunit[i],(unsigned char *)s,4);
				cfg->pmu[j]->dgunit[i][4] = '\0';
				s = s + 4;
				i++;
			}
		} //if for Digital Words FActtors ends

		copy_cbyc (cfg->pmu[j]->fnom,(unsigned char *)s,2);
		cfg->pmu[j]->fnom[2]='\0';
		s = s + 2;

		copy_cbyc (cfg->pmu[j]->cfg_cnt,(unsigned char *)s,2);
		cfg->pmu[j]->cfg_cnt[2]='\0';
		s = s + 2;

//		printf("bingo gmail\n");
		//pthread_mutex_lock(&mutex_cfg);
		if(coherencyOfGen == true && angleDiffApp == true && DDS == 2 && IamLPDC == true) {

			if(found1 != NULL)
				cfgparser_For_DDS2(cfg,j,found1,1);

			else if(found2 != NULL) 
				cfgparser_For_DDS2(cfg,j,found2,2);
			

		} else if(angleDiffApp == true && DDS == 2 && IamLPDC == true) { 

			if(found1 != NULL)
				cfgparser_For_DDS2(cfg,j,found1,1);
			
		} else if(coherencyOfGen == true && DDS == 2 && IamLPDC == true) {

			//printf("I am entering cfgparser_For_DDS2\n");
			if(found2 != NULL) 
				cfgparser_For_DDS2(cfg,j,found2,2);
		}


		//pthread_mutex_unlock(&mutex_cfg);

		j++;
	}//While for PMU number ends

//	printf("bingo bingo\n");

	copy_cbyc (cfg->data_rate,(unsigned char *)s,2);
	cfg->data_rate[2] = '\0';
	s += 2;
	cfg->cfgnext = NULL;
	cfg->cfgprev = NULL;
//   	printf("Data Rate %d\n", to_intconvertor(cfg->data_rate));

	/* Adjust the configuration object pointers and Lock the mutex_cfg */
//	pthread_mutex_lock(&mutex_cfg);

	// Index is kept to replace the cfgfirst if it matches
	int index = 0; 
	
		if (cfgfirst == NULL)  { // Main if

		cfgfirst = cfg;		

		max_element_of_PMUInfo = 2000;
		memset(&hashForPMUInfo, 0, sizeof(hashForPMUInfo));
		if( hcreate_r(max_element_of_PMUInfo, &hashForPMUInfo) == 0 ) {
  			perror("hcreate_r");
  			exit(1);
		}		
		pmuInfo = malloc(sizeof(struct PMUInfo));

		unsigned int idcode = to_intconvertor(cfgfirst->idcode);

		pmuInfo = createPMUInfoNode(pmuInfo, idcode,-1,-1,dataFrameSize,0,false);		
        
        COMBINE_DF_SIZE = dataFrameSize;
        memset(pmuInfoKey,'\0',50);
        sprintf(pmuInfoKey,"%d",pmuInfo->Idcode);
        pmuKey = pmuInfoKey;
		hash_add_element_to_PMUInfoTable(pmuKey,&pmuInfo[0],&hashForPMUInfo); 

	} else {

		struct cfg_frame *temp_cfg = cfgfirst,*tprev_cfg;
		tprev_cfg = temp_cfg;

		//Check if the configuration frame already exists
		while(temp_cfg!=NULL){

			if(!ncmp_cbyc(cfg->idcode,temp_cfg->idcode,2)) {

				match = 1;
				break;	

			} else {

				index++;
				tprev_cfg = temp_cfg;
				temp_cfg = temp_cfg->cfgnext;
			}
		}// While ends

		ENTRY entry;
		ENTRY *found;
			
		if(match) {			

			if(!index) {

				// Replace the cfgfirst
				cfg->cfgnext = cfgfirst->cfgnext;				
				free_cfgframe_object(cfgfirst);	
				cfgfirst = cfg;
				
				memset(pmuInfoKey,'\0',50);
				sprintf(pmuInfoKey,"%d",to_intconvertor(cfg->idcode));
        		pmuKey = pmuInfoKey;
				entry.key = pmuKey;
				hsearch_r( entry, FIND, &found, &hashForPMUInfo);
				if(found != NULL) {
					
					COMBINE_DF_SIZE -= ((struct PMUInfo *)found->data)->dataFrameSize;

					((struct PMUInfo *)found->data)->dataFrameSize = dataFrameSize;

					COMBINE_DF_SIZE += dataFrameSize;

					int nextStartIndex = ((struct PMUInfo *)found->data)->startIndex+ dataFrameSize;
					hash_update_element_to_PMUInfoTable(found,nextStartIndex);						

				} else {

					printf("Bingo No ID in the hash table 1\n");
				}

			} else {

				// Replace in between cfg
				tprev_cfg->cfgnext = cfg;
				cfg->cfgprev = tprev_cfg;
				cfg->cfgnext = temp_cfg->cfgnext;	
				free_cfgframe_object(temp_cfg);

				memset(pmuInfoKey,'\0',50);
				sprintf(pmuInfoKey,"%d",to_intconvertor(cfg->idcode));
        		pmuKey = pmuInfoKey;
        		entry.key = pmuKey;				
				hsearch_r( entry, FIND, &found, &hashForPMUInfo);
				
				if(found != NULL) {
				
					COMBINE_DF_SIZE -= ((struct PMUInfo *)found->data)->dataFrameSize;
						
					((struct PMUInfo *)found->data)->dataFrameSize = dataFrameSize; // Add checksum byte Last node
					COMBINE_DF_SIZE += dataFrameSize;

					int nextStartIndex = ((struct PMUInfo *)found->data)->startIndex+ dataFrameSize;
					hash_update_element_to_PMUInfoTable(found,nextStartIndex);	
				
				} else {

					printf("Bingo No ID in the hash table 2\n");	
				}			
			}

		} else { // No match and not first cfg 

			tprev_cfg->cfgnext = cfg;
			cfg->cfgprev = tprev_cfg;

			unsigned int tID = to_intconvertor(tprev_cfg->idcode);
			memset(pmuInfoKey,'\0',50);
			sprintf(pmuInfoKey,"%d",tID);

        	pmuKey = pmuInfoKey;
			entry.key = pmuKey;			
			hsearch_r( entry, FIND, &found, &hashForPMUInfo);
			
			if(found != NULL) {
				
				pmuInfo = malloc(sizeof(struct PMUInfo));

				int idcode = to_intconvertor(cfg->idcode);
				int startIndex = ((struct PMUInfo *)found->data)->startIndex + ((struct PMUInfo *)found->data)->dataFrameSize;		
				pmuInfo = createPMUInfoNode(pmuInfo,idcode,-1,tID,dataFrameSize,startIndex,false);
				//printf("dataFrameSize %d\n",pmuInfo->dataFrameSize);

				COMBINE_DF_SIZE += pmuInfo->dataFrameSize;

				memset(pmuInfoKey,'\0',50);
				sprintf(pmuInfoKey,"%d",pmuInfo->Idcode);
		        pmuKey = pmuInfoKey;

				hash_add_element_to_PMUInfoTable(pmuKey,&pmuInfo[0],&hashForPMUInfo); 				
				
			} /*else {
				printf("bingo ID in hash table 3 but problem\n");
			}*/
		}   
	} //Main if ends
//	printf("cfgparser ends\n");
	pthread_mutex_unlock(&mutex_cfg);
} 


/* ----------------------------------------------------------------------------	*/
/* FUNCTION  dataparser():                                	     		*/
/* Parses the data frames and creates data objects. It searches for config-	*/
/* uration objects that matches with the IDCODE and then creates data objects. 	*/
/* ----------------------------------------------------------------------------	*/

int dataparser(unsigned char data[]) { 

	int match=0,i,j=0;
	int stat_status,config_change = 0;
	unsigned int num_pmu,phnmr,annmr,dgnmr; 
	unsigned char framesize[3],idcode[3],stat[3],outer_stat[3],*d;
	struct cfg_frame *temp_cfg;
	struct data_frame *df;

	d = data;
	d += 2;	// Skip SYN 

	//SEPARATE FRAMESIZE
	copy_cbyc (framesize,d,2);
	framesize[2]='\0';
	d += 2;

	//SEPARATE IDCODE
	copy_cbyc (idcode,d,2);
	idcode[2]='\0';

    unsigned int id = to_intconvertor(idcode);
//    printf("ID = %d\n",id);

	pthread_mutex_lock(&mutex_cfg);

	temp_cfg = cfgfirst;
	long int time_base;

	// Check for the IDCODE in Configuration Frame
	while(temp_cfg != NULL){

		if(!ncmp_cbyc(idcode,temp_cfg->idcode,2)) {

			match = 1;
			time_base = to_long_int_convertor(temp_cfg->time_base);
			break;	

		} else {

			temp_cfg = temp_cfg->cfgnext;
		}
	}
	pthread_mutex_unlock(&mutex_cfg);

	// If idcode matches with cfg idcode
	if(match){

		//Allocate memory for data frame 
		df = malloc(sizeof(struct data_frame));

		if(!df) {

			printf("Not enough memory df\n");
			exit(1);
		}
		df->dnext = NULL;

		// Allocate memory for df->framesize
		df->framesize = malloc(3*sizeof(unsigned char));

		if(!df->framesize) {

			printf("Not enough memory df->idcode\n");
			exit(1);
		}

		// Allocate memory for df->idcode
		df->idcode = malloc(3*sizeof(unsigned char));

		if(!df->idcode) {

			printf("Not enough memory df->idcode\n");
			exit(1);
		}

		// Allocate memory for df->soc
		df->soc = malloc(5*sizeof(unsigned char));

		if(!df->soc) {

			printf("Not enough memory df->soc\n");
			exit(1);
		}

		// Allocate memory for df->fracsec
		df->fracsec = malloc(5*sizeof(unsigned char));

		if(!df->fracsec) {

			printf("Not enough memory df->fracsec\n");
			exit(1);
		}

		// Allocate Memeory For Each PMU
		num_pmu = to_intconvertor(temp_cfg->num_pmu);
		d += 10;  // Kedar on 08-07-13

		df->dpmu = malloc(num_pmu* sizeof(struct data_for_each_pmu *));

		if(!df->dpmu) {

			printf("Not enough memory df->dpmu[][]\n");
			exit(1);
		}

		for (i = 0; i < num_pmu; i++) {

			df->dpmu[i] = malloc(sizeof(struct data_for_each_pmu));
		}

		/* Now start separating the data from data frame - Copy Framesize */
		d -= 12; // Kedar on 08-07-13
		copy_cbyc (df->framesize,d,2);
		df->framesize[2] = '\0';
		//printf("FrameSize = %x ---- %d\n",df->framesize,to_intconvertor(df->framesize));
		d += 2;

		//Copy IDCODE						
		copy_cbyc (df->idcode,idcode,2);	
		//printf("IDOCDE = %x  --- %d\n",idcode,to_intconvertor(idcode));
		df->idcode[2] = '\0';
		d += 2;

		//Copy SOC						
		copy_cbyc (df->soc,d,4);
		df->soc[4] = '\0';
		d += 4;

		//Copy FRACSEC						
		copy_cbyc (df->fracsec,d,4);
		df->fracsec[4] = '\0';
		d += 4;

		// Kedar 22-09-2013
		// unsigned int IDcode = to_intconvertor(df->idcode); 
		// unsigned int l_soc = to_long_int_convertor(df->soc);			
		// unsigned char *fsec;
		// fsec = malloc(3*sizeof(unsigned char));
		// fsec[0] = df->fracsec[1];
		// fsec[1] = df->fracsec[2];
		// fsec[2] = df->fracsec[3];
		// unsigned int l_fracsec = to_long_int_convertor1(fsec);

	   // writeTimeToLog(1,IDcode,l_soc,l_fracsec);
//	    free (fsec);
	
		//Copy NUM PMU
		df->num_pmu = num_pmu;

		// Separate the data for each PMU	    	
		while(j<num_pmu) {		  					


			copy_cbyc (stat,d,2);
			stat[2] = '\0';
			d += 2;	

			// Check Stat Word for each data block 
			stat_status = check_statword(stat);

			if((stat_status == 14)||(stat_status == 10)) {

				//Status for configuration bits have been changed. Add the pmu id to the 'status_change_pmupdcid linked list'.	
				add_id_to_status_change_list(idcode); //idcode = PMU/PDC

			} 	
			
//			printf("J = %d Stat = %d\n",j,stat_status);
			// Extract PHNMR, DGNMR, ANNMR
			phnmr = to_intconvertor(temp_cfg->pmu[j]->phnmr);	
			annmr = to_intconvertor(temp_cfg->pmu[j]->annmr);
			dgnmr = to_intconvertor(temp_cfg->pmu[j]->dgnmr);

			//Allocate memory for stat, Phasors, Analogs,Digitals and Phasors and Frequencies
			/* Memory Allocation Begins */

			// Allocate memory for stat
			df->dpmu[j]->stat = malloc(3*sizeof(unsigned char));

			if(!df->dpmu[j]->stat) {

				printf("Not enough memory for df->dpmu[j]->stat\n");
			}

			df->dpmu[j]->phasors = malloc(phnmr*sizeof(unsigned char *));

			if(!df->dpmu[j]->phasors) {

				printf("Not enough memory df->dpmu[j]->phasors[][]\n");
				exit(1);
			}

			if(temp_cfg->pmu[j]->fmt->phasor == '1') {

				for (i = 0; i < phnmr; i++) 

					df->dpmu[j]->phasors[i] = malloc(9*sizeof(unsigned char));
			} else {

				for (i = 0; i < phnmr; i++) 

					df->dpmu[j]->phasors[i] = malloc(5*sizeof(unsigned char));
			}


			/* For Analogs */

			df->dpmu[j]->analog = malloc(annmr*sizeof(unsigned char *));

			if(!df->dpmu[j]->analog) {

				printf("Not enough memory df->dpmu[j]->analog[][]\n");
				exit(1);
			}

			if(temp_cfg->pmu[j]->fmt->analog == '1') {

				for (i = 0; i < annmr; i++) 

					df->dpmu[j]->analog[i] = malloc(9*sizeof(unsigned char));
			} else {

				for (i = 0; i < annmr; i++) 

					df->dpmu[j]->analog[i] = malloc(5*sizeof(unsigned char));
			}

			/* For Frequency */
			if(temp_cfg->pmu[j]->fmt->freq == '1') {

				df->dpmu[j]->freq = malloc(5*sizeof(unsigned char));
				df->dpmu[j]->dfreq = malloc(5*sizeof(unsigned char));

			} else {

				df->dpmu[j]->freq = malloc(3*sizeof(unsigned char));
				df->dpmu[j]->dfreq = malloc(3*sizeof(unsigned char));
			}

			/* For Digital */
			df->dpmu[j]->digital = malloc(dgnmr* sizeof(unsigned char*));

			if(!df->dpmu[j]->digital) {

				printf("Not enough memory df->dpmu[j]->digital[][]\n");
				exit(1);
			}

			for (i = 0; i < dgnmr; i++) {

				df->dpmu[j]->digital[i] = malloc(3*sizeof(unsigned char));
			}
			/* Memory Allocation Ends */					

			df->dpmu[j]->idcode = to_intconvertor(temp_cfg->pmu[j]->idcode); // Added by KK on 21-11-13

			//Check stat word of each PMU data block
			copy_cbyc (df->dpmu[j]->stat,stat,2);
			df->dpmu[j]->stat[2] = '\0';
			//printf("%d --- %d\n",to_intconvertor(stat),to_intconvertor(df->dpmu[j]->stat));
			memset(stat,'\0',3);

			// Copy FMT
			df->dpmu[j]->fmt = malloc(sizeof(struct format));
			df->dpmu[j]->fmt->freq = temp_cfg->pmu[j]->fmt->freq;
			df->dpmu[j]->fmt->analog = temp_cfg->pmu[j]->fmt->analog;
			df->dpmu[j]->fmt->phasor = temp_cfg->pmu[j]->fmt->phasor;
			df->dpmu[j]->fmt->polar =  temp_cfg->pmu[j]->fmt->polar;

			// Copy num of phasors analogs and digitals	
			df->dpmu[j]->phnmr = phnmr;
			df->dpmu[j]->annmr = annmr;			
			df->dpmu[j]->dgnmr = dgnmr;

			//Phasors 	
			if(temp_cfg->pmu[j]->fmt->phasor == '1') {

				for(i=0;i<phnmr;i++){	

					copy_cbyc (df->dpmu[j]->phasors[i],d,8);
					df->dpmu[j]->phasors[i][8] = '\0';
					d += 8;
				}
			} else {

				for(i=0;i<phnmr;i++){					

					copy_cbyc (df->dpmu[j]->phasors[i],d,4);
					df->dpmu[j]->phasors[i][4] = '\0';
					d += 4;
				}
			}						

			/* For Freq */ 	
			if(temp_cfg->pmu[j]->fmt->freq == '1') {

				copy_cbyc (df->dpmu[j]->freq,d,4);
				df->dpmu[j]->freq[4] = '\0';
				d += 4;					 

				copy_cbyc (df->dpmu[j]->dfreq,d,4);
				df->dpmu[j]->dfreq[4] = '\0';
				d += 4;					 

			} else {

				copy_cbyc (df->dpmu[j]->freq,d,2);
				df->dpmu[j]->freq[2] = '\0';
				d += 2;					 

				copy_cbyc (df->dpmu[j]->dfreq,d,2);
				df->dpmu[j]->dfreq[2] = '\0';
				d += 2;					 
			}						

			/* For Analogs */
			if(temp_cfg->pmu[j]->fmt->analog == '1') {

				for(i = 0; i<annmr; i++){					

					copy_cbyc (df->dpmu[j]->analog[i],d,4);
					df->dpmu[j]->analog[i][4] = '\0';
					d += 4;
				}
			} else {
				for(i = 0; i<annmr; i++){					

					copy_cbyc (df->dpmu[j]->analog[i],d,2);
					df->dpmu[j]->analog[i][2] = '\0';
					d += 2;
				}
			}						

			/* For Digital */
			for(i = 0; i<dgnmr; i++) {

				copy_cbyc (df->dpmu[j]->digital[i],d,2);
				df->dpmu[j]->digital[i][2] = '\0';
				d += 2;
			}

			j++;
		} //While ends

		// Now start Time aligning and Sorting Operation for data_frame df
//		writeTimeToLog(2,IDcode,l_soc,l_fracsec);
		//free_dataframe_object(df);

		pthread_mutex_lock(&mutex_on_TSB);		
		time_align(df);
		pthread_mutex_unlock(&mutex_on_TSB);

	} else {

		//No match for configuration frame
		printf("Configuration is not fresent for received data frame!\n");	
	}  

	if((config_change == 14) ||(config_change == 10)) 
		return config_change;
	else 
		return stat_status;
} 

/* ----------------------------------------------------------------------------	*/
/* FUNCTION  check_statword():                                	     		*/
/* Check the STAT word of the data frames for any change in the data block.	*/
/* Some of the prime errors are handled.				 	*/
/* ----------------------------------------------------------------------------	*/

int check_statword(unsigned char stat[]) { 

	/*
	STAT Word Bits representation:

	Bit-15,14 & 09	=	82	=	Data modified by PDC, for handling missing data frame? 
	Bit-10	=	0400	=	Configuration change, set to 1 for 1 min to advise configuration will change
	Bit-11	=	0800	=	PMU trigger detected, 0 when no trigger.
	Bit-12	=	0400	=	Data sorting, 0 by time stamp, 1 by arrival.
	Bit-13	=	2000	=	PMU sync, 0 when in sync with a UTC traceable time source
	Bit-14 & 15 =  Data error:
						01 =	4000	=	PMU error. including configuration error?.
						10 =	8000	=	PMU in test mode (do not use values) or
									absent data tags have been inserted (do not use values)
						11 =	12000=	PMU error (do not use values)

	*/

	int ret = 0;

	/* For missing data frame? According to 2011 standard! */
	if(stat[0] == 0x82) {	// Bit-15-14 and Bit 09 
							// Changed by Kedar on 4-7-13
		
		printf("Data Frame missing or Data modified by PDC?\n");
		ret = 9;	// earlier 16
		return ret;

	} else if ((stat[0] & 0x04) == 0x04) {	// Bit-10

		printf("Configuration Change error!\n");
		ret = 10;
		return ret;

	} else if ((stat[0] & 0x12) == 0x12) {	// Bit-14 & 15

		printf("PMU error (do not use values)!\n");
		ret = 16;
		return ret;

	} else if ((stat[0] & 0x40) == 0x40) {	// Bit-14

		printf("PMU error may include configuration error\n");
		ret = 14;
		return ret;

	} else if((stat[0] & 0x80) == 0x80) {	// Bit-15

		printf("PMU in test mode or data invalid?\n");
		ret = 15;
		return ret;

	} else if ((stat[0] & 0x20) == 0x20) {	// Bit-13

		printf("PMU Sync error\n");
		ret = 13;
		return ret;

	} else if ((stat[0] & 0x10) == 0x10) {	// Bit-12

		printf("Data sorting changed to by arrival!\n");
		ret = 12;
		return ret;

	} else if ((stat[0] & 0x08) == 0x08) {	// Bit-11

		printf("PMU Trigger detected!\n");
		ret = 11;
		return ret;
	} 

	return ret;
} 


/* ----------------------------------------------------------------------------	*/
/* FUNCTION  add_id_to_status_change_list():                  	     		*/
/* Status of data block has been changed. It adds the IDCODE of the PMU/PDC  	*/
/* from which the data block is received to the status_change_pmupdcid' LL      */
/* ----------------------------------------------------------------------------	*/

void add_id_to_status_change_list(unsigned char idcode[]) { 

	struct status_change_pmupdcid *t;

	t = malloc(sizeof(struct status_change_pmupdcid));

	if(!t) {

		printf("No enough memory for struct (status_change_pmupdcid) t\n");
	}

	copy_cbyc(t->idcode,idcode,2);
	t->idcode[2] = '\0';
	t->pmuid_next = NULL;		

	pthread_mutex_lock(&mutex_status_change);

	if(root_pmuid == NULL) {

		t = root_pmuid;		

	} else {

		struct status_change_pmupdcid *temp = root_pmuid;	
		while(temp->pmuid_next!=NULL) {			
			temp = temp->pmuid_next;
		}	

		temp->pmuid_next = t;						
	}

	pthread_mutex_unlock(&mutex_status_change);
} 


/* ----------------------------------------------------------------------------	*/
/* FUNCTION  remove_id_from_status_change_list():              	     		*/
/* It removes the IDCODE of PMU/PDC from status_change_pmupdcid' LL		*/
/* on receipt of configuration frame of corresponding PMU/PDC		  	*/
/* ----------------------------------------------------------------------------	*/

void remove_id_from_status_change_list(unsigned char idcode[]) { 

	struct status_change_pmupdcid *tprev;

	pthread_mutex_lock(&mutex_status_change);

	if(root_pmuid == NULL) {

		//printf("***No Stat change***\n");

	} else {

		struct status_change_pmupdcid *temp = root_pmuid;	

		while(temp != NULL) {	

			if(!ncmp_cbyc(temp->idcode,idcode,2)) {

				break;		

			} else {

				tprev = temp;		
				temp = temp->pmuid_next;
			}
		}	

		// If It is the first element(root) in the list		
		if(!ncmp_cbyc(temp->idcode,root_pmuid->idcode,2)) {

			root_pmuid =  root_pmuid->pmuid_next;
			free(temp);
			// If it is any element other than first(root) in the list
		} else {

			tprev->pmuid_next = temp->pmuid_next;
			free(temp);			
		}
	}
	pthread_mutex_unlock(&mutex_status_change);
}


/* ----------------------------------------------------------------------------	*/
/* FUNCTION  to_intconvertor():                                	     		*/
/* ----------------------------------------------------------------------------	*/

unsigned int to_intconvertor(unsigned char array[]) {

	unsigned int n;
	n = array[0];
	n <<= 8;
	n |= array[1];
	return n;
}


/* ----------------------------------------------------------------------------	*/
/* FUNCTION  long_int_to_ascii_convertor():                                	*/
/* ----------------------------------------------------------------------------	*/

void long_int_to_ascii_convertor(unsigned long int n,unsigned char hex[]) {

	hex[0] = n >> 24;
	hex[1] = n >> 16;
	hex[2] = n >> 8;
	hex[3] = n ;
}


/* ----------------------------------------------------------------------------	*/
/* FUNCTION  int_to_ascii_convertor(): 		                               	*/
/* ----------------------------------------------------------------------------	*/

void int_to_ascii_convertor(unsigned int n,unsigned char hex[]) {

	hex[0] = n >> 8;
	hex[1] = n ;
}


/* ----------------------------------------------------------------------------	*/
/* FUNCTION  copy_cbyc():                                	     		*/
/* ----------------------------------------------------------------------------	*/

void copy_cbyc(unsigned char dst[],unsigned char s[],int size) {

	int i;
	for(i = 0; i< size; i++) 
	{
		dst[i] = s[i];
		
	}	
	//printf("%x %x",dst,s);
	//printf("\n");
}


/* ----------------------------------------------------------------------------	*/
/* FUNCTION  ncmp_cbyc():                                	     		*/
/* ----------------------------------------------------------------------------	*/

int ncmp_cbyc(unsigned char dst[],unsigned char src[],int size) {

	int i,flag = 0;
	if(dst == NULL)
		printf("DST NULL");
	if(src == NULL)
		printf("SRC NULL");
	for(i = 0; i< size; i++)	{

		if(dst[i] != src[i]) {

			flag = 1;
			break; 	
		}	
	}		
	return flag;
}


/* ----------------------------------------------------------------------------	*/
/* FUNCTION  byte_by_byte_copy():                                	     	*/
/* ----------------------------------------------------------------------------	*/

void byte_by_byte_copy(unsigned char dst[],unsigned char src[],int index,int n) {

	int i;
	for(i = 0;i<n; i++) 
		dst[index + i] = src[i];					
}


/* ----------------------------------------------------------------------------	*/
/* FUNCTION  to_long_int_convertor():                                	     	*/
/* ----------------------------------------------------------------------------	*/

unsigned int to_long_int_convertor(unsigned char array[]) {

	unsigned int n;
	n = array[0];
	n <<= 8;
	n |= array[1];
	n <<= 8;
	n |= array[2];
	n <<= 8;
	n |= array[3];
	return n;
}

/* ----------------------------------------------------------------------------	*/
/* FUNCTION  to_long_int_convertor1():                               	     	*/
/* ----------------------------------------------------------------------------	*/

unsigned int to_long_int_convertor1(unsigned char array[]) {

	unsigned int n;

	n = array[0] <<16;
	n |= array[1] << 8;
	n |= array[2];

	return n;

}

/* ----------------------------------------------------------------------------	*/
/* FUNCTION  compute_CRC():                                	   	  	*/
/* ----------------------------------------------------------------------------	*/

uint16_t compute_CRC(unsigned char *message,int length) {

	uint16_t crc = 0x0ffff,temp,quick;
	int i;

	for(i=0;i<length;i++){
		temp=(crc>>8)^message[i];
		crc<<=8;
		quick=temp ^ ( temp >>4);
		crc ^=quick;
		quick<<=5;
		crc ^=quick;
		quick <<=7;
		crc ^= quick;
	}
	return crc;
}



void Analysing(int index,long int l_soc, long int l_fracsec) {
	pthread_mutex_lock(&mutex_Analysis);
  	int invi_count = 1300;
  	
	struct timeval tm;
	switch(index){
		case 10:// This is for Min N/W delay
			// This will execute for each arrived PMU packet
			// This is Minimum arrivel time
			gettimeofday(&tm, NULL);
			if(first_arrival_lpdc > tm.tv_usec){
				first_arrival_lpdc =  tm.tv_usec;
				Min_pmutolpdc = tm.tv_usec - l_fracsec;
				if(Min_pmutolpdc<0){
					printf("Min_pmutolpdc %ld\n",Min_pmutolpdc);
					Min_pmutolpdc += 1000000;
				}
			}
			
			break;
			
		case 11:// This is for Aggregated Data Transmission
			// This will execte only at the time of aggregated dispatch
			Min_pmutolpdcdelay_sum += Min_pmutolpdc; // Minimum delay from PMU to LPDC
			computation_time_sum += (l_fracsec - first_arrival_lpdc); // Aggregated Dispatch time - First arrival time

			first_arrival_lpdc = 9999999; // Reset the Min value for next tiestamp
			Min_pmutolpdc = 9999999;
			
			break;
			
		case 12:
			gettimeofday(&tm, NULL);
			if(IamLPDC) {

				dataFrameCreateTime += (tm.tv_usec - l_fracsec);
				count_patial_computation ++; // No. of aggregated packets

				if(count_patial_computation > invi_count){
					// Analyse delay and exit
					float AvgComputationTime = (computation_time_sum*1.0 / count_patial_computation);
					float AvgMinDelay_PMUtoLPDC = (Min_pmutolpdcdelay_sum*1.0 / count_patial_computation);
					float AvgTimetoCreateFrame = (dataFrameCreateTime*1.0 / count_patial_computation);

					//printf("computation_time_sum = %ld, Min_pmutolpdcdelay_sum = %ld\n",computation_time_sum,Min_pmutolpdcdelay_sum);
					printf("AvgComputationTime = %f, AvgMinDelay_PMUtoLPDC = %f AvgTimetoCreateFrame %f\n",AvgComputationTime,AvgMinDelay_PMUtoLPDC,AvgTimetoCreateFrame);					
/*					exit(0);*/
					flagApps = 1;
				}

			} else if(IamSPDC) {

				appComputeTime += (tm.tv_usec - l_fracsec);
				count_patial_computation ++; // No. of aggregated packets

				if(count_patial_computation > invi_count){
					// Analyse delay and exit
					float AvgComputationTime = (computation_time_sum*1.0 / count_patial_computation);
					float AvgMinDelay_PMUtoLPDC = (Min_pmutolpdcdelay_sum*1.0 / count_patial_computation);
					float AvgTimeToAppComputeTime = (appComputeTime*1.0 / count_patial_computation);
					printf("AvgComputationTime = %f, AvgMinDelay_PMUtoLPDC = %f AvgTimeToAppComputeTime %f\n",AvgComputationTime,AvgMinDelay_PMUtoLPDC,AvgTimeToAppComputeTime);
					flagApps = 1;					
/*					exit(0);*/
				}
			}
			break;			
		case 13:
			break;
	}		
	pthread_mutex_unlock(&mutex_Analysis);
}

/* ----------------------------------------------------------------------------	*/
/* FUNCTION decode_ieee_single():                                	     	*/
/* ----------------------------------------------------------------------------	*/

float decode_ieee_single(const void *v) {

	const unsigned char *data = v;
	int s, e;
	unsigned long src;
	long f;
	float value;

	src = ((unsigned long)data[0] << 24) |
			((unsigned long)data[1] << 16) |
			((unsigned long)data[2] << 8) |
			((unsigned long)data[3]);

	s = (src & 0x80000000UL) >> 31;
	e = (src & 0x7F800000UL) >> 23;
	f = (src & 0x007FFFFFUL);

	if (e == 255 && f != 0) {
		/* NaN (Not a Number) */
		value = DBL_MAX;

	} else if (e == 255 && f == 0 && s == 1) {
		/* Negative infinity */
		value = -DBL_MAX;
	} else if (e == 255 && f == 0 && s == 0) {
		/* Positive infinity */
		value = DBL_MAX;
	} else if (e > 0 && e < 255) {
		/* Normal number */
		f += 0x00800000UL;
		if (s) f = -f;
		value = ldexp(f, e - 150);
	} else if (e == 0 && f != 0) {
		/* Denormal number */
		if (s) f = -f;
		value = ldexp(f, -149);
	} else if (e == 0 && f == 0 && s == 1) {
		/* Negative zero */
		value = 0;
	} else if (e == 0 && f == 0 && s == 0) {
		/* Positive zero */
		value = 0;
	} else {
		/* Never happens */
		printf("s = %d, e = %d, f = %lu\n", s, e, f);
		assert(!"Woops, unhandled case in decode_ieee_single()");
	}

	return value;
}


/* --------------------------------------------------------------------------------------	*/
/* FUNCTION createPMUInfoNode (struct PMUInfo *  pmuInfo, unsigned int idcode,
								unsigned int nextIdcode,unsigned int prevIdcode,
								int dataFrameSize,int startIndex,bool flag)					*/
/* --------------------------------------------------------------------------------------	*/

struct PMUInfo * createPMUInfoNode (struct PMUInfo *  pmuInfo, unsigned int idcode,
									unsigned int nextIdcode,unsigned int prevIdcode,
									int dataFrameSize,int startIndex,bool flag) {

	pmuInfo->Idcode = idcode;
	pmuInfo->nextIdcode = nextIdcode; // no next node
	pmuInfo->prevIdcode  = prevIdcode; // no previous node 
	pmuInfo->dataFrameSize = dataFrameSize ;//+ 16; 
	pmuInfo->startIndex = startIndex;
	pmuInfo->visited = flag;		

	return pmuInfo;
}

/* --------------------------------------------------------------------------------------	*/
/* FUNCTION hash_add_element_to_PMUInfoTable(
											char * elmt_key,struct PMUInfo elmt_data, 
											struct hsearch_data * table)    				*/
/* --------------------------------------------------------------------------------------	*/

void hash_add_element_to_PMUInfoTable(char * elmt_key,struct PMUInfo *elmt_data, struct hsearch_data * table) {

  ENTRY item;
  ENTRY * ret;
//  struct PMUInfo *elmt_data1 = &elmt_data;

  item.key = strdup(elmt_key);
  item.data = elmt_data;  

  if( hsearch_r(item, ENTER, &ret, table) == 0 ) {
    perror("hsearch_r");
    exit(1);
  }
  return;

}

/* --------------------------------------------------------------------------------------	*/
/* FUNCTION hash_update_element_to_PMUInfoTable (ENTRY *f_item,int nextStartIndex)  						  	*/
/* --------------------------------------------------------------------------------------	*/

void hash_update_element_to_PMUInfoTable (ENTRY *item,int nextStartIndex) {
				
	unsigned int tnextID = ((struct PMUInfo *) item->data)->nextIdcode;
	unsigned int xx;
	xx = nextStartIndex;
	
	ENTRY *found;
	ENTRY entry;	
	
	while(tnextID != -1) {			

		memset(pmuInfoKey,'\0',50);
		sprintf(pmuInfoKey,"%d",tnextID);		
		pmuKey = pmuInfoKey;
		entry.key = pmuKey;

		hsearch_r(entry,FIND,&found,&hashForPMUInfo); 
		if(found != NULL) {				 
				
			((struct PMUInfo *)found->data)->startIndex = xx;				
			xx = ((struct PMUInfo *)found->data)->startIndex + ((struct PMUInfo *)found->data)->dataFrameSize;	
			tnextID = ((struct PMUInfo *)found->data)->nextIdcode;

		} else {
				printf("No item of in this table\n");
			
		}        		
	}
}


/**************************************** End of File *******************************************************/
