/* ----------------------------------------------------------------------------- 
 * align_sort.h
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


#define MAXTSB 10
#define OLDFRAMECOUNT 10
#define MAXTSBk 5
#define OLDFRAMECOUNTk 5

/* ---------------------------------------------------------------- */
/*                         global variable                          */
/* ---------------------------------------------------------------- */

extern int front;
extern int rear;
extern int Tsb_Count;

/* ---------------------------------------------------------------- */
/*                         global Data Structure                    */
/* ---------------------------------------------------------------- */

/* Data Structure for Time Stamp Buffer */
struct TimeStampBuffer {

	char *soc;
	char *fracsec;
	int count; // Current index
	int num; // Total Number of PMUs
	int counter; // Timer that will be initially set to 0
	int ready_to_send;
	struct pmupdc_id_list *idlist;
	struct data_frame *first_data_frame;

}TSB[MAXTSB],TSBk[MAXTSBk];

struct pmupdc_id_list {

	char *idcode;
	int num_pmu;
	struct pmupdc_id_list *nextid;
};

// Kedar K on 8/6/2013
struct  oldDataFramesTimeStampList
{
	unsigned int soc;
	unsigned int fracsec;

}ODFT[OLDFRAMECOUNT],ODFTk[OLDFRAMECOUNTk];

extern int old_df_front;
extern int old_df_rear;

/* ---------------------------------------------------------------- */
/*                 	Function prototypes   		            */
/* ---------------------------------------------------------------- */

void time_align(struct data_frame *df);

void assign_df_to_TSB(struct data_frame *df,int index);

void dispatch(int index);

//Below functions are added for Application7 (ALL_DATA)
void time_alignk(struct data_frame *df);

void assign_df_to_TSBk(struct data_frame *df,int index);

void dispatchk(int index);
void clear_TSBk(int index);
void adjustOldFrameCountk(int index);
int create_dataframek(int index);
void intermediate_dispatchk(int xx);


void clear_TSB(int index);

int create_dataframe(int index);

void checkForMissingFrames(ENTRY*);

int create_cfgframe(struct  cfg_frame *);

void sort_data_inside_TSB(int index);

struct data_frame* generate_dummy_dataframe(unsigned char *);

void adjustOldFrameCount(int index);

void intermediate_dispatch(int xx);

int matchDataFrameTimeToTSBTime(unsigned int df_soc,unsigned int df_fracsec);

void attacheEachDataFrame(struct data_frame *temp_df,int z);
/**************************************** End of File *******************************************************/
