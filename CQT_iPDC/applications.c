/* ----------------------------------------------------------------------------- 
 * applications.c
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
 *		Kedar V. Khandeparkar <kedar.khandeparkar@gmail.com>			
 *
 * ----------------------------------------------------------------------------- */
#define _GNU_SOURCE
 
#include <stdio.h>
#include <string.h>
#include <stdlib.h> 
#include <math.h>       /* fabs */
#include "parser.h"
#include "global.h" 
#include "dallocate.h"
#include "applications.h"
#include "apps.h"

/* ----------------------------------------------------------------------------		*/
/* FUNCTION  VoltageStability(struct data_frame *df):			 					*/
/* ----------------------------------------------------------------------------		*/

void VoltageStability(struct data_frame *df,ENTRY *ent) { // for fixed point the phunits are not considered

//	printf("Inside VoltageStability\n");
	unsigned int lPDCID,pmuID;
	unsigned int numOfPMUs,i,pIndex;
	ENTRY entry;
	ENTRY *found=NULL;
	ENTRY *found1;
	unsigned char format;
	struct MapPMUToPhasor *tempMapPMUToPhasor;
	struct Phasor *phasor;
	
	unsigned char *fp_angle,*fx_angle,polar,*d;
	float angle,threshold;

	fp_angle = malloc(5*sizeof(unsigned char));
	fx_angle = malloc(3*sizeof(unsigned char));	

	while(df!=NULL) { // populate the hash table with actual phasor values

		lPDCID = to_intconvertor(df->idcode);
		numOfPMUs = df->num_pmu;
		for(i = 0;i<numOfPMUs;i++) {

			pmuID = df->dpmu[i]->idcode;
			memset(pmuIDKey,'\0',10);			
			sprintf(pmuIDKey,"%d",pmuID);
        	pmuKey = pmuIDKey;
			entry.key = pmuKey;						
			hsearch_r( entry,FIND, &found,&hashForMapPMUToPhasor);	
			
			if(found != NULL) {

				tempMapPMUToPhasor = ((struct MapPMUToPhasor *)found->data);
				format = tempMapPMUToPhasor->format;
				int nn = ((struct MapPMUToPhasor *)found->data)->numOfPhasors;
				int ll =0;
				while(ll<nn) {

					pIndex = ((struct MapPMUToPhasor *)found->data)->PhasorIndex[ll];
					d = df->dpmu[i]->phasors[pIndex];
					if((format & 0x20) == 0x20) { // floating

						d += 4;
						copy_cbyc (fp_angle,d,4);
						fp_angle[4] = '\0';
						angle = decode_ieee_single(fp_angle);					

					} else {

						d += 2;
						copy_cbyc (fx_angle,d,2);
						fx_angle[2] = '\0';
						angle = to_intconvertor(fx_angle);					
					}

					memset(appKey,'\0',50);					
					sprintf(appKey,"%d%d%d%s",1,lPDCID,pmuID,((struct MapPMUToPhasor *)found->data)->PhasorName[ll]);
					//printf("Key %s\n",appKey );
					key = appKey;
					entry.key = key;
					hsearch_r( entry,FIND, &found1,&hashForVSAppDetails);	
					if(found1 != NULL) {

						((struct VSAppDetails *)found1->data)->Phasor_Value = angle;
					} else {
						//printf("No key %s some problem PhasorName %s\n",appKey,((struct MapPMUToPhasor *)found->data)->PhasorName[pIndex]);
					}
					
					ll++;
				}
			}
		}
		df = df->dnext;
	} // while to populate phasor value ends
	
	// Angle difference application
	float angleDiff,angle1,angle2;
	threshold = ((struct VSAppInfo *)ent->data)->threshold;
	
	int noOfApps = ((struct VSAppInfo *)ent->data)->noOfApps;	
	//int noOfApps = 14;	
	ENTRY *found2,*found3;	
	for(i =0;i<noOfApps;i++) {

		entry.key = VSHashKeyList.key[i];
		hsearch_r(entry,FIND, &found2,&hashForVSAppDetails);	
		if(found2 != NULL)
		{
			entry.key = ((struct VSAppDetails *)found2->data)->peerHashKey;
			hsearch_r(entry,FIND, &found3,&hashForVSAppDetails);	
			angle1 = ((struct VSAppDetails *)found2->data)->Phasor_Value;
			angle2 = ((struct VSAppDetails *)found3->data)->Phasor_Value;
			if(fabs(angle1-angle2+TWOPI) > threshold){
			//	printf("violation of threshold\n");
				;
			}
		}
	}
}


/* ----------------------------------------------------------------------------		*/
/* FUNCTION  CoherencyOfGenerators():								 				*/
/* ----------------------------------------------------------------------------		*/

void CoherencyOfGenerators() { // for fixed point the phunits are not considered

	//printf("Inside CoherencyOfGenerators\n");

	struct CCG *ccgApp = firstCCGApp;
	int i,j,k,pmuID,n;
	float groupCOI =0,globalCOI = 0,H = 0,temp;
	
	ENTRY ccgEntry;
	ENTRY *ccgFound;

//	printf("ccgApp->numOfModes %d\n", ccgApp->numOfModes);	

	for (i = 0;i<ccgApp->numOfModes;i++) {

		 for (j = 0; j < ccgApp->numOfLPDC; ++j)
		{
			n = ccgApp->lPDC[j]->groupDetails[i]->numOfGenerators;
			groupCOI = 0;
			for (k = 0;k<n;k++) { // n PMUs in each group

				pmuID = ccgApp->lPDC[j]->groupDetails[i]->PMU_ID[k];
				memset(appKey,'\0',50);
				sprintf(appKey,"%d%d",pmuID,ccgApp->phasorIndex[i]);				
				ccgEntry.key = appKey;
				hsearch_r(ccgEntry,FIND, &ccgFound,&CGGhash);				
				if(ccgFound != NULL) 
					groupCOI += ((struct CGGMapPMUNPhasor *)ccgFound->data)->angle*180/M_PI;									
			}
			groupCOI = groupCOI/(ccgApp->lPDC[j]->groupDetails[i]->Hg);
			ccgApp->lPDC[j]->groupDetails[i]->groupCOI = groupCOI;
			//groupCOI = 0;
			H += ccgApp->lPDC[j]->groupDetails[i]->Hg;
			globalCOI += groupCOI*ccgApp->lPDC[j]->groupDetails[i]->Hg;
			globalCOI = globalCOI/H;
			ccgApp->globalCOI[i] = globalCOI;		
		}		
	}
	
	int p = 0;
	for (i = 0;i<ccgApp->numOfModes;i++) {

		for (j = 0; j < ccgApp->numOfLPDC; ++j) {

			temp = ccgApp->globalCOI[i] - ccgApp->lPDC[j]->groupDetails[i]->groupCOI;
			if(temp>180)
				temp = fabs(360-temp);
			else if(temp<-180)
				temp = fabs(360+temp);
			
			if( temp > ccgApp->threshold[p++]) {
				ccgApp->lPDC[j]->groupDetails[i]->violation = true;
				//printf("Global COI %f Group COI %f threshold %f deviation %f\n",ccgApp->globalCOI[i], ccgApp->lPDC[j]->groupDetails[i]->groupCOI,ccgApp->threshold[i],temp);
			}
		}
	}

	// Reset the violation flag to false
	for (i = 0;i<ccgApp->numOfModes;i++) 
		for (j = 0; j < ccgApp->numOfLPDC; ++j) 		
				ccgApp->globalCOI[i],ccgApp->lPDC[j]->groupDetails[i]->violation = false;
}
