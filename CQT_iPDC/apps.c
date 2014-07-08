/* ----------------------------------------------------------------------------- 
 * align_sort.c
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
 #include <math.h>
#include <search.h> // hcreate_r() hdestroy_r() struct hsearch_data
#include "apps.h"
#include "global.h"

char *l1,*d1,*svptr;
size_t l2=0;
ssize_t result;
FILE *fp1;

/* ----------------------------------------------------------------------------	*/
/* FUNCTION  buildAppHashTables():						                        */
/* ----------------------------------------------------------------------------	*/

void buildAppHashTables() {

	TWOPI = 2*M_PI;
	printf("Creating hash tables\n");
	max_element_of_VSAppInfo = 10; // of elements in search table
	max_element_of_Apps_Details = 2000; // of elements in search table
	max_element_of_MapPMUToPhasor = 2000;
	max_element_of_CGG = 1000;
	max_element_of_LSE = 1000;
	max_element_of_cggHashForPhasorIndex = 1000;
	maxPhasors = 3;

	/* Open the iPDC CSV Setup File */	
	
	if(IamSPDC) {

		fp1 = fopen ("apps2.csv","r");	

		if(fp1 != NULL) {
		
			getdelim (&l1, &l2, ('\n'), fp1); 

			if ((result = getdelim (&l1, &l2, ('\n'), fp1)) >0)
			{
				/* For The First column CSV Header */
				
				readInertiaConstant();

				getdelim (&l1, &l2, ('\n'), fp1); // Skip Header

	//			printf("%s\n", l1);				
				d1 = strtok(l1,",");			
				d1 = strtok(NULL,",");
				
				TotApps = atoi(d1);			
				int i;
				for(i = 0;i<TotApps;i++)
				{	

					getdelim (&l1, &l2, ('\n'), fp1); // Skip Header
					getdelim (&l1, &l2, ('\n'), fp1);

					d1 = strtok (l1,","); // Total Apps			
					
					int appID = atoi(d1);					

					//printf("App ID %d\n", appID);
					if(appID == 1) // Voltage Stability Application
					{	
						angleDiffApp = true; // set the app flag													

						// we create the hash for Applications
						memset(&hashForVSApp, 0, sizeof(hashForVSApp));
						
						if( hcreate_r(max_element_of_VSAppInfo, &hashForVSApp) == 0 ) {
							perror("hcreate_r");
					  		exit(1);
						}
					
						struct VSAppInfo *VSAppInfo;
						VSAppInfo = malloc(TotApps*sizeof(struct VSAppInfo));
						VSAppInfo->appID = appID;						
						populateVSApp(VSAppInfo,i,d1);

						// Create a New Hash Table for App details
						memset(&hashForVSAppDetails, 0, sizeof(hashForVSAppDetails));
						if( hcreate_r(max_element_of_Apps_Details, &hashForVSAppDetails) == 0 ) {
								perror("hcreate_r");  					  					
						}

						// Create New Hash to store PMU ID to Phasor Map
						memset(&hashForMapPMUToPhasor, 0, sizeof(hashForMapPMUToPhasor));
						if( hcreate_r(max_element_of_MapPMUToPhasor, &hashForMapPMUToPhasor) == 0 ) {
								perror("hcreate_r");  					
						}
						
						struct VSAppDetails *VSAppDetails = malloc(VSAppInfo->noOfApps*2*sizeof(struct VSAppDetails));								

						populateVSAppDetails(VSAppInfo,VSAppDetails,d1);

					} else if(appID == 2) // Coherency of generators
					{
						coherencyOfGen = true;
						// we create the hash for CCG
						memset(&CGGhash, 0, sizeof(CGGhash));
						
						if(hcreate_r(max_element_of_CGG, &CGGhash) == 0 ) {
							perror("hcreate_r");				  	
						}	

						if(firstCCGApp == NULL) 
							firstCCGApp = (struct CCG *)malloc(sizeof(struct CCG));											
						firstCCGApp->appID = appID;												
						d1 = strtok(NULL,",");
						firstCCGApp->SPDCID = atoi(d1); 

						d1 = strtok(NULL,",");
						firstCCGApp->numOfLPDC = atoi(d1); 
						//printf("Num of LPDCs %d\n", firstCCGApp->numOfLPDC );
						firstCCGApp->lPDCID = malloc(firstCCGApp->numOfLPDC*sizeof(int));
						int i;
						for (i = 0;i<firstCCGApp->numOfLPDC-1;i++)
						{
							d1 = strtok(NULL,"::");
							firstCCGApp->lPDCID[i] = atof(d1);
						}
						
						d1 = strtok(NULL,":,");
						firstCCGApp->lPDCID[i] = atof(d1);
						
						d1 = strtok(NULL,",");
						firstCCGApp->numOfModes = atoi(d1); 
						populateCGG(firstCCGApp);
					} 				
				}
			}		
		}

	} else if (IamLPDC) {

		fp1 = fopen ("apps1.csv","r");	
				if(fp1 != NULL) {
		
			getdelim (&l1, &l2, ('\n'), fp1); 

			if ((result = getdelim (&l1, &l2, ('\n'), fp1)) >0)
			{
				/* For The First column CSV Header */
				
				readInertiaConstant();

				getdelim (&l1, &l2, ('\n'), fp1); // Skip Header

	//			printf("%s\n", l1);				
				d1 = strtok(l1,",");			
				d1 = strtok(NULL,",");
				
				TotApps = atoi(d1);			
				int i;
				for(i = 0;i<TotApps;i++)
				{	

					getdelim (&l1, &l2, ('\n'), fp1); // Skip Header
					getdelim (&l1, &l2, ('\n'), fp1);

					d1 = strtok (l1,","); // Total Apps			
					
					int appID = atoi(d1);					

					//printf("App ID %d\n", appID);
					if(appID == 1) // Voltage Stability Application
					{	
						angleDiffApp = true; // set the app flag													

						// we create the hash for Applications
						memset(&hashForVSApp, 0, sizeof(hashForVSApp));
						
						if( hcreate_r(max_element_of_VSAppInfo, &hashForVSApp) == 0 ) {
							perror("hcreate_r");
					  		exit(1);
						}
					
						struct VSAppInfo *VSAppInfo;
						VSAppInfo = malloc(TotApps*sizeof(struct VSAppInfo));
						VSAppInfo->appID = appID;						
						populateVSApp(VSAppInfo,i,d1);

						// Create a New Hash Table for App details
						memset(&hashForVSAppDetails, 0, sizeof(hashForVSAppDetails));
						if( hcreate_r(max_element_of_Apps_Details, &hashForVSAppDetails) == 0 ) {
								perror("hcreate_r");  					  					
						}

						// Create New Hash to store PMU ID to Phasor Map
						memset(&hashForMapPMUToPhasor, 0, sizeof(hashForMapPMUToPhasor));
						if( hcreate_r(max_element_of_MapPMUToPhasor, &hashForMapPMUToPhasor) == 0 ) {
								perror("hcreate_r");  					
						}
						
						struct VSAppDetails *VSAppDetails = malloc(VSAppInfo->noOfApps*2*sizeof(struct VSAppDetails));								

						populateVSAppDetails(VSAppInfo,VSAppDetails,d1);

					} else if(appID == 2) // Coherency of generators
					{
						coherencyOfGen = true;

						memset(&cggHashForPhasorIndex, 0, sizeof(cggHashForPhasorIndex));
						if( hcreate_r(max_element_of_cggHashForPhasorIndex, &cggHashForPhasorIndex) == 0 ) {
								perror("hcreate_r");  					
						}
						
						// we create the hash for CCG						
						d1 = strtok(NULL,",");
						//d1 = strtok(NULL,",");
						int noOfLPDCs = atoi(d1);
						//printf("%s\n", d1);
						getdelim (&l1, &l2, ('\n'), fp1);

						d1 = strtok(l1,",");						

						int x = 0;
						int k,cnt,pmuid;
						while(x != noOfLPDCs) {

							getdelim (&l1, &l2, ('\n'), fp1);
							d1 = strtok(l1,",");
							//printf("%s--\n", d1);
							int id = atoi(d1);							

							if(id == PDC_IDCODE) 
								break;
							else 
								x++;
						}

						d1 = strtok(NULL,",");
						cnt = atoi(d1);
						//printf("CNT %d\n", cnt);
						for(k = 0; k < cnt; k++) {

							d1 = strtok(NULL,",");
							pmuid = atoi(d1);
							struct MapCGGPMUID2PhasorIndex *mapCCG = malloc(sizeof(struct MapCGGPMUID2PhasorIndex));
							mapCCG->index = 0;
							mapCCG->visited = false;
							memset(appKey,'\0',50);
							sprintf(appKey,"%d",pmuid);
							//printf("%s\n", appKey);
							key = appKey;							
							hash_add_element_to_MapCGGPMUID2PhasorIndex(key,&mapCCG[0],&cggHashForPhasorIndex);							
						}														
					} 				
				}
			}		
		}
	}
}



