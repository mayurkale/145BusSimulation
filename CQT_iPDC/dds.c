/* ----------------------------------------------------------------------------- 
 * dds.c
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
#include  <string.h>
#include  <stdlib.h> 
#include <search.h> // Hash search
#include  "parser.h"
#include  "global.h" 
#include "apps.h"
#include "dds.h"

/* --------------------------------------------------------------------------------------	*/
/* FUNCTION void cfgparser_For_DDS2(struct cfg_frame *cfg,int j, 
					ENTRY *found)		 						  	*/
/* --------------------------------------------------------------------------------------	*/

void cfgparser_For_DDS2(struct cfg_frame *cfg,int t, ENTRY *found,int multiFlag) {

//	printf("inside cfgparser_For_DDS2\n");
	unsigned int num_pmu,phn,ann,dgn;
	int i,j,dgchannels,match = 0;

	struct cfg_frame *cfgDDS2;
	struct channel_names *cn;
	unsigned int dataFrameSize_DDS2 = 0;

	/******************** PARSING BEGINGS *******************/

	cfgDDS2 = malloc(sizeof(struct cfg_frame));

	if(!cfgDDS2) {

		printf("No enough memory for cfg\n");
	}

	/* Memory Allocation Begins - Allocate memory to framesize */
	cfgDDS2->framesize = malloc(3*sizeof(unsigned char));
	if(!cfg->framesize) {

		printf("No enough memory for cfgDDS2->framesize\n");
	}

	// Allocate memory to idcode
	cfgDDS2->idcode = malloc(3*sizeof(unsigned char));
	if(!cfgDDS2->idcode) {
		printf("No enough memory for cfgDDS2->idcode\n");
	}

	// Allocate memory to soc
	cfgDDS2->soc = malloc(5*sizeof(unsigned char));
	if(!cfgDDS2->soc) {
		printf("Not enough memory for cfgDDS2->soc\n");
	}

	// Allocate memory to fracsec
	cfgDDS2->fracsec = malloc(5*sizeof(unsigned char));
	if(!cfgDDS2->fracsec) {
		printf("Not enough memory for cfgDDS2->fracsec\n");
	}

	// Allocate memory to time_base
	cfgDDS2->time_base = malloc(5*sizeof(unsigned char));
	if(!cfgDDS2->time_base) {
		printf("Not enough memory for cfgDDS2->time_base\n");
	}

	// Allocate memory to num_pmu
	cfgDDS2->num_pmu = malloc(3*sizeof(unsigned char));
	if(!cfgDDS2->num_pmu) {
		printf("No enough memory for cfgDDS2->num_pmu\n");
	}

	// Allocate memory to data_rate
	cfgDDS2->data_rate = malloc(3*sizeof(unsigned char));
	if(!cfgDDS2->data_rate) {
		printf("No enough memory for cfgDDS2->data_rate\n");
	}

	// Separate the FRAME SIZE
	int nn;
	char *nPAD = malloc(3*sizeof(unsigned char));
	memset(nPAD,'\0',3);	
		
	if(multiFlag == 1)  {
	
		nn = ((struct MapPMUToPhasor *)found->data)->numOfPhasors;	
		unsigned int fsize = 54 + nn*20; //channelnames + conversion factor
		int_to_ascii_convertor(fsize,nPAD);

	} else if(multiFlag == 2) {

		nn = 1;	// AS CGG has only one phasor to be considered 
		unsigned int fsize = 54 + nn*20; //channelnames + conversion factor
		int_to_ascii_convertor(fsize,nPAD);
	} 
		
	copy_cbyc(cfgDDS2->framesize,nPAD,2);
	cfgDDS2->framesize[2] = '\0';
	unsigned int framesize;
	framesize = to_intconvertor(cfgDDS2->framesize);

	//SEPARATE IDCODE
	copy_cbyc(cfgDDS2->idcode,cfg->idcode,2);
	cfgDDS2->idcode[2] = '\0';
	int id = to_intconvertor(cfgDDS2->idcode);
	
	//SEPARATE SOC	
	copy_cbyc(cfgDDS2->soc,cfg->soc,4);
	cfgDDS2->soc[4] = '\0';

	//SEPARATE FRACSEC	
	copy_cbyc(cfgDDS2->fracsec,cfg->fracsec,4);
	cfgDDS2->fracsec[4] = '\0';

	//SEPARATE TIMEBASE
	copy_cbyc (cfgDDS2->time_base,cfg->time_base,4);
	cfgDDS2->time_base[4]='\0';

	//SEPARATE PMU NUM
	copy_cbyc (cfgDDS2->num_pmu,cfg->num_pmu,2);
	cfgDDS2->num_pmu[2] = '\0';

	num_pmu = to_intconvertor(cfgDDS2->num_pmu);	
//	printf("Number of PMU's %d\n",num_pmu);

	// Allocate Memeory For Each PMU
	cfgDDS2->pmu = malloc(sizeof(struct for_each_pmu *)); // There is only one PMU
	if(!cfgDDS2->pmu) {
		printf("Not enough memory for pmu[][]\n");
		exit(1);
	}

	cfgDDS2->pmu[0] = malloc(sizeof(struct for_each_pmu));

	j = 0;


	// Memory Allocation for stn
	cfgDDS2->pmu[j]->stn = malloc(17*sizeof(unsigned char));
	if(!cfgDDS2->pmu[j]->stn) {
		printf("Not enough memory cfgDDS2->pmu[j]->stn\n");
		exit(1);
	}

	// Memory Allocation for idcode
	cfgDDS2->pmu[j]->idcode = malloc(3*sizeof(unsigned char));
	if(!cfgDDS2->pmu[j]->idcode) {
		printf("Not enough memory cfgDDS2->pmu[j]->idcode\n");
		exit(1);
	}

	// Memory Allocation for format
	cfgDDS2->pmu[j]->data_format = malloc(3*sizeof(unsigned char));
	if(!cfgDDS2->pmu[j]->data_format) {
		printf("Not enough memory cfgDDS2->pmu[j]->data_format\n");
		exit(1);
	}

	// Memory Allocation for phnmr
	cfgDDS2->pmu[j]->phnmr = malloc(3*sizeof(unsigned char));
	if(!cfgDDS2->pmu[j]->phnmr) {
		printf("Not enough memory cfgDDS2->pmu[j]->phnmr\n");
		exit(1);
	}

	// Memory Allocation for annmr
	cfgDDS2->pmu[j]->annmr = malloc(3*sizeof(unsigned char));
	if(!cfgDDS2->pmu[j]->annmr) {
		printf("Not enough memory cfgDDS2->pmu[j]->annmr\n");
		exit(1);
	}

	// Memory Allocation for dgnmr
	cfgDDS2->pmu[j]->dgnmr = malloc(3*sizeof(unsigned char));
	if(!cfgDDS2->pmu[j]->dgnmr) {
		printf("Not enough memory cfgDDS2->pmu[j]->dgnmr\n");
		exit(1);
	}

	// Memory Allocation for fnom
	cfgDDS2->pmu[j]->fnom = malloc(3*sizeof(unsigned char));
	if(!cfgDDS2->pmu[j]->fnom) {
		printf("Not enough memory cfgDDS2->pmu[j]->fnom\n");
		exit(1);
	}

	// Memory Allocation for cfgDDS2_cnt
	cfgDDS2->pmu[j]->cfg_cnt = malloc(3*sizeof(unsigned char));
	if(!cfgDDS2->pmu[j]->cfg_cnt) {
		printf("Not enough memory cfgDDS2->pmu[j]->cfgDDS2_cnt\n");
		exit(1);
	}

	//SEPARATE STATION NAME
	copy_cbyc (cfgDDS2->pmu[j]->stn,cfg->pmu[t]->stn,16);
	cfgDDS2->pmu[j]->stn[16] = '\0';
	
	//SEPARATE IDCODE		
	copy_cbyc (cfgDDS2->pmu[j]->idcode,cfg->pmu[t]->idcode,2);
	cfgDDS2->pmu[j]->idcode[2] = '\0';
	//printf("ID = %d\n",to_intconvertor(cfgDDS2->pmu[j]->idcode));
	
	//SEPARATE DATA FORMAT		
	copy_cbyc (cfgDDS2->pmu[j]->data_format,cfg->pmu[t]->data_format,2);
	cfgDDS2->pmu[j]->data_format[2]='\0';
	
	//SEPARATE PHASORS	
	memset(nPAD,'\0',3);
	int_to_ascii_convertor(nn,nPAD);
	copy_cbyc (cfgDDS2->pmu[j]->phnmr,nPAD,2);
	cfgDDS2->pmu[j]->phnmr[2]='\0';
	phn = to_intconvertor(cfgDDS2->pmu[j]->phnmr);
	
	//SEPARATE ANALOGS			
	memset(nPAD,'\0',3);
	int_to_ascii_convertor(0,nPAD);
	copy_cbyc (cfgDDS2->pmu[j]->annmr,nPAD,2);
	cfgDDS2->pmu[j]->annmr[2]='\0';
	ann = to_intconvertor(cfgDDS2->pmu[j]->annmr);

	//SEPARATE DIGITALS			
	memset(nPAD,'\0',3);
	int_to_ascii_convertor(0,nPAD);
	copy_cbyc (cfgDDS2->pmu[j]->dgnmr,nPAD,2);
	cfgDDS2->pmu[j]->dgnmr[2]='\0';
	dgn = to_intconvertor(cfgDDS2->pmu[j]->dgnmr);	

	unsigned char hex = cfgDDS2->pmu[j]->data_format[1];	


	if(multiFlag == 1)
		((struct MapPMUToPhasor *)found->data)->format = hex;	
	else if(multiFlag == 2)
		((struct MapCGGPMUID2PhasorIndex *)found->data)->format = hex;	

	hex <<= 4;
	
	dataFrameSize_DDS2 += 2; // For stat word	
	
	if((hex & 0x80) == 0x80) {
	
		dataFrameSize_DDS2 += 8; // freq and dfreq		
	
	} else {
	
		dataFrameSize_DDS2 += 4;  //freq and dfreq
	}
	
	
	if((hex & 0x20) == 0x20) {
	
		dataFrameSize_DDS2 += phn*8;
	
	} else {

		dataFrameSize_DDS2 += phn*4;
	}
	
	cn = malloc(sizeof(struct channel_names));
	cn->first = NULL;

	////SEPARATE PHASOR NAMES 
	if(phn != 0){

		cn->phnames = malloc(phn*sizeof(unsigned char*));

		if(!cn->phnames) {
			printf("Not enough memory cfgDDS2->pmu[j]->cn->phnames[][]\n");
			exit(1);
		}

		for (i = 0; i < phn; i++) {

			cn->phnames[i] = malloc(17*sizeof(unsigned char));
		}

		i = 0;	//Index for PHNAMES

		if(found != NULL) {

			if(multiFlag == 1) {
//					printf("bingo1\n");
				while(i<phn){

					copy_cbyc (cn->phnames[i],((struct MapPMUToPhasor *)found->data)->PhasorName[i],16);
					cn->phnames[i][16]='\0';       
					i++;
				}
			} else if(multiFlag == 2) {

//				printf("bingo21\n");
		
				copy_cbyc (cn->phnames[0],"V1",2); // Hardcoded the name
				cn->phnames[i][16]='\0';       
//				printf("bingo22\n");
			}
		} else {
			
			printf("some problem\n");
		}
	} 		

	cfgDDS2->pmu[j]->cnext = cn;//Assign to pointers

	///PHASOR FACTORS
	if(phn != 0){

		cfgDDS2->pmu[j]->phunit = malloc(phn*sizeof(unsigned char*));

		if(!cfgDDS2->pmu[j]->phunit) {

			printf("Not enough memory cfgDDS2->pmu[j]->phunit[][]\n");
			exit(1);
		}

		for (i = 0; i < phn; i++) {

			cfgDDS2->pmu[j]->phunit[i] = malloc(5);
		}

		i = 0;
		int h;
		if(multiFlag == 1) {
			while(i<phn){ //Separate the Phasor conversion factors

				h = ((struct MapPMUToPhasor *)found->data)->DDS2_Phasor_Index[i];
				copy_cbyc (cfgDDS2->pmu[j]->phunit[i],cfg->pmu[t]->phunit[h],4);
				cfgDDS2->pmu[j]->phunit[i][4] = '\0';			
				i++;
			}
		} else if (multiFlag == 2) {

			//printf("bingo--\n");
			copy_cbyc (cfgDDS2->pmu[j]->phunit[0],cfg->pmu[t]->phunit[0],4);
			cfgDDS2->pmu[j]->phunit[i][4] = '\0';			
		}
	}//if for PHASOR Factors ends
	
	copy_cbyc (cfgDDS2->pmu[j]->fnom,cfg->pmu[t]->fnom,2);
	cfgDDS2->pmu[j]->fnom[2]='\0';	

	copy_cbyc (cfgDDS2->pmu[j]->cfg_cnt,cfg->pmu[t]->cfg_cnt,2);
	cfgDDS2->pmu[j]->cfg_cnt[2]='\0';

	cfgDDS2->cfgnext = NULL;

	int index = 0; 
	if(cfgfirst_DDS2 == NULL) {
		
		COMBINE_DF_SIZE_DDS2 = dataFrameSize_DDS2;
		cfgfirst_DDS2 = cfgDDS2;
		if(multiFlag == 1)
		{
//			printf("bingo++\n");
			((struct MapPMUToPhasor *)found->data)->dataFrameSize = dataFrameSize_DDS2;
			((struct MapPMUToPhasor *)found->data)->startIndex = 0;
			((struct MapPMUToPhasor *)found->data)->nextIdcode = -1;
			((struct MapPMUToPhasor *)found->data)->prevIdcode = -1;		
			//printf("+multiFlag %d ID = %d, startIndex %d, dataframe_DDS2 %d\n",multiFlag, id,((struct MapPMUToPhasor *)found->data)->startIndex,dataFrameSize_DDS2);

		} else if(multiFlag == 2) {

//			printf("bingo--\n");

			((struct MapCGGPMUID2PhasorIndex *)found->data)->dataFrameSize = dataFrameSize_DDS2;
			((struct MapCGGPMUID2PhasorIndex *)found->data)->startIndex = 0;
			//printf("-multiFlag %d, ID = %d, startIndex %d, dataframe_DDS2 %d\n",multiFlag, id,((struct MapPMUToPhasor *)found->data)->startIndex,((struct MapCGGPMUID2PhasorIndex *)found->data)->dataFrameSize);
		}

	} else {

		struct cfg_frame *temp_cfg = cfgfirst_DDS2,*tprev_cfg;
		tprev_cfg = temp_cfg;
		bool match = false;

		//Check if the configuration frame already exists
		while(temp_cfg!=NULL){

			if(!ncmp_cbyc(cfg->idcode,temp_cfg->idcode,2)) {

				match = true;
				break;	

			} else {
				
				index++;
				tprev_cfg = temp_cfg;
				temp_cfg = temp_cfg->cfgnext;
			}
		}// While ends

		ENTRY entry;
		ENTRY *found1 = NULL,*found2 = NULL;
		
		if(match) {			

			if(!index) {

				// Replace the cfgfirst
				cfgDDS2->cfgnext = cfgfirst_DDS2->cfgnext;				
				free_cfgframe_object(cfgfirst_DDS2);	
				cfgfirst_DDS2 = cfgDDS2;

				memset(dds2Key,'\0',5);
				int id = to_intconvertor(cfgDDS2->idcode);
        		sprintf(dds2Key,"%d",id);
        		dds2K = dds2Key;
		
				entry.key = dds2K;
				hsearch_r( entry, FIND, &found1, &hashForMapPMUToPhasor);
				if(found != NULL) {
					
					COMBINE_DF_SIZE_DDS2 -= ((struct MapPMUToPhasor *)found1->data)->dataFrameSize;
					((struct MapPMUToPhasor *)found1->data)->dataFrameSize = dataFrameSize_DDS2;
					COMBINE_DF_SIZE_DDS2 += dataFrameSize_DDS2;

					int nextIdcode = to_intconvertor(cfgfirst_DDS2->cfgnext->idcode);
					((struct MapPMUToPhasor *)found1->data)->nextIdcode = nextIdcode;
					int nextStartIndex = ((struct MapPMUToPhasor *)found1->data)->startIndex+ dataFrameSize_DDS2;
					hash_update_Index_N_NextID_to_MapPMUToPhasor(found1,nextStartIndex);

				} else {

					printf("Bingo No ID in the hash table 1\n");
				}

			} else {

				// Replace in between cfg
				tprev_cfg->cfgnext = cfgDDS2;
				cfgDDS2->cfgprev = tprev_cfg;
				cfgDDS2->cfgnext = temp_cfg->cfgnext;							
				free_cfgframe_object(temp_cfg);

				memset(dds2Key,'\0',5);
				int id = to_intconvertor(cfgDDS2->idcode);
        		sprintf(dds2Key,"%d",id);
        		dds2K = dds2Key;
		
				entry.key = dds2K;
				hsearch_r( entry, FIND, &found1, &hashForMapPMUToPhasor);				
				if(found != NULL) {
				
					COMBINE_DF_SIZE_DDS2 -= ((struct MapPMUToPhasor *)found1->data)->dataFrameSize;
						
					((struct MapPMUToPhasor *)found1->data)->dataFrameSize = dataFrameSize_DDS2; // Add checksum byte Last node
					COMBINE_DF_SIZE_DDS2 += dataFrameSize_DDS2;

					int nextIdcode = to_intconvertor(cfgDDS2->cfgnext->idcode);
					((struct MapPMUToPhasor *)found1->data)->nextIdcode = nextIdcode;
					((struct MapPMUToPhasor *)found1->data)->prevIdcode = to_intconvertor(cfgDDS2->cfgprev->idcode);
					int nextStartIndex = ((struct MapPMUToPhasor *)found1->data)->startIndex+ dataFrameSize_DDS2;
					hash_update_Index_N_NextID_to_MapPMUToPhasor(found1,nextStartIndex);
				
				} else {

					printf("Bingo No ID in the hash table 2\n");	
				}
			}

		} else { // No match and not first cfg 

			tprev_cfg->cfgnext = cfgDDS2;
			cfgDDS2->cfgprev = tprev_cfg;			

			unsigned int tID = to_intconvertor(tprev_cfg->idcode);

		//	printf("TID  = %d\n", tID);
			memset(dds2Key,'\0',5);
        	sprintf(dds2Key,"%d",tID);
        	dds2K = dds2Key;
			entry.key = dds2K;

			if(angleDiffApp)
				hsearch_r( entry, FIND, &found1, &hashForMapPMUToPhasor);

			if(coherencyOfGen)
				hsearch_r( entry, FIND, &found2, &cggHashForPhasorIndex);

			int startIndex = 0;
			COMBINE_DF_SIZE_DDS2 += dataFrameSize_DDS2;

			if(found1 != NULL) {
							
				startIndex = ((struct MapPMUToPhasor *)found1->data)->startIndex + ((struct MapPMUToPhasor *)found1->data)->dataFrameSize;																		

				if(multiFlag == 1)
				{

					((struct MapPMUToPhasor *)found->data)->startIndex = startIndex;
					((struct MapPMUToPhasor *)found->data)->dataFrameSize = dataFrameSize_DDS2;
					((struct MapPMUToPhasor *)found->data)->nextIdcode = -1;
			//		printf("++multiFlag %d, ID = %d, startIndex %d, dataframe_DDS2 %d\n",multiFlag, id,((struct MapPMUToPhasor *)found->data)->startIndex,dataFrameSize_DDS2);
				} else if (multiFlag == 2) {

					((struct MapCGGPMUID2PhasorIndex *)found->data)->startIndex = startIndex;
					((struct MapCGGPMUID2PhasorIndex *)found->data)->dataFrameSize = dataFrameSize_DDS2;
			//		printf("--multiFlag %d, ID = %d, startIndex %d, dataframe_DDS2 %d\n",multiFlag, id,((struct MapCGGPMUID2PhasorIndex *)found->data)->startIndex,dataFrameSize_DDS2);

				}
			} else if (found2 != NULL) {
			
				startIndex = ((struct MapCGGPMUID2PhasorIndex *)found2->data)->startIndex + ((struct MapCGGPMUID2PhasorIndex *)found2->data)->dataFrameSize;																		
		//		printf(":::tID %d startIndex %d\n", tID, startIndex);

				if(multiFlag == 1)
				{

					((struct MapPMUToPhasor *)found->data)->startIndex = startIndex;
					((struct MapPMUToPhasor *)found->data)->dataFrameSize = dataFrameSize_DDS2;
					((struct MapPMUToPhasor *)found->data)->nextIdcode = -1;
			//		printf("**multiFlag %d, ID = %d, startIndex %d, dataframe_DDS2 %d\n",multiFlag, id,((struct MapPMUToPhasor *)found->data)->startIndex,dataFrameSize_DDS2);

				} else if (multiFlag == 2) {

					((struct MapCGGPMUID2PhasorIndex *)found->data)->startIndex = startIndex;
					((struct MapCGGPMUID2PhasorIndex *)found->data)->dataFrameSize = dataFrameSize_DDS2;
			//		printf("@@multiFlag %d, ID = %d, startIndex %d, dataframe_DDS2 %d\n", multiFlag, id,((struct MapCGGPMUID2PhasorIndex *)found->data)->startIndex,dataFrameSize_DDS2);

				}

			} else {
				printf("bingo no ID in hash table 3. Some problem\n");
			}
		}   				
	}
}

