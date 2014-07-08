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

int numAppRunning;
bool angleDiffApp,coherencyOfGen,stateEstimation;


// State Estimator
#include <float.h>
#include <math.h>
#include <assert.h>
#include  <pthread.h>

#include <gsl/gsl_matrix.h>
#include <gsl/gsl_blas.h>
#include <gsl/gsl_complex.h>
#include <gsl/gsl_complex_math.h>
#include <gsl/gsl_linalg.h>
#include <gsl/gsl_linalg.h>
#include <sys/time.h>

pthread_mutex_t mutex_App2;
struct App2_Info{
	int app_id;
	
	// Matrixes 
	gsl_matrix_complex * Mpinv;
	struct sparseMatrix *MpinvsparseHead;
	struct sparseMatrix *MpinvsparseTail;
	
	gsl_matrix_complex * Z;
	struct hsearch_data hashForZmatrixIndex;			
	
	gsl_matrix_complex * Vhat;
	
}* app2;


struct sparseMatrix{
	int x;
	int y;
	gsl_complex value;
	struct sparseMatrix* next;
};

void createSparse(struct sparseMatrix **sparsehead, struct sparseMatrix **sparsetail, gsl_matrix_complex * Mat);
void multsparse(struct sparseMatrix* sparsehead, gsl_matrix_complex * onedMat, gsl_matrix_complex * outMat);
/// End OF State Estimator




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

struct MapPMUToPhasor
{
	char **PhasorName;
	int *PhasorIndex;
	int numOfPhasors;
	int format;	
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

struct hsearch_data hashForVSApp,hashForVSAppDetails,hashForMapPMUToPhasor,CGGhash,LSEZhash;

size_t max_element_of_VSAppInfo;  // of elements in search table
size_t max_element_of_Apps_Details; // of elements in search table
size_t max_element_of_MapPMUToPhasor;
size_t max_element_of_CGG;
size_t max_element_of_LSE;

char appKey[50],*key,pmuIDKey[10];
struct HashKeyList VSHashKeyList;
//struct PMUIDHashKeyList *AngleDiffIDKeyList;

int maxPhasors;

void hash_add_element_to_VSAppTable(char * elmt_key,struct VSAppInfo *elmt_data, struct hsearch_data * table);
void hash_add_element_to_VSAppDetailTable(char * elmt_key, struct VSAppDetails *elmt_data,struct hsearch_data * table); 
void populateVSApp(struct VSAppInfo VSAppInfo[],int index,char *d1);
void populateVSAppDetails(struct VSAppInfo *VSAppInfo,struct VSAppDetails *VSAppDetails, char *d1);
void hash_add_element_to_MapPMUToPhasor(char * elmt_key,struct MapPMUToPhasor *elmt_data,struct hsearch_data * table);
void hash_update_element_to_MapPMUToPhasor(ENTRY *found,char *key,char PhasorName []);
void readInertiaConstant();
void populateCGG(struct CCG *ccg);
void hash_add_element_to_CCGTable(char * elmt_key,struct CGGMapPMUNPhasor *elmt_data,struct hsearch_data * table);
void hash_update_element_to_CCGTable(ENTRY *found,float angle);