void readInertiaConstant() {

	printf("Storing the inertia Constants\n");
	d1 = strtok_r(l1,",",&svptr);
	total_buses = atoi(d1);

	buses_power = (float*)malloc(total_buses * sizeof(float));	
	d1 = strtok_r(NULL,",",&svptr);
	listed_buses = atoi(d1);

	
	char *line2;
	char *d2, *svptr2;
	size_t len2 = 0,len=0;

	getdelim (&l1, &l2, ('\n'), fp1);
	d1 = strtok_r(l1,",",&svptr);

	getdelim (&line2, &len2, ('\n'), fp1);
	d2 = strtok_r(line2,",",&svptr2);
	int i;


	for(i=0; i<listed_buses; i++){
		d1 = strtok_r(NULL,",",&svptr);
		d2 = strtok_r(NULL,",",&svptr2);
		
		buses_power[atoi(d1)-1] = atof(d2);
		//printf("%f %s\n",buses_power[atoi(d1)-1],d2);
	}
}



void populateCGG(struct CCG *ccg) {

	int i,j;

	// Memory Allocation
	printf("Populate Coherent Group of generators\n");	

	ccg->lPDC = malloc(ccg->numOfLPDC*sizeof(struct ForEachLPDC *));	
	ccg->mode = malloc(ccg->numOfModes*sizeof(float));	
	ccg->threshold = malloc(2*ccg->numOfModes*sizeof(float));	
	ccg->phasorIndex = malloc(ccg->numOfModes*sizeof(float));	
	ccg->globalCOI = malloc(ccg->numOfModes*sizeof(float));	
	
	for(j=0; j<ccg->numOfLPDC; j++) 
		ccg->lPDC[j] = malloc(sizeof(struct ForEachLPDC));

	for(j=0; j<ccg->numOfLPDC; j++)  
		ccg->lPDC[j]->groupDetails = malloc(ccg->numOfModes*sizeof(struct EachGroupDetails *));		
	

	for(j=0; j<ccg->numOfLPDC; j++)  
		for(i=0; i<ccg->numOfModes; i++) 
				ccg->lPDC[j]->groupDetails[i] = malloc(sizeof(struct EachGroupDetails));						

	int g_count;
	ENTRY entry;
	ENTRY *found;
						
	int p = 0;					
	for(i=0; i<ccg->numOfModes; i++) {

		getdelim (&l1, &l2, ('\n'), fp1); // Remove Header
		getdelim (&l1, &l2, ('\n'), fp1);

		d1 = strtok_r(l1,",",&svptr);
		ccg->mode[i] = atof(d1);

		for(j = 0;j<ccg->numOfLPDC;j++) {

			d1 = strtok_r(NULL,",",&svptr);
			g_count = atoi(d1);
			ccg->lPDC[j]->groupDetails[i]->numOfGenerators = g_count;
			ccg->lPDC[j]->groupDetails[i]->violation = false;
		}
		
		d1 = strtok_r(NULL,",",&svptr);		
		d1 = strtok_r(NULL,",",&svptr);
		
		d1 = strtok_r(NULL,",",&svptr);
		int pIndex = atoi(d1);
		ccg->phasorIndex[i] = pIndex;		

		d1 = strtok_r(NULL,",",&svptr);
		float threshold = atof(d1);
		ccg->threshold[p++] = threshold;

		d1 = strtok_r(NULL,",",&svptr);
		threshold = atof(d1);
		ccg->threshold[p++] = threshold;

		int k;
		for(j = 0;j<ccg->numOfLPDC;j++) {

			getdelim (&l1, &l2, ('\n'), fp1); 	
			d1 = strtok_r(l1,",",&svptr);

			int pID;
			ccg->lPDC[j]->groupDetails[i]->PMU_ID = malloc(ccg->lPDC[j]->groupDetails[i]->numOfGenerators*sizeof(int));
			for (k = 0; k < ccg->lPDC[j]->groupDetails[i]->numOfGenerators; ++k) // Add PMU id under each group
			{
				d1 = strtok_r(NULL,",",&svptr);				
				pID = atoi(d1);
				ccg->lPDC[j]->groupDetails[i]->PMU_ID[k] = pID;	
				ccg->lPDC[j]->groupDetails[i]->Hg += buses_power[atoi(d1)-1];				

				memset(appKey,'\0',50);
				sprintf(appKey,"%d%d",pID,ccg->phasorIndex[i]);				
				entry.key = appKey;
				hsearch_r( entry,FIND, &found,&CGGhash);				
				if(found == NULL) {
					struct CGGMapPMUNPhasor ccgMap;
					hash_add_element_to_CCGTable(appKey,&ccgMap,&CGGhash);
				}
			}
		}		
	}		
}


