/* ----------------------------------------------------------------------------- 
 * global.h
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


#include  <pthread.h>
#include  <netinet/in.h>
#include  <stdio.h>
#include <stdbool.h>
#include <limits.h>

#define MAX_SPDCBUFF_SIZE 40000 // Swadesh Modified
int finalFramFlag;

#define MAX_STRING_SIZE 7000 // Kedar modified from 5000 on 2013-07-15 16:25:09 
#define MAXBUFLEN 40000//19000  //4000//16000  // Kedar added from parser.h on 2013-07-16 17:06:54

/* ---------------------------------------------------------------- */
/*                    On The Run Processing                         */
/* ---------------------------------------------------------------- */


// DSE
#define Datarate 50
struct hsearch_data hash_app2latencies;
struct app2latencies{
	long int first_arrival_lpdc;
	long int Min_pmutolpdc;
};

long int app2_prviousTS,app2_currentTS;
int app2_sequence;

int fake;
int flagapp2;

pthread_mutex_t mutex_app2_Analysis;
long int app2_first_arrival_lpdc;
long int app2_Min_pmutolpdc;
long int app2_Min_pmutolpdcdelay_sum;
long int app2_DSE_time_sum;
long int app2_DSE_count;

// For Application7 analysis
//pthread_mutex_t mutex_Analysis;
//int flagapp1;
long int first_arrival_lpdc7;
long int Min_pmutolpdc7;
long int Min_pmutolpdcdelay_sum7;
long int count_patial_computation7;

//long int computation_time_sum;

// For Aggregated Dispatch and Distributed Partial Computation
pthread_mutex_t mutex_Analysis;
int flagapp1;
long int first_arrival_lpdc;
long int Min_pmutolpdc;
long int Min_pmutolpdcdelay_sum;
long int count_patial_computation;
long int computation_time_sum;

	
// For Early Dispatch and DQT
struct hsearch_data hashForAnalysis;
struct Analysis{
	unsigned long int current_fsec;
	unsigned long int arrive_fsec;
	
	int pckt_count;
	
	long int pmutolpdcdelay_sum;
	long int parsetime_sum;
	long int appRuntime_sum;
	long int alligntime_sum;
	long int pdcDelay_sum;
	
};


int flagapp6;

int app6PmuCount;
int app6CurrentPmuCount;
int* app6Pmuids;

long int lpdctospdc_latencysum;
int lpdctospdc_pktcount;

unsigned long int modelcreationtime_sum;
int modelcreate_count;

pthread_mutex_t mutex_App6Analysis;
long int total_pkt_arrivals;
long int total_local_violation;
long int total_filtered;
long int total_globle_violation;

pthread_mutex_t mutex_App7Analysis;
long int total_pkt_arrivals7;

long int lpdctospdc_latencysum7 ;
long int lpdctospdc_pktcount7;
int flagapp7;
long int app7pktsent;
long int Min_lpdctospdc7;
long int Min_lpdctospdcdelay_sum7;
long int dataFrameCreateTime;
/* ---------------------------------------------------------------- */
/*                         global variables                         */
/* ---------------------------------------------------------------- */

pthread_mutex_t mutex_cfg;  /* To lock cfg data objects */
pthread_mutex_t mutex_Lower_Layer_Details;  /* To lock objects of connection table that hold lower layer PMU/PDC ip and protocol */
pthread_mutex_t mutex_Upper_Layer_Details;  /* To lock objects of connection table that hold upper layer PDC ip and protocol */
pthread_mutex_t mutex_status_change;
pthread_mutex_t mutex_on_TSB;
pthread_mutex_t mutex_on_TSBk;//mayur change
pthread_mutex_t mutex_on_thread; // Added by KK on 19-Oct-2013

unsigned char *cfgframe,*dataframe;

struct sockaddr_in UDP_my_addr,TCP_my_addr; /* my address information */
struct sockaddr_in UL_UDP_addr,UL_TCP_addr; /* connector’s address information */
int UL_UDP_sockfd,UL_TCP_sockfd; /* socket descriptors */
pthread_t UDP_thread,TCP_thread,p_thread;

FILE *fp_log,*fp_updc,*f;
char tname[20];
char *dLog;

pthread_mutex_t mutex_logTimeBuff; // Added by swadesh 27 Jan 2014

/* --------------------------------------------------------------------	*/
/*				global DataBase variables		*/
/* --------------------------------------------------------------------	*/

struct sockaddr_in DB_Server_addr; // DB Address Information
int DB_sockfd,DB_addr_len;

int PDC_IDCODE,TCPPORT,UDPPORT;
char dbserver_ip[20];

unsigned char DATASYNC[3],CFGSYNC[3],CMDSYNC[3],CMDDATASEND[3],CMDDATAOFF[3],CMDCFGSEND[3];

// Kedar 08-07-13
int old_fsize;

// Kedar 22-09-2013

unsigned int COMBINE_DF_SIZE;
// Added by KK on 22/11/2013:10:46
bool LPDCApp,SPDCApp;
bool ALL_DATA,ANG_STAB,STATE_EST,COH_GEN;
int app6;


/**************************************** End of File *******************************************************/
