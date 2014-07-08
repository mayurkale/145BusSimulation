/* ----------------------------------------------------------------------------- 
 * ServerFunction.h
 * 
 * PMU Simulator - Phasor Measurement Unit Simulator
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



#define TB 1000000 //6777216 // roundf(powf(2,24) - 1);


/* ---------------------------------------------------------------- */
/*                         global variables                         */
/* ---------------------------------------------------------------- */

#define MAX_STRING_SIZE 500

//int start_id, pmu_count;

int pmu_count;
int dup_total_buses;

int *pmuids;

FILE ** fd_csv;

unsigned char** cfg_frame;
unsigned char** data_frame;

int* data_frame_size,*cfg_frame_size;
int * UDP_sockfd;



struct sockaddr_in* UDP_my_addr; /* my address information */


//struct sockaddr_in* UDP_addr; // connector’s address information
//int * UDP_addr_len;


int Max_dup_allowed;
int* candidates_count;
int ** UDP_addr_lengths;
struct sockaddr_in** UDP_addresses; // connectors address information


int send_cfg_count;


struct sigaction sa;
pthread_cond_t cond;


int div_count;
pthread_cond_t* sending;
pthread_mutex_t* mutex_sending;
struct sender_data{
int index, start, end;
};


pthread_mutex_t mutex_data;  /* To manage data ON/OFF */



long int soc, fracsec, fsecNum;
uint16_t chk;	// For checksum calculation

struct timeval tim;



/* ------------------------------------------------------------------ */
/*                       Global Datastructure                         */
/* ------------------------------------------------------------------ */

struct ConfigurationFrame 
{
	int  cfg_pmuID; 
	int  cfg_fdf; 
	int  cfg_af; 
	int  cfg_pf; 
	int  cfg_pn; 
	int  cfg_phnmr_val; 
	int  cfg_annmr_val; 
	int  cfg_dgnmr_val; 
	int  cfg_fnom; 
	int  cfg_dataRate; 
	char *cfg_STNname; 
	char *cfg_phasor_channels; 
	char *cfg_analog_channels; 
	char *cfg_digital_channels; 
	unsigned char *cfg_phasor_phunit; 
	unsigned char *cfg_analog_anunit; 
	
};

struct ConfigurationFrame **cfg_infos;

/* ------------------------------------------------------------------ */
/*                       Function prototypes                          */
/* ------------------------------------------------------------------ */

void auto_create_cfg(int pmuID, int Frequency);

int csv_create_cfg(int pmuID, char *filePath);

void generate_data_frame(int key);

/**************************************** End of File *******************************************************/