/* ----------------------------------------------------------------------------	*/
/* FUNCTION  populateVSApp(struct VSAppInfo VSAppInfo [],int index, 
								 char *d1,int TotApps):                          */
/* ----------------------------------------------------------------------------	*/

void populateVSApp(struct VSAppInfo VSAppInfo [],int index, char *d1) {

	printf("Storing Voltage Stability Applications\n");
	int i =0;

	d1 = strtok (NULL,","); 
	//printf("%s\n", d1);
	VSAppInfo[index].SPDCID = atoi(d1);
//				printf("superPDCId %d\n",VSAppInfo[index].SPDCID);

	d1 = strtok (NULL,","); 
	VSAppInfo[index].noOfLPDC = atoi(d1);
//				printf("numOfLPDC %d\n",VSAppInfo[index].noOfLPDC);

	VSAppInfo[index].LPDCID = malloc(VSAppInfo[index].noOfLPDC*sizeof(int));

	while(i<VSAppInfo[index].noOfLPDC-1) {

		d1 = strtok (NULL,":"); 								
		VSAppInfo[index].LPDCID[i] = atoi(d1);					
//					printf("%d\n", VSAppInfo[index].LPDCID[i]); 
		i++;
	}
	
	d1 = strtok (NULL,":,"); 
	VSAppInfo[index].LPDCID[i] = atoi(d1);

//				printf("%d\n", VSAppInfo[index].LPDCID[i]);

	d1 = strtok (NULL,","); 
	VSAppInfo[index].noOfApps = atoi(d1);
//				printf("numOfApps %d\n",VSAppInfo[index].noOfApps);											
		
	d1 = strtok (NULL,","); 
	VSAppInfo[index].threshold = atoi(d1);	

//				printf("threshold %d\n",VSAppInfo[index].threshold);
	memset(appKey,'\0',50);
	sprintf(appKey,"%d",VSAppInfo[index].appID);
	key = appKey;
	hash_add_element_to_VSAppTable(key,&VSAppInfo[index],&hashForVSApp);
	VSAppInfo[index].appNext = NULL;

	if(appFirst == NULL) {
		appFirst = &VSAppInfo[index];
	} else {
		struct VSAppInfo *temp_app = appFirst;
		while(temp_app->appNext != NULL)
			temp_app = temp_app->appNext;
		temp_app->appNext = temp_app;
	}	
}


