/* ----------------------------------------------------------------------------- 
 * applications.h
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

#include <search.h>
#include <stdbool.h>

//#define TAB 4

int total_buses;
int listed_buses;
float* buses_power;
int TotApps;

int TWOPI;



struct VSAppInfo {

	int appID;
	int SPDCID;
	int noOfLPDC;
	int *LPDCID;
	int noOfApps;
	int threshold;
	struct VSAppInfo *appNext;
}*appFirst;

struct VSAppDetails
{
	int option;
	int appID;
	int LPDC_ID;
	int PMU_ID;
	char *PhasorName;
	float Phasor_Value;
	char *peerHashKey;
};


struct HashKeyList
{
	char **key;	
};

// struct PMUIDHashKeyList
// {
// 	unsigned int idcode;
// 	struct PMUIDHashKeyList *next;
// };

struct MapPMUToPhasor
{
	unsigned int Idcode;
	char **PhasorName;
	unsigned int *PhasorIndex; // index of the phasor in actual data frame
	int *DDS2_Phasor_Index;
	int numOfPhasors;
	unsigned char format;	

	unsigned int startIndex; /* To create a DDS2 data frame */
	unsigned int dataFrameSize;
	bool visited;
	unsigned int nextIdcode;
	unsigned int prevIdcode;

};

struct EachGroupDetails // PMUs under each group
{
	int numOfGenerators;
	int *PMU_ID;	
	float Hg;
	float groupCOI;
	bool violation;
	
};


struct CCG
{
	int appID;
	int SPDCID;
	int numOfLPDC;
	int numOfModes;
	int *lPDCID;
	float *mode;	
	float *threshold;
	int *phasorIndex;
	float *globalCOI;

	struct ForEachLPDC {
				
		struct EachGroupDetails **groupDetails; // Groups under each LPDC

	}**lPDC;

}*firstCCGApp;


struct CGGMapPMUNPhasor // Coherent Group of generators Maps PMUID+PhasorIndex to Phasor Value
{
	float angle;
};


struct LSEIndex { 

	int *indexInPMU;
	int *indexInZmeas;

};

struct MapCGGPMUID2PhasorIndex {

	int index;
	unsigned char format;	
	unsigned int startIndex; /* To create a DDS2 data frame */
	unsigned int dataFrameSize;
	bool visited;

};


struct hsearch_data hashForVSApp,hashForVSAppDetails,hashForMapPMUToPhasor,CGGhash,LSEZhash, cggHashForPhasorIndex;

size_t max_element_of_VSAppInfo;  // of elements in search table
size_t max_element_of_Apps_Details; // of elements in search table
size_t max_element_of_MapPMUToPhasor;
size_t max_element_of_CGG;
size_t max_element_of_LSE;
size_t max_element_of_cggHashForPhasorIndex;


char appKey[50],*key,pmuIDKey[10];
struct HashKeyList VSHashKeyList;

bool angleDiffApp,coherencyOfGen;
int maxPhasors;

void hash_add_element_to_VSAppTable(char * elmt_key,struct VSAppInfo *elmt_data, struct hsearch_data * table);
void hash_add_element_to_VSAppDetailTable(char * elmt_key, struct VSAppDetails *elmt_data,struct hsearch_data * table); 
void populateVSApp(struct VSAppInfo VSAppInfo[],int index,char *d1);
void populateVSAppDetails(struct VSAppInfo *VSAppInfo,struct VSAppDetails *VSAppDetails, char *d1);

void hash_add_element_to_MapPMUToPhasor(char * elmt_key,struct MapPMUToPhasor *elmt_data,struct hsearch_data * table);
void hash_update_element_to_MapPMUToPhasor(ENTRY *found,char *key,char PhasorName []);
void hash_update_Index_N_NextID_to_MapPMUToPhasor(ENTRY *found,int nextStartIndex);

void readInertiaConstant();
void populateCGG(struct CCG *ccg);
void hash_add_element_to_CCGTable(char * elmt_key,struct CGGMapPMUNPhasor *elmt_data,struct hsearch_data * table);
void hash_update_element_to_CCGTable(ENTRY *found,float angle);

void hash_add_element_to_MapCGGPMUID2PhasorIndex(char * elmt_key,struct MapCGGPMUID2PhasorIndex *elmt_data,struct hsearch_data * table);