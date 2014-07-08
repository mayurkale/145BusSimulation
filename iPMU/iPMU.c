#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <pthread.h>
#include <errno.h>
#include <math.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/signal.h>
#include <unistd.h>
#include "iPMU.h"
#include "function.h"
#include "connection.h"

pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex_data = PTHREAD_MUTEX_INITIALIZER;

int pmuse = 0;
long int prev_soc = 0;

float phasorV_meg[] = {63.48,63.50,63.52};	// Voltage nominal as 63.5V
float phasorI_meg[] = {0.998,1.0,0.994};	// Current nominal as 1.0Amp
float phase_ang[] 	= {-120.6,0.65,120.45}; 
double degreeToRad;




/* ----------------------------------------------------------------------------	*/
/* FUNCTION  generate_data_frame():	               					*/
/* Function to generate the data frame. Based on the Configuration Frame 	     */
/* attributes.                          					               */
/* ----------------------------------------------------------------------------	*/

void generate_data_frame(int key)
{

	float valf;
	long int vall;
	int k,indx=0,vali;
     	unsigned char temp1[1],temp2[2];
     	unsigned char temp3[3],temp4[4];
     
     	struct timespec *cal_timeSpec;
     	cal_timeSpec = malloc(sizeof(struct timespec));
//     	cal_timeSpec1 = malloc(sizeo(struct timespec));

	memset(data_frame[key],'\0',sizeof(data_frame_size[key]));

	/* Insert SYNC Word in data frame */
	data_frame[key][indx++] = 0xAA; 
	data_frame[key][indx++] = 0x01; 

	/* Insert data frame size in data frame */
	i2c(data_frame_size[key],temp2);
	B_copy(data_frame[key],temp2,indx,2);
	indx = indx + 2;

	/* Insert PMU ID in data frame */
	i2c(cfg_infos[key]->cfg_pmuID,temp2);
	B_copy(data_frame[key],temp2,indx,2);
	indx = indx + 2;

	/* Insert SOC value in data frame */
     	/* No PPS,so have to manage by seeing local time */
     	clock_gettime(CLOCK_REALTIME,cal_timeSpec);

//printf("cal_timeSpec->tv_sec = %ld \n",cal_timeSpec->tv_sec);
 
//if(key == 0){
	if(key == 0){	
		if (fsecNum >= cfg_infos[key]->cfg_dataRate)
		{	
			cal_timeSpec->tv_sec ++;
			fsecNum = 0;
		}
	//}
		soc = (long)cal_timeSpec->tv_sec;
		fracsec = roundf(fsecNum*TB/cfg_infos[key]->cfg_dataRate);
	}
	
	li2c(soc,temp4);
	B_copy(data_frame[key],temp4,indx,4);
	indx = indx + 4;

     	/* Insert Time Quality flag of one byte,default is zero,user setable in future? */
	temp1[0] = 0;
	B_copy(data_frame[key],temp1,indx,1);
	indx = indx + 1;

     	/* Insert fraction of second in data frame of three bytes */
     	
	li2c_3byte(fracsec,temp3);
	B_copy(data_frame[key],temp3,indx,3);
	indx = indx + 3;

	if(key == pmu_count-1){
		fsecNum += 1;   
	//	printf("PMUID: %d, SOC : %ld, FRACSEC : %ld\n ",cfg_infos[key]->cfg_pmuID,soc,fracsec);
	}

	/* Insert STAT Word in data frame Default or Changed */
	time_t curnt_soc = time(NULL);

	if(pmuse == 0) 
	{
		prev_soc = curnt_soc;
	} 

	if((curnt_soc-prev_soc) > 1)
	{ 
		printf("\tSTAT word Changed due to PMU SYNC Error.");
		data_frame[key][indx++] = 0x20;
		data_frame[key][indx++] = 0x00;
	}
	else
	{
		/* If not insert default STAT Word: 0000 */
		data_frame[key][indx++] = 0x00;
		data_frame[key][indx++] = 0x00;
	}

	prev_soc = curnt_soc;
	pmuse = 1;


	char *mData,*d1,*d2;
	int j;

	mData = measurement_Return (key);
/*printf("tupple = %s\n", mData);		*/
		
	d1 = strtok (mData,","); 
	d1 = strtok (NULL,","); 
	//printf("start = %s\n", d1);
	/* Fix Point Phasor Measurements */
	
		if(cfg_infos[key]->cfg_pf == 0)		
		{
/*		printf("I am here\n");*/
			for(j=0,k=0; j<cfg_infos[key]->cfg_phnmr_val; j++)
			{
				k++;
				temp3[0] = cfg_infos[key]->cfg_phasor_phunit[k++];
				temp3[1] = cfg_infos[key]->cfg_phasor_phunit[k++];
				temp3[2] = cfg_infos[key]->cfg_phasor_phunit[k++];
				vall = c2li_3byte(temp3);

				d1 = strtok (NULL,","); 
				vali = atof(d1)*vall;
				i2c(vali,temp2);
				B_copy(data_frame[key],temp2,indx,2);
				indx = indx + 2;

				d2 = strtok (NULL,","); 
				vali = atof(d1)*vall;
				i2c(vali,temp2);
				B_copy(data_frame[key],temp2,indx,2);
				indx = indx + 2;
//printf("ph = %s-%s\n", d1, d2);
			}
		}
		else	      /* Floating Point Phasor */
		{
/*		printf("I am not here\n");*/
			for(j=0,k=0; j<cfg_infos[key]->cfg_phnmr_val; j++)
			{
				k++;
/*				temp3[0] = cfg_infos[key]->cfg_phasor_phunit[k++];*/
/*				temp3[1] = cfg_infos[key]->cfg_phasor_phunit[k++];*/
/*				temp3[2] = cfg_infos[key]->cfg_phasor_phunit[k++];*/
/*				vall = c2li_3byte(temp3);*/

				d1 = strtok (NULL,","); 
				valf = atof(d1);
/*				valf = atof(d1)*vall;*/
				f2c(valf,temp4);
				B_copy(data_frame[key],temp4,indx,4);
				indx = indx + 4;
/*printf("amp = %g, after ampliitude = %g\n",atof(d1),valf);*/
				d2 = strtok (NULL,","); 
				// Modification by Swadesh Jain
				//valf = atof(d2)*vall;  
				valf = atof(d2);
				valf = valf*degreeToRad;			
		
/*				if(valf < 0) valf = valf + 360; // Changing into  positive angle if it is negative*/
				//valf = valf *(3.1415/180); // Convert into radians
				
				f2c(valf,temp4);
				B_copy(data_frame[key],temp4,indx,4);
				indx = indx + 4;
				
/*printf("pmuID = %d, angle = %g , after angle = %g\n",cfg_infos[key]->cfg_pmuID,atof(d2),valf);*/

			}
//printf("%d,%ld,%ld,%s,%s\n",cfg_infos[key]->cfg_pmuID,soc,fracsec,d1,d2);
/*printf("%d, %d\n",cfg_infos[key]->cfg_pmuID,cfg_infos[key]->cfg_phnmr_val);*/
		}

		/* Fix Point Frequency & DFrequency */
		if(cfg_infos[key]->cfg_fdf == 0)
		{
			if(cfg_infos[key]->cfg_fnom == 1)
				vali = 50;
			else
				vali = 60;
				 
			d1 = strtok (NULL,",");
			vali = (atof(d1)-vali)*1000;
			i2c(vali,temp2);
			B_copy(data_frame[key],temp2,indx,2);
			indx = indx + 2;

			d2 = strtok (NULL,","); 
			vali = (atof(d2)*100);
			i2c(vali,temp2);
			B_copy(data_frame[key],temp2,indx,2);
			indx = indx + 2;
//printf("FREQ = %s-%s\n\n\n", d1, d2);
		}
		else	      	/* Floating Point Frequency & DFrequency */
		{
			d1 = strtok (NULL,","); 
			valf = atof(d1);
			f2c(valf,temp4);
			B_copy(data_frame[key],temp4,indx,4);
			indx = indx + 4;

			d2 = strtok (NULL,","); 
			valf = atof(d2);
			f2c(valf,temp4);
			B_copy(data_frame[key],temp4,indx,4);
			indx = indx + 4;
			
//printf("FREQ = %s-%s\n\n\n", d1, d2);
		}

		/* Fix Point Analog */
		if(cfg_infos[key]->cfg_af == 0)
		{
			for(j=0,k=0; j<cfg_infos[key]->cfg_annmr_val; j++)
			{
				k++;
				temp3[0] = cfg_infos[key]->cfg_analog_anunit[k++];
				temp3[1] = cfg_infos[key]->cfg_analog_anunit[k++];
				temp3[2] = cfg_infos[key]->cfg_analog_anunit[k++];
				vall = c2li_3byte(temp3);

				d1 = strtok (NULL,",");
				vali = atof(d1)*vall;
				i2c(vali,temp2);
				B_copy(data_frame[key],temp2,indx,2);
				indx = indx + 2;
//printf("AN = %s\n", d1);
			}
		}
		else      /* Insert Floating point Analog values in data frame */
		{
			for(j=0,k=0; j<cfg_infos[key]->cfg_annmr_val; j++)
			{
				k++;
				temp3[0] = cfg_infos[key]->cfg_analog_anunit[k++];
				temp3[1] = cfg_infos[key]->cfg_analog_anunit[k++];
				temp3[2] = cfg_infos[key]->cfg_analog_anunit[k++];
				vall = c2li_3byte(temp3);

				d2 = strtok (NULL,","); 
				valf = (atof(d2));
				f2c(valf,temp4);
				B_copy(data_frame[key],temp4,indx,4);
				indx = indx + 4;
			}
		}
	
	
	/* Calculate and insert the Checksum value in data frame (till now) */
	chk = compute_CRC(data_frame[key],indx);

     data_frame[key][indx++] = (chk >> 8) & ~(~0<<8);  	/* CHKSUM high byte; */
     data_frame[key][indx++] = (chk ) & ~(~0<<8);     	/* CHKSUM low byte;  */

/*	printf("\nData Frame Size = %d : ",indx);
	for (i=0; i<indx; i++)
	{
		printf("%X-",data_frame[key][i]);
	}
*/
} // End of generate_data_frame;