/* ----------------------------------------------------------------------------	*/
/* FUNCTION  populateVSVSAppDetails(struct VSAppInfo *VSAppInfo,
										struct VSAppDetails *VSAppDetails, char *d1):*/
/* ----------------------------------------------------------------------------	*/

void populateVSAppDetails(struct VSAppInfo *VSAppInfo,struct VSAppDetails *VSAppDetails, char *d1) {

	printf("Storing Voltage Stability Application Details\n");
	int i,j = 0;

	VSHashKeyList.key =  malloc(VSAppInfo->noOfApps*sizeof(char*));

	for(i=0;i<VSAppInfo->noOfApps;i++) {

		VSAppDetails[j].PhasorName = malloc(50*sizeof(char));
		VSAppDetails[j+1].PhasorName = malloc(50*sizeof(char));
		VSAppDetails[j].peerHashKey = malloc(100*sizeof(char));
		VSAppDetails[j+1].peerHashKey = malloc(100*sizeof(char));

		VSHashKeyList.key[i] = malloc(50*sizeof(char));

		memset(VSAppDetails[j].PhasorName,'\0',50);
		memset(VSAppDetails[j+1].PhasorName,'\0',50);
		memset(VSAppDetails[j].peerHashKey,'\0',100);
		memset(VSAppDetails[j+1].peerHashKey,'\0',100);
		memset(VSHashKeyList.key[i],'\0',50);

		result = getdelim (&l1, &l2, ('\n'), fp1);
							
		
		VSAppDetails[j].appID = VSAppInfo->appID;
		VSAppDetails[j+1].appID = VSAppInfo->appID;

		d1 = strtok (l1,":,");				
//		printf("%s\n", d1);
		VSAppDetails[j].LPDC_ID = atoi(d1);
//					printf("%d\n",VSAppDetails[j].LPDC_ID);
		
		d1 = strtok (NULL,",:");
		VSAppDetails[j+1].LPDC_ID = atoi(d1);
//					printf("%d\n",VSAppDetails[j+1].LPDC_ID);

		d1 = strtok (NULL,",:");
		VSAppDetails[j].PMU_ID = atoi(d1);
//					printf("PMU ID %d\n", VSAppDetails[j].PMU_ID);	

		d1 = strtok (NULL,",:");
		VSAppDetails[j+1].PMU_ID = atoi(d1);
//					printf("PMU ID %d\n", VSAppDetails[j+1].PMU_ID);	

		d1 = strtok (NULL,",:");
		strcpy(VSAppDetails[j].PhasorName,d1);
//					printf("PhasorName %s\n",VSAppDetails[j].PhasorName);	

		d1 = strtok (NULL,",:");
		strcpy(VSAppDetails[j+1].PhasorName,d1);
//					printf("PhasorName %s\n",VSAppDetails[j+1].PhasorName);	

		sprintf(VSAppDetails[j].peerHashKey,"%d%d%d%s",VSAppInfo->appID,VSAppDetails[j+1].LPDC_ID,VSAppDetails[j+1].PMU_ID,VSAppDetails[j+1].PhasorName);
//					printf("Hash key %s\n", VSAppDetails[j].peerHashKey);
		sprintf(VSAppDetails[j+1].peerHashKey,"%d%d%d%s",VSAppInfo->appID,VSAppDetails[j].LPDC_ID,VSAppDetails[j].PMU_ID,VSAppDetails[j].PhasorName);
//					printf("Hash key %s\n", VSAppDetails[j+1].peerHashKey);

		key = VSAppDetails[j+1].peerHashKey;
		hash_add_element_to_VSAppDetailTable(key,&VSAppDetails[j],&hashForVSAppDetails);
		key = VSAppDetails[j].peerHashKey;
		strcpy(VSHashKeyList.key[i],key); // Store all keys of hash tables
		hash_add_element_to_VSAppDetailTable(key,&VSAppDetails[j+1],&hashForVSAppDetails);
				
		ENTRY entry;
		ENTRY *found;
		
		memset(pmuIDKey,'\0',10); //=============== j
		sprintf(pmuIDKey,"%d",VSAppDetails[j].PMU_ID);
		key = pmuIDKey;
		entry.key = key;
		hsearch_r( entry,FIND, &found,&hashForMapPMUToPhasor);				

		if(found == NULL) { 
		
			struct MapPMUToPhasor *mapPMUToPhasor = malloc(sizeof(struct MapPMUToPhasor));			
			mapPMUToPhasor->PhasorName = malloc(maxPhasors*sizeof(char *));
			int k;
			for (k = 0;k<maxPhasors;k++) {

				mapPMUToPhasor->PhasorName[k] = malloc(20*sizeof(char));
				memset(mapPMUToPhasor->PhasorName[k],'\0',20);
			}			
			
			strncpy(mapPMUToPhasor->PhasorName[0],VSAppDetails[j].PhasorName,16);			
			mapPMUToPhasor->numOfPhasors = 1;			
			mapPMUToPhasor->PhasorIndex = malloc(maxPhasors*sizeof(unsigned int));
			mapPMUToPhasor->DDS2_Phasor_Index = malloc(maxPhasors*sizeof(int));					

			hash_add_element_to_MapPMUToPhasor(key,&mapPMUToPhasor[0],&hashForMapPMUToPhasor); // ADD MapPMUToPhasor
		
		} else {

			hash_update_element_to_MapPMUToPhasor(found,key,VSAppDetails[j].PhasorName);
		}//=============== j

		memset(pmuIDKey,'\0',10);//=============== j+1
		sprintf(pmuIDKey,"%d",VSAppDetails[j+1].PMU_ID);
		key = pmuIDKey;
		entry.key = key;
		hsearch_r( entry,FIND, &found,&hashForMapPMUToPhasor);				
		if(found == NULL) {

			struct MapPMUToPhasor *mapPMUToPhasor = malloc(sizeof(struct MapPMUToPhasor));
			mapPMUToPhasor->PhasorName = malloc(maxPhasors*sizeof(char *));
			mapPMUToPhasor->visited = false;
			int k;
			for (k = 0;k<maxPhasors;k++) {

				mapPMUToPhasor->PhasorName[k] = malloc(50*sizeof(char *));
				memset(mapPMUToPhasor->PhasorName[k],'\0',50);
			}			
			
			strncpy(mapPMUToPhasor->PhasorName[0],VSAppDetails[j].PhasorName,16);			
			mapPMUToPhasor->numOfPhasors = 1;			
			mapPMUToPhasor->PhasorIndex = malloc(maxPhasors*sizeof(int));
			mapPMUToPhasor->DDS2_Phasor_Index = malloc(maxPhasors*sizeof(int));					
			hash_add_element_to_MapPMUToPhasor(key,&mapPMUToPhasor[0],&hashForMapPMUToPhasor);
		} else {
		
			hash_update_element_to_MapPMUToPhasor(found,key,VSAppDetails[j+1].PhasorName);
		}//=============== j+1
		j+=2;
	}
}