/* --------------------------------------------------------------------------------------	*/
/* FUNCTION void create_DDS2_dataframe(struct data_frame *df)								*/
/* --------------------------------------------------------------------------------------	*/

int create_DDS2_dataframe(struct data_frame *df) {

//	printf("-->create_DDS2_dataframe\n");
	int total_frame_size = 0;
	unsigned char temp[3];
	struct data_frame *temp_df;	
	uint16_t chk;
	unsigned int offset = 14;
	//unsigned int dfSize;

	temp_df = df;

	// Calculate the single combined data frame size
	total_frame_size = COMBINE_DF_SIZE_DDS2 + 16 ;	/* 16 is for comman fields SYNC + FRAMESIZE + idcode + soc + fracsec + checksum + Analysis TS*/
	if(dataframe_DDS2 == NULL) {

		dataframe_DDS2 = malloc((total_frame_size + 1 + 8)*sizeof(unsigned char));
		if(!dataframe_DDS2)
			printf("No enough memory for dataframe\n");
		memset(dataframe_DDS2,'\0',total_frame_size+1+8);						
	
	} else {

		char *fsize;
		fsize = malloc(3);
		fsize[0] = dataframe_DDS2[2];
		fsize[1] = dataframe_DDS2[3];
		fsize[2] = '\0';
		int size = to_intconvertor(fsize);		
		free(fsize);

		if(size <total_frame_size) {

			free(dataframe_DDS2);
			dataframe_DDS2 = malloc((total_frame_size + 1 +8)*sizeof(unsigned char)); 	
			
			if(!dataframe_DDS2)
				printf("No enough memory for dataframe\n");
			memset(dataframe_DDS2,'\0',total_frame_size+1+8);			
	
		} else {
			
			memset(dataframe_DDS2,'\0',total_frame_size+1+8);		
		}
	}
				
	// Start the data frame creation 
	int z = 0;
	byte_by_byte_copy(dataframe_DDS2,DATASYNC,z,2); // SYNC Word for data frame
	z += 2;

	memset(temp,'\0',3);
	int_to_ascii_convertor(total_frame_size,temp);
	byte_by_byte_copy(dataframe_DDS2,temp,z,2); // FRAME SIZE
	z += 2;

	memset(temp,'\0',3);
	int_to_ascii_convertor(PDC_IDCODE,temp);
	byte_by_byte_copy(dataframe_DDS2,temp,z,2); // PDC ID
	z += 2;

	byte_by_byte_copy(dataframe_DDS2,(unsigned char *)df->soc,z,4); //SOC
	z += 4;
	byte_by_byte_copy(dataframe_DDS2,(unsigned char *)df->fracsec,z,4); //FRACSEC + Time Quality?
	z += 4;

	temp_df = df;
	
	ENTRY entry1,entry2;
	ENTRY *found1 = NULL,*found2 = NULL;

	while(temp_df != NULL) { // 1

		memset(dds2Key,'\0',5);
		int id = to_intconvertor(temp_df->idcode);
		sprintf(dds2Key,"%d",id);
        pmuKey = dds2Key;
        entry1.key = pmuKey;						
        entry2.key = pmuKey;
        if(angleDiffApp) {

			hsearch_r( entry1, FIND, &found1, &hashForMapPMUToPhasor);
		}
		
		if(found1 != NULL) {				

			z = offset + ((struct MapPMUToPhasor *)found1->data)->startIndex;
			((struct MapPMUToPhasor *)found1->data)->visited = true;
			int pIndex = ((struct MapPMUToPhasor *)found1->data)->DDS2_Phasor_Index[0];
			attacheEachdataframeInDDS2(found1,temp_df,z,1);		
			temp_df = temp_df->dnext;	
		} else 	{

			if(coherencyOfGen)
				hsearch_r( entry2, FIND, &found2, &cggHashForPhasorIndex);
			if(found2 != NULL) {

				z = offset + ((struct MapCGGPMUID2PhasorIndex *)found2->data)->startIndex;
				((struct MapCGGPMUID2PhasorIndex *)found2->data)->visited = true;				
//				printf("z = %d, totalSize %d\n",z,total_frame_size);
				//attacheEachdataframeInDDS2(found2,temp_df,z,2);		
				temp_df = temp_df->dnext;	
				
			} else {

				//printf("Node not found in HT hashForMapPMUToPhasor in create_DDS2_dataframe() %s \n",pmuKey);					
				temp_df = temp_df->dnext;					
			}
			
		} 
		
		found2 = found1 = NULL;		
	} // 1 while

	found2 = found1 = NULL;	


// Search for unvisted nodes/Missing frames	
	struct cfg_frame *temp_cfg = cfgfirst_DDS2;
	int id;

	while(temp_cfg != NULL) 
	{
		id = to_intconvertor(temp_cfg->idcode);
		memset(dds2Key,'\0',5);
		sprintf(dds2Key,"%d",id);
    	pmuKey = dds2Key;
    	entry1.key = pmuKey;								
    	entry2.key = pmuKey;

    	if(angleDiffApp)
			hsearch_r(entry1, FIND, &found1, &hashForMapPMUToPhasor);
		if(found1 != NULL )
		{

			if(((struct MapPMUToPhasor *)found1->data)->visited == false) {

				unsigned char *temp = malloc(3*sizeof(unsigned char));
				memset(temp,'\0',3);
				int_to_ascii_convertor(id,temp);
				printf("Missing ID = %d\n",id );

				struct data_frame *curr_df;			
				curr_df = generate_dummy_dataframe_DDS2(found1,temp);
				free(temp);			

				int z = offset +  ((struct MapPMUToPhasor *)found1->data)->startIndex;
				
				attacheEachdataframeInDDS2(found1,curr_df,z,1);													

			} else {

				((struct MapPMUToPhasor *)found1->data)->visited = false;				
			}

		} else {

			if(coherencyOfGen)
				hsearch_r(entry2, FIND, &found2, &cggHashForPhasorIndex);
			if(found2 != NULL) {

				if(((struct MapCGGPMUID2PhasorIndex *)found2->data)->visited == false) {

					unsigned char *temp = malloc(3*sizeof(unsigned char));
					memset(temp,'\0',3);
					int_to_ascii_convertor(id,temp);

					struct data_frame *curr_df;			
					curr_df = generate_dummy_dataframe_DDS2(found2,temp);
					free(temp);			

					int z = offset + ((struct MapCGGPMUID2PhasorIndex *)found2->data)->startIndex;
				//	printf("bingo lingo\n");
					attacheEachdataframeInDDS2(found2,curr_df,z,2);													

				} else {

					((struct MapCGGPMUID2PhasorIndex *)found2->data)->visited = false;				
				}
			}
		}

		temp_cfg = temp_cfg->cfgnext;
		found2 = found1 = NULL;		
	}


	// Attach a checksum
	int totalSize = offset + COMBINE_DF_SIZE_DDS2;
	chk = compute_CRC(dataframe_DDS2,totalSize);
	dataframe_DDS2[totalSize++] = (chk >> 8) & ~(~0<<8);  /* CHKSUM high byte; */
	dataframe_DDS2[totalSize++] = (chk ) & ~(~0<<8);   	/* CHKSUM low byte;  */

	// For iPDC configuration changed
	old_fsize_dds2 = totalSize;
//	printf("totalSize %d \n",totalSize);
//	printf("<--create_DDS2_dataframe\n");
	return totalSize;	
} 