/***************************************************************************************
*******************************************************************/

int csv_cfg_create(int pmuID, FILE *fp_csv, int key)
{
	int  tempi,i,j,k;
	int indx,frmt;
	char *d1,*rline=NULL;
	char stn[16],buff[10],c=' ';
	unsigned char temp1[1],temp2[2];
	unsigned char hex,temp3[3],temp4[4];

	size_t len = 0;
	ssize_t read;

	/* Open the saved PMU Setup File and read the CFG frame if any? */
	//FILE *fp_csv = fopen (filePath,"r");

	if (fp_csv != NULL)
	{
		read = getline(&rline,&len,fp_csv);	// For "Configuration Fields"
		read = getline(&rline,&len,fp_csv); // For "Configuration Items"

		if(read == 0)
			return 1;	// Return 1 if file is empty?
		else
		{
			/* Allocate the memory for the ConfigurationFrame object and assigned fields */
			cfg_infos[key]->cfg_pmuID = pmuID;
			
			cfg_infos[key]->cfg_STNname = malloc(16);
			memset(cfg_infos[key]->cfg_STNname,'\0',16);
			strcpy(stn,"iPMU-");
			sprintf(buff,"%d",cfg_infos[key]->cfg_pmuID);
			strcat(stn,buff);
			for (i=strlen(stn);i<16;i++)
				stn[i] = c;
			strncpy(cfg_infos[key]->cfg_STNname,stn,16);

			d1 = strtok (rline,",");
			//printf("\n%s.\niPMU ID = %d\nStation Name = %s\n",d1,cfg_infos[key]->cfg_pmuID,cfg_infos[key]->cfg_STNname);

			d1 = strtok (NULL,",");
			tempi = atoi(d1);
			if(tempi == 50)
				cfg_infos[key]->cfg_fnom = 1;
			else
				cfg_infos[key]->cfg_fnom = 0;
			//printf("Frequency = %d\n",tempi);

			d1 = strtok (NULL,",");
			tempi = atoi(d1);
			cfg_infos[key]->cfg_dataRate = tempi;
			//printf("Data Rate = %d\n",tempi);

			d1 = strtok (NULL,",");
			tempi = atoi(d1);
			cfg_infos[key]->cfg_phnmr_val = tempi;
			//printf("Phasors = %d\n",tempi);
		
			d1 = strtok (NULL,",");
			tempi = atoi(d1);
			cfg_infos[key]->cfg_annmr_val = tempi;
			//printf("Analogs = %d\n",tempi);

			d1 = strtok (NULL,",");
			tempi = atoi(d1);
			cfg_infos[key]->cfg_dgnmr_val = tempi;
			//printf("Digitals = %d\n",tempi);

			// Identify the data format polar,rectangular,floating/fixed point?	
			d1 = strtok (NULL,",");
			frmt = atoi(d1);
			i2c(frmt,temp2);
			hex = temp2[1];

			hex <<= 4;
			if((hex & 0x80) == 0x80) cfg_infos[key]->cfg_fdf = 1; else cfg_infos[key]->cfg_fdf = 0;
			if((hex & 0x40) == 0x40) cfg_infos[key]->cfg_af  = 1; else cfg_infos[key]->cfg_af  = 0;
			if((hex & 0x20) == 0x20) cfg_infos[key]->cfg_pf  = 1; else cfg_infos[key]->cfg_pf  = 0;
			if((hex & 0x10) == 0x10) cfg_infos[key]->cfg_pn  = 1; else cfg_infos[key]->cfg_pn  = 0;	
			
			/* Allocate the memory for Phasor channel names */
			cfg_infos[key]->cfg_phasor_channels = (char *)malloc((cfg_infos[key]->cfg_phnmr_val*16) * sizeof(char));
			memset(cfg_infos[key]->cfg_phasor_channels,'\0',sizeof(cfg_infos[key]->cfg_phasor_channels));	

			/* Allocate the memory for Phasor channels PHUNIT */
			cfg_infos[key]->cfg_phasor_phunit = (unsigned char *)malloc((cfg_infos[key]->cfg_phnmr_val*4) * sizeof(unsigned char));
			memset(cfg_infos[key]->cfg_phasor_phunit,'\0',sizeof(cfg_infos[key]->cfg_phasor_phunit));	

			for (i=0,k=0; i<cfg_infos[key]->cfg_phnmr_val; i++)
			{
				d1 = strtok (NULL,",");
				memset(stn,'\0',16);
				strcpy(stn,d1);
				j = strlen(stn);
				
				for (; j<16; j++)
				{
					stn[j] = c;
				}
				strncat(cfg_infos[key]->cfg_phasor_channels,stn,16);						

				d1 = strtok (NULL,",");
				if (!strcasecmp(d1,"v"))
					cfg_infos[key]->cfg_phasor_phunit[k++] = 0x00; // Voltage
				else
					cfg_infos[key]->cfg_phasor_phunit[k++] = 0x01; // Current
				
				d1 = strtok (NULL,",");
				li2c_3byte (atol(d1),temp3);
				cfg_infos[key]->cfg_phasor_phunit[k++] = temp3[0];
				cfg_infos[key]->cfg_phasor_phunit[k++] = temp3[1];
				cfg_infos[key]->cfg_phasor_phunit[k++] = temp3[2];
			}
			
			/* Allocate the memory for Analog channel names */
			cfg_infos[key]->cfg_analog_channels = (char *)malloc((cfg_infos[key]->cfg_annmr_val*16) * sizeof(char));
			memset(cfg_infos[key]->cfg_analog_channels,'\0',sizeof(cfg_infos[key]->cfg_analog_channels));	

			/* Allocate the memory for Analog channels ANUNIT */
			cfg_infos[key]->cfg_analog_anunit = (unsigned char *)malloc((cfg_infos[key]->cfg_annmr_val*4) * sizeof(unsigned char));
			memset(cfg_infos[key]->cfg_analog_anunit,'\0',sizeof(cfg_infos[key]->cfg_analog_anunit));	

			for (i=0,k=0; i<cfg_infos[key]->cfg_annmr_val; i++)
			{
				d1 = strtok (NULL,",");
				memset(stn,'\0',16);
				strcpy(stn,d1);
				j = strlen(stn);
				
				for (; j<16; j++)
				{
					stn[j] = c;
				}
				strncat(cfg_infos[key]->cfg_analog_channels,stn,16);						
				
				d1 = strtok (NULL,",");
				if (!strcasecmp(d1,"rms"))
					cfg_infos[key]->cfg_analog_anunit[k++] = 0x00; // RMS
				else if (!strcasecmp(d1,"pow"))
					cfg_infos[key]->cfg_analog_anunit[k++] = 0x01; // POW
				else
					cfg_infos[key]->cfg_analog_anunit[k++] = 0x10; // Peak
				
				d1 = strtok (NULL,",");
				li2c_3byte (atol(d1),temp3);
				cfg_infos[key]->cfg_analog_anunit[k++] = temp3[0];
				cfg_infos[key]->cfg_analog_anunit[k++] = temp3[1];
				cfg_infos[key]->cfg_analog_anunit[k++] = temp3[2];
			}
			
			/* Close the file as there is no more channels! */
			//fclose(fp_csv);

			/* Calculate the total size of CFG Frame */
			cfg_frame_size[key] = 0;
			cfg_frame_size[key] = 54 + (16*cfg_infos[key]->cfg_phnmr_val) + (16*cfg_infos[key]->cfg_annmr_val)+ (16*cfg_infos[key]->cfg_dgnmr_val*16) + (4*cfg_infos[key]->cfg_phnmr_val) + (4*cfg_infos[key]->cfg_annmr_val) + (4*cfg_infos[key]->cfg_dgnmr_val);

			/* Calculate the total size of Data Frame */
			data_frame_size[key] = 18; // Fixed fields size

			/* Calculate 4/8 bytes for each PHNMR */
			if (cfg_infos[key]->cfg_pf == 0) 
			{
				data_frame_size[key] = data_frame_size[key] + (4*cfg_infos[key]->cfg_phnmr_val);
			}
			else 
			{
				data_frame_size[key] = data_frame_size[key] + (8*cfg_infos[key]->cfg_phnmr_val);
			}

			/* Calculate 2/4 bytes for each ANNMR */
			if (cfg_infos[key]->cfg_af == 0) 
			{
				data_frame_size[key] = data_frame_size[key] + (2*cfg_infos[key]->cfg_annmr_val);
			}
			else 
			{
				data_frame_size[key] = data_frame_size[key] + (4*cfg_infos[key]->cfg_annmr_val);
			}

			/* Calculate 2/4 bytes for both (FREQ + DFREQ) */
			if (cfg_infos[key]->cfg_fdf == 0) 
			{				
				data_frame_size[key] = data_frame_size[key] + 4;
			}
			else 
			{
				data_frame_size[key] = data_frame_size[key] + 8;
			}

			/* Calculate 2 bytes for each DGNMR */
			data_frame_size[key] = data_frame_size[key] + (2*cfg_infos[key]->cfg_dgnmr_val);

			/* Insert the fields in new CFG Frame: */
			memset(cfg_frame[key],'\0',sizeof(cfg_frame[key]));

			/* sync word */
			indx = 0;
			cfg_frame[key][indx++] = 0xAA; 
			cfg_frame[key][indx++] = 0x31; 

			/* Frame Size */
			i2c(cfg_frame_size[key],temp2);
			B_copy(cfg_frame[key],temp2,indx,2);
			indx = indx + 2;

			/* PMU ID */
			i2c(cfg_infos[key]->cfg_pmuID,temp2);
			B_copy(cfg_frame[key],temp2,indx,2);
			indx = indx + 2;

			/* SOC */				
			indx = indx + 4;

			/* Fraction of Second */
			indx = indx + 4;

			/* Time Base is static 1000000μs */
			li2c(1000000,temp4);
			B_copy(cfg_frame[key],temp4,indx,4);
			indx = indx + 4;

			/* Number of PMU: static "0001" */
			i2c(1,temp2);
			B_copy(cfg_frame[key],temp2,indx,2);
			indx = indx + 2;

			/* Station Name */
			B_copy(cfg_frame[key],(unsigned char *)cfg_infos[key]->cfg_STNname,indx,16);
			indx = indx + 16;

			/* PMU ID */					
			i2c(cfg_infos[key]->cfg_pmuID,temp2);
			B_copy(cfg_frame[key],temp2,indx,2);
			indx = indx + 2;

			/* Format Word */
			i2c(frmt,temp2);
			B_copy(cfg_frame[key],temp2,indx,2);
			indx = indx + 2;

			/* PHNMR: Number of Phasors */
			i2c(cfg_infos[key]->cfg_phnmr_val,temp2);
			B_copy(cfg_frame[key],temp2,indx,2);
			indx = indx + 2;

			/* ANNMR: Number of Analogs */
			i2c(cfg_infos[key]->cfg_annmr_val,temp2);
			B_copy(cfg_frame[key],temp2,indx,2);
			indx = indx + 2;

			/* DGNMR: Number of Digitals */
			i2c(cfg_infos[key]->cfg_dgnmr_val,temp2);
			B_copy(cfg_frame[key],temp2,indx,2);
			indx = indx + 2;
			
			/* Phasor channel Names */ 
			B_copy(cfg_frame[key],(unsigned char *)cfg_infos[key]->cfg_phasor_channels,indx,16*cfg_infos[key]->cfg_phnmr_val);
			indx = indx + 16*cfg_infos[key]->cfg_phnmr_val;

			/* Analog channel Names */ 
			B_copy(cfg_frame[key],(unsigned char *)cfg_infos[key]->cfg_analog_channels,indx,16*cfg_infos[key]->cfg_annmr_val);
			indx = indx + 16*cfg_infos[key]->cfg_annmr_val;

			/* Digital channel Names */ 
			B_copy(cfg_frame[key],(unsigned char *)cfg_infos[key]->cfg_digital_channels,indx,16*cfg_infos[key]->cfg_dgnmr_val);
			indx = indx + 16*cfg_infos[key]->cfg_dgnmr_val;
			
			/* FACTOR VALUES for Phasor */
			B_copy(cfg_frame[key],cfg_infos[key]->cfg_phasor_phunit,indx,4*cfg_infos[key]->cfg_phnmr_val);
			indx = indx + 4*cfg_infos[key]->cfg_phnmr_val;
			
			/* FACTOR VALUES for Analog */
			B_copy(cfg_frame[key],cfg_infos[key]->cfg_analog_anunit,indx,4*cfg_infos[key]->cfg_annmr_val);
			indx = indx + 4*cfg_infos[key]->cfg_annmr_val;

			/* FACTOR VALUES for Digital */
/*			B_copy(cfg_frame[key],cfg_infos[key]->cfg_digital_dgunit,indx,4*cfg_infos[key]->cfg_dgnmr_val);
			indx = indx + 4*cfg_infos[key]->cfg_dgnmr_val; */
			
			/* Nominal Frequency */
			i2c(cfg_infos[key]->cfg_fnom,temp2);
			B_copy(cfg_frame[key],temp2,indx,2);
			indx = indx + 2;
			
			/* Configuration Count: Static '0000' */
			i2c(0,temp2);
			B_copy(cfg_frame[key],temp2,indx,2);
			indx = indx + 2;
			
			/* Data Rate */
			i2c(cfg_infos[key]->cfg_dataRate,temp2);
			B_copy(cfg_frame[key],temp2,indx,2);
			indx = indx + 2;
			
		   	/* SOC */
		   	gettimeofday(&tim,NULL);
		   	soc = (long) tim.tv_sec;
		   	li2c(soc,temp4);
		   	B_copy(cfg_frame[key],temp4,6,4);

		   	/* Time quality fields */
		   	temp1[0] = 0; 
		   	B_copy(cfg_frame[key],temp1,10,1);

		   	/* Fraction of Second */
		   	fracsec = tim.tv_usec;
		   	li2c(fracsec,temp3);
		   	B_copy(cfg_frame[key],temp3,11,3);

			/* CRC-Checksum (up to now) */
			chk = compute_CRC(cfg_frame[key],indx);
			cfg_frame[key][indx++] = (chk >> 8) & ~(~0<<8);  	/* CHKSUM high byte */
			cfg_frame[key][indx++] = (chk ) & ~(~0<<8);     	/* CHKSUM low byte  */

			//printf("CFG Frame size  = %d.\n",cfg_frame_size);
			//printf("Data Frame size = %d-Bytes.\n",data_frame_size);
			
		}		
	}
	else
	{
		printf("CSV File not present in the system!\n");
	}

	return 0;
}