/* ----------------------------------------------------------------------------	*/
/* FUNCTION  hash_add_element_to_CCGTable(char * elmt_key,
                      struct CGGMapPMUNPhasor elmt_data,
                      struct hsearch_data * table):         		     		*/
/* ----------------------------------------------------------------------------	*/

void hash_add_element_to_CCGTable(char * elmt_key,
                      struct CGGMapPMUNPhasor *elmt_data,
                      struct hsearch_data * table) {

  ENTRY item;
  ENTRY * ret;
    
  item.key = strdup(elmt_key);
  item.data = elmt_data;
	
  if( hsearch_r(item, ENTER, &ret, table) == 0 ) {

    perror("hsearch_r");
    exit(1);
  }
  return;
}


/* ----------------------------------------------------------------------------	*/
/* FUNCTION  hash_update_element_to_CCGTable(ENTRY *found,float angle): 		*/
/* ----------------------------------------------------------------------------	*/
void hash_update_element_to_CCGTable(ENTRY *found,float angle) {

	((struct CGGMapPMUNPhasor *)found->data)->angle = angle;		
		  
  	return;
}

/* ----------------------------------------------------------------------------	*/
/* FUNCTION  hash_add_element_to_VSAppTable(char * elmt_key,
                      struct VSAppInfo elmt_data,
                      struct hsearch_data * table):         		     		*/