/* --------------------------------------------------------------------------------------	*/
/* FUNCTION void attacheEachdataframeInDDS2(ENTRY *found,struct data_frame *temp_df,int z)	*/
/* --------------------------------------------------------------------------------------	*/

void attacheEachdataframeInDDS2(ENTRY *found,struct data_frame *temp_df,int z,int multiFlag) {

	int j = 0;
	//while(j < temp_df->num_pmu) { // 2

	//Copy STAT Word

	byte_by_byte_copy(dataframe_DDS2,temp_df->dpmu[j]->stat,z,2);

	z += 2;

	int i = 0;
	unsigned int pIndex;
	unsigned char hex;

	//Copy Phasors
	if(multiFlag == 1)
		hex = ((struct MapPMUToPhasor *)found->data)->format;	
	else if(multiFlag == 2)
		hex = ((struct MapCGGPMUID2PhasorIndex *)found->data)->format;	

	hex <<= 4;
	
	int nn;

	if(temp_df->dpmu[j]->phnmr != 0) {		

		if(multiFlag == 1) {
		
			nn = ((struct MapPMUToPhasor *)found->data)->numOfPhasors;
			if((hex & 0x20) == 0x20) {
				
				while(i < nn) {

					pIndex = ((struct MapPMUToPhasor *)found->data)->DDS2_Phasor_Index[i];
					byte_by_byte_copy(dataframe_DDS2,temp_df->dpmu[j]->phasors[pIndex],z,8); // Phasors
					z += 8;
					i++;
				}

			} else {
				
				while(i < nn) {

					//pIndex = ((struct MapPMUToPhasor *)found->data)->PhasorIndex[i];
					pIndex = ((struct MapPMUToPhasor *)found->data)->DDS2_Phasor_Index[i];
					byte_by_byte_copy(dataframe_DDS2,temp_df->dpmu[j]->phasors[pIndex],z,4); // Phasors
					z += 4;
					i++;
				}
			}			

		} else if(multiFlag == 2) {
		
			nn = 1;

			if((hex & 0x20) == 0x20) {
				
				byte_by_byte_copy(dataframe_DDS2,temp_df->dpmu[j]->phasors[0],z,8); // Phasors
				z += 8;
			
			} else {

				byte_by_byte_copy(dataframe_DDS2,temp_df->dpmu[j]->phasors[0],z,4); // Phasors
				z += 4;				
			}			
		}
	}

	//Copy FREQ
	if((hex & 0x80) == 0x80) {

		byte_by_byte_copy(dataframe_DDS2,temp_df->dpmu[j]->freq,z,4); // FREQ
		z += 4;
		byte_by_byte_copy(dataframe_DDS2,temp_df->dpmu[j]->dfreq,z,4); // FREQ
		z += 4;

		} else {

		byte_by_byte_copy(dataframe,temp_df->dpmu[j]->freq,z,2); // FREQ
		z += 2;
		byte_by_byte_copy(dataframe,temp_df->dpmu[j]->dfreq,z,2); // FREQ
		z += 2;

	}						
	//	j++;			
	//} // 2 while
}


/* ----------------------------------------------------------------------------	*/
/* FUNCTION unsigned char* generate_dummy_dataframe_DDS2(ENTRY *found,
										unsigned char *idcode):					*/
/* ----------------------------------------------------------------------------	*/

struct data_frame* generate_dummy_dataframe_DDS2(ENTRY *found,unsigned char *idcode)
{
	int match=0,i,j=0, fsize=0;
	unsigned char tempI[2];
	unsigned int num_pmu,phnmr,annmr,dgnmr; 
	struct cfg_frame *temp_cfg;
	struct data_frame *df;

	unsigned char temp2[2] = {0, 0};
	unsigned char temp4[4] = {0, 0, 0, 0};
	unsigned char temp8[8] = {0, 0, 0, 0, 0, 0, 0, 0};

	unsigned char hex = ((struct MapPMUToPhasor *)found->data)->format;
	hex <<= 4;
	// If idcode matches with cfg idcode
	//if(match){

		//Allocate memory for data frame 
		df = malloc(sizeof(struct data_frame));
		if(!df) {

			printf("Not enough memory for dummy df\n");
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
		num_pmu = 1;
		/* For each PMU data */
		df->dpmu = malloc(num_pmu* sizeof(struct data_for_each_pmu *));

		if(!df->dpmu) {

			printf("Not enough memory df->dpmu[][]\n");
			exit(1);
		}

		for (i = 0; i < num_pmu; i++) {

			df->dpmu[i] = malloc(sizeof(struct data_for_each_pmu));
		}

		/* Now start separating the data from data frame */
		/* Copy Framesize */
		copy_cbyc (df->framesize,temp2,2);
		df->framesize[2] = '\0';

		//Copy IDCODE
		copy_cbyc (df->idcode,idcode,2);	
		df->idcode[2] = '\0';

		//Copy SOC
		copy_cbyc (df->soc,temp4,4);
		df->soc[4] = '\0';

		//Copy FRACSEC
		copy_cbyc (df->fracsec,temp4,4);
		df->fracsec[4] = '\0';
		fsize += 16;	// For SYNC + FRAMESIZE + idcode + soc + fracsec + checksum

		//Copy NUM PMU
		df->num_pmu = num_pmu;

		// Separate the data for each PMU	    	
		j =0;
		//while(j<num_pmu) {		  					

			// Extract PHNMR, DGNMR, ANNMR values
			phnmr = ((struct MapPMUToPhasor *)found->data)->numOfPhasors;
			annmr = 0;
			dgnmr = 0;

			//Allocate memory for stat, Phasors, Analogs,Digitals and Phasors and Frequencies
			/* Memory Allocation Begins */

			// Allocate memory for stat
			df->dpmu[j]->stat = malloc(3*sizeof(unsigned char));

			if(!df->dpmu[j]->stat) {

				printf("Not enough memory for df->dpmu[j]->stat\n");
			}

			// Allocate memory for Phasor
			df->dpmu[j]->phasors = malloc(phnmr*sizeof(unsigned char *));

			if(!df->dpmu[j]->phasors) {

				printf("Not enough memory df->dpmu[j]->phasors[][]\n");
				exit(1);
			}

			if((hex & 0x20) == 0x20) {
			
				for (i = 0; i < phnmr; i++)

					df->dpmu[j]->phasors[i] = malloc(9*sizeof(unsigned char));
			} else {

				for (i = 0; i < phnmr; i++) 

					df->dpmu[j]->phasors[i] = malloc(5*sizeof(unsigned char));
			}

			
			/* Allocate memory for Frequency & DFREQ */
			if((hex & 0x80) == 0x80) {

				df->dpmu[j]->freq = malloc(5*sizeof(unsigned char));
				df->dpmu[j]->dfreq = malloc(5*sizeof(unsigned char));

			} else {

				df->dpmu[j]->freq = malloc(3*sizeof(unsigned char));
				df->dpmu[j]->dfreq = malloc(3*sizeof(unsigned char));
			}

			/* Memory Allocation Ends here */		

			// Copy the PMU data fields
			tempI[0] = 0x82; //changed by Kedar on 4-7-2013
			tempI[1] = 0x00;

			copy_cbyc (df->dpmu[j]->stat,tempI,2);
			df->dpmu[j]->stat[2] = '\0';
			fsize += 2;

			// Copy number of phasors, analogs, and digitals	
			df->dpmu[j]->phnmr = phnmr;
			df->dpmu[j]->annmr = annmr;			
			df->dpmu[j]->dgnmr = dgnmr;

			//Phasors 	
			if((hex & 0x20) == 0x20) {

				for(i=0;i<phnmr;i++){	

					copy_cbyc (df->dpmu[j]->phasors[i],temp8,8);
					df->dpmu[j]->phasors[i][8] = '\0';
					fsize += 8;
				}
			} else {

				for(i=0;i<phnmr;i++){					

					copy_cbyc (df->dpmu[j]->phasors[i],temp4,4);
					df->dpmu[j]->phasors[i][4] = '\0';
					fsize += 4;
				}
			}						

			/* For Freq */ 	
			if((hex & 0x80) == 0x80) {

				copy_cbyc (df->dpmu[j]->freq,temp4,4);
				df->dpmu[j]->freq[4] = '\0';
				fsize += 4;

				copy_cbyc (df->dpmu[j]->dfreq,temp4,4);
				df->dpmu[j]->dfreq[4] = '\0';
				fsize += 4;

			} else {

				copy_cbyc (df->dpmu[j]->freq,temp2,2);
				df->dpmu[j]->freq[2] = '\0';
				fsize += 2;

				copy_cbyc (df->dpmu[j]->dfreq,temp2,2);
				df->dpmu[j]->dfreq[2] = '\0';
				fsize += 2;
			}								
		
		//	j++;
		//} //While ends for sub PMUs

		tempI[0] = fsize>>8;
		tempI[1] = fsize;
		copy_cbyc (df->framesize,tempI,2);
		df->framesize[2] = '\0';

		return df;
	//}
}