int main(int argc,char **argv)
{
	if (argc != 2)
	{
		printf("Passing arguments does not match with the iPMU inputs. Try Again?\nSyntex : ./iPMU <PMU Info CSV files Path> \n");
		exit(EXIT_SUCCESS);
	}
	degreeToRad = M_PI/180;
	
	int i;
	char *line;
	char *d1, *svptr;
	size_t len = 0;

	FILE *f_info = fopen(argv[1],"r");
	
	getdelim (&line, &len, ('\n'), f_info); // Remove header
	getdelim (&line, &len, ('\n'), f_info);
	
	d1 = strtok_r(line,",",&svptr);
	pmu_count = atoi(d1);	

/*printf("%d\n",pmu_count);*/

	d1 = strtok_r(NULL,",",&svptr);
	char * pre = strdup(d1);
	
	d1 = strtok_r(NULL,",",&svptr);
	dup_total_buses = atoi(d1);
	
	d1 = strtok_r(NULL,",",&svptr);
	Max_dup_allowed = atoi(d1)+1;
	
	d1 = strtok_r(NULL,",",&svptr);
	int syncPort = atoi(d1);
	
/*printf("%s\n",pre);*/
	
	pmuids = (int*)malloc(pmu_count * sizeof(int));
	
	getdelim (&line, &len, ('\n'), f_info); // Remove header
	
	for(i=0; i<pmu_count; i++){
		getdelim (&line, &len, ('\n'), f_info);
		d1 = strtok_r(line,",",&svptr);
		pmuids[i] = atoi(d1);
/*printf("%d\n",pmuids[i]);*/
	}
	
	fd_csv = (FILE **) malloc(pmu_count * sizeof(FILE *));
	
	cfg_frame = (unsigned char**) malloc(pmu_count * sizeof(unsigned char*));	
	data_frame = (unsigned char**) malloc(pmu_count * sizeof(unsigned char*));
	cfg_infos = (struct ConfigurationFrame **) malloc(pmu_count * sizeof(struct ConfigurationFrame *));

	for(i=0; i<pmu_count; i++){
		cfg_frame[i] = (unsigned char*) malloc( MAX_STRING_SIZE * sizeof(unsigned char));
		data_frame[i] = (unsigned char*) malloc( MAX_STRING_SIZE * sizeof(unsigned char));
		cfg_infos[i] = (struct ConfigurationFrame *) malloc(sizeof(struct ConfigurationFrame));
	}
	
	data_frame_size = (int *) malloc(pmu_count * sizeof(int));
	cfg_frame_size = (int *) malloc(pmu_count * sizeof(int));
	UDP_sockfd = (int *) malloc(pmu_count * sizeof(int));
	
	
	UDP_my_addr = (struct sockaddr_in*) malloc(pmu_count * sizeof(struct sockaddr_in));
	
	UDP_addresses = (struct sockaddr_in**) malloc(pmu_count * sizeof(struct sockaddr_in*)); 
	UDP_addr_lengths = (int **) malloc(pmu_count * sizeof(int*));
	candidates_count = (int*)malloc(pmu_count * sizeof(int));
	
	for(i=0; i < pmu_count ; i++){
		UDP_addresses[i] = (struct sockaddr_in*) malloc(Max_dup_allowed * sizeof(struct sockaddr_in)); 
		UDP_addr_lengths[i] = (int *) malloc(Max_dup_allowed * sizeof(int));
		candidates_count[i] = 0;
	}	
	
	
	for(i=0; i < pmu_count ; i++){
		char path[20];
		strcpy(path,pre);
	    
	    	char name[5];
	    	sprintf(name,"iPMU%d",pmuids[i]);
		
		strcat(path,name);
		char * suf = ".csv";
		strcat(path,suf);

		/*printf("Path = %s \n",path);*/

		fd_csv[i] = fopen(path,"r");

		if(NULL == fd_csv[i])
		{
			printf("\n fopen() Error!!!\n");
			return 1;
		}

		csv_cfg_create(pmuids[i],fd_csv[i],i); // Two rows automatically eliminate
		server(pmuids[i],4000+pmuids[i],6000+pmuids[i],i);
		
		char *l1;
		size_t l2 = 0;
		getdelim (&l1, &l2, ('\n'), fd_csv[i]); // Third row eliminated
    }
        
    sa.sa_handler = sigchld_handler; // reap all dead processes
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {

		perror("sigaction");
		exit(1);
	}

	div_count = pmu_count/50;
/*	div_count = 2;*/
	struct sender_data ** sd = (struct sender_data**)malloc(div_count*sizeof(struct sender_data*));
	int k = 0;
	for(i=0; i < div_count ; i++){
		sd[i] = (struct sender_data *) malloc(sizeof(struct sender_data));
		sd[i]->index = i;
		sd[i]->start = k;
		
		if(i==div_count-1){
			sd[i]->end = pmu_count;		
/*			sd[i]->end = 100;*/
		}else{
			sd[i]->end = k+50;		
			k += 50;
		}
		printf("end = %d\n",sd[i]->end);
	}
	
	sending = (pthread_cond_t*) malloc(div_count*sizeof(pthread_cond_t));
	mutex_sending = (pthread_mutex_t*) malloc(div_count*sizeof(pthread_mutex_t));
	
	int err;
	pthread_attr_t attr;
	pthread_attr_init(&attr);

	if((err = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED))) {

		perror(strerror(err));
		exit(1);
	}																																										     	  
	if((err = pthread_attr_setschedpolicy(&attr,SCHED_FIFO))) {

		perror(strerror(err));		     
		exit(1);
	}
	
	for(i=0; i < div_count ; i++){
		pthread_t t;
		if((err = pthread_create(&t,&attr,sender,(void*)sd[i]))){
			perror(strerror(err));
			exit(1);	
		}
	}

/*generate_data_frame(0);*/

	pthread_cond_wait(&cond,&mutex_data);

	int listenfd = 0,connfd = 0;
	struct sockaddr_in serv_addr;
	char sendBuff[10];  
	 
	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	memset(&serv_addr, '0', sizeof(serv_addr));
		  
	serv_addr.sin_family = AF_INET;    
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); 
	serv_addr.sin_port = htons(syncPort);    
	 
	bind(listenfd, (struct sockaddr*)&serv_addr,sizeof(serv_addr));
	if(listen(listenfd, 1) == -1){
		printf("Failed to listen\n");
		return -1;
	}
	printf("Prepared at ..%d\n",syncPort);
	struct timeval tm;  
	connfd = accept(listenfd, (struct sockaddr*)NULL ,NULL); // accept awaiting request
	
	read(connfd, sendBuff, strlen(sendBuff));
	gettimeofday(&tm, NULL);
	printf("%ld,%ld\n",tm.tv_sec,tm.tv_usec);
	
		
/*	usleep(5000*1000); // To avoid config not found..*/
	SEND_DATA();
/*	close(connfd);    */
	usleep(5000*1000);
	free(pre);
	return 0;
	
} /* end of main */

/*************************************** End of Program ***********************************************/ 