/* ----------------------------------------------------------------------------	*/

void hash_add_element_to_VSAppTable(char * elmt_key,
                      struct VSAppInfo *elmt_data,
                      struct hsearch_data * table) {

  ENTRY item;
  ENTRY * ret;
  //struct VSAppInfo *elmt_data1 = &elmt_data; 
    
  item.key = strdup(elmt_key);
  item.data = elmt_data;
	
  if( hsearch_r(item, ENTER, &ret, table) == 0 ) {

    perror("hsearch_r");
    exit(1);
  }

  return;
}

/* ----------------------------------------------------------------------------	*/
/* FUNCTION  hash_add_element_to_VSAppDetailTable(
					  char * elmt_key,
                      struct VSAppDetails elmt_data,
                      struct hsearch_data * table):                 	     	*/
/* ----------------------------------------------------------------------------	*/

void hash_add_element_to_VSAppDetailTable(char * elmt_key,
                      struct VSAppDetails *elmt_data,
                      struct hsearch_data * table) {

  ENTRY item;
  ENTRY * ret;
  //struct VSAppDetails *elmt_data1 = &elmt_data;

  item.key = strdup(elmt_key);
  item.data = elmt_data;  

  if( hsearch_r(item, ENTER, &ret, table) == 0 ) {
    perror("hsearch_r");
    exit(1);
  }

  //printf("KEY %s\n", elmt_key);
  return;
}

/* ----------------------------------------------------------------------------	*/
/* FUNCTION  hash_add_element_to_DetailAppTable(
					  char * elmt_key,
                      struct MapPMUToPhasor elmt_data,
                      struct hsearch_data * table):                 	     	*/
/* ----------------------------------------------------------------------------	*/

void hash_add_element_to_MapPMUToPhasor(char * elmt_key,
                      struct MapPMUToPhasor *elmt_data,
                      struct hsearch_data * table) {

  ENTRY item;
  ENTRY * ret;
//  struct MapPMUToPhasor *elmt_data1 = &elmt_data;

  item.key = strdup(elmt_key);
  item.data = elmt_data;  

  if( hsearch_r(item, ENTER, &ret, table) == 0 ) {
    perror("hsearch_r");
    exit(1);
  }
  return;	
}

/* ----------------------------------------------------------------------------	*/
/* FUNCTION  void hash_update_element_to_MapPMUToPhasor(ENTRY *found,
														char *key,
														char PhasorName [])		*/
/* ----------------------------------------------------------------------------	*/

void hash_update_element_to_MapPMUToPhasor(ENTRY *found,char *key,char PhasorName []) {

	int n = ((struct MapPMUToPhasor *)found->data)->numOfPhasors;
	int i;
	bool flag =false;
	for(i = 0;i<n;i++) {

		if(!
			(PhasorName,((struct MapPMUToPhasor *)found->data)->PhasorName[i])) {
			flag = true;
			break;
		}
	}

	if(flag == true) {

		strcpy(((struct MapPMUToPhasor *)found->data)->PhasorName[n],PhasorName);		
		((struct MapPMUToPhasor *)found->data)->numOfPhasors = n+1;
	}
}

/* ----------------------------------------------------------------------------	*/
/* FUNCTION  void hash_update_Index_N_NextID_to_MapPMUToPhasor (ENTRY *found,
														int nextStartIndex)		*/
/* ----------------------------------------------------------------------------	*/

void hash_update_Index_N_NextID_to_MapPMUToPhasor(ENTRY *found1,int nextStartIndex) {

	unsigned int tnextID = ((struct MapPMUToPhasor *) found1->data)->nextIdcode;
	unsigned int xx;
	xx = nextStartIndex;
	
	ENTRY *found;
	ENTRY entry;	
	
	while(tnextID != -1) {			
	
		memset(pmuIDKey,'\0',10);
		sprintf(pmuIDKey,"%d",tnextID);		
		key = pmuIDKey;
		entry.key = key;
		
		hsearch_r(entry,FIND,&found,&hashForMapPMUToPhasor); 
		if(found != NULL) {				 
				
			((struct MapPMUToPhasor *)found->data)->startIndex = xx;				
			xx = ((struct MapPMUToPhasor *)found->data)->startIndex + ((struct MapPMUToPhasor *)found->data)->dataFrameSize;	
			tnextID = ((struct MapPMUToPhasor *)found->data)->nextIdcode;

		} else {
				printf("No item of in this table\n");
			
		}        		
	}
}


/* ----------------------------------------------------------------------------	*/
/* FUNCTION  hash_add_element_to_MapCGGPMUID2PhasorIndex (char * elmt_key,struct 
									MapCGGPMUID2PhasorIndex *elmt_data,
											struct hsearch_data * table)		*/
/* ----------------------------------------------------------------------------	*/

void hash_add_element_to_MapCGGPMUID2PhasorIndex(char * elmt_key,
						struct MapCGGPMUID2PhasorIndex *elmt_data,
						struct hsearch_data * table) {


  ENTRY item;
  ENTRY * ret;

  item.key = strdup(elmt_key);
  item.data = elmt_data;  

  if( hsearch_r(item, ENTER, &ret, table) == 0 ) {
    perror("hsearch_r");
    exit(1);
  }
  return;	
}