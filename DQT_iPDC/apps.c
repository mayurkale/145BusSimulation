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
#include <search.h> // hcreate_r() hdestroy_r() struct hsearch_data
#include "apps.h"
#include "global.h"
#include "parser.h"
#include "align_sort.h" 
#include "connections.h"
#include "applications.h"

void createSparse(struct sparseMatrix **sparsehead, struct sparseMatrix **sparsetail, gsl_matrix_complex * Mat){
	*sparsehead = NULL,*sparsetail = NULL;
	int p,q;
	for(p=0; p<Mat->size1; p++){
		for(q =0; q<Mat->size2; q++){
			gsl_complex a = gsl_matrix_complex_get (Mat, p, q);
			/*			if(GSL_REAL(a) == 0 && GSL_IMAG(a) == 0){*/
			if(fabs(GSL_REAL(a)) < 0.000000001 && fabs(GSL_IMAG(a)) < 0.000000001){
				continue;
			}
			else{
				if(*sparsehead == NULL){
					*sparsehead = (struct sparseMatrix*)malloc(sizeof(struct sparseMatrix));
					(*sparsehead)->x = p;
					(*sparsehead)->y = q;
					GSL_SET_COMPLEX (&((*sparsehead)->value), GSL_REAL(a), GSL_IMAG(a));
					(*sparsehead)->next = NULL;
					*sparsetail = *sparsehead;
				}
				else{
					(*sparsetail)->next = (struct sparseMatrix*)malloc(sizeof(struct sparseMatrix));
					*sparsetail = (*sparsetail)->next;
					(*sparsetail)->x = p;
					(*sparsetail)->y = q;
					GSL_SET_COMPLEX (&((*sparsetail)->value), GSL_REAL(a), GSL_IMAG(a));
					(*sparsetail)->next = NULL;
				}
			}
		}
	}
}

void multsparse(struct sparseMatrix* sparsehead, gsl_matrix_complex * onedMat, gsl_matrix_complex * outMat){
	struct sparseMatrix* previous = sparsehead, *current = sparsehead->next;
	gsl_complex cursum = gsl_complex_mul(previous->value,gsl_matrix_complex_get (onedMat, previous->y, 0));
	while(current != NULL){
		if(previous->x == current->x){
			gsl_complex tmp = gsl_complex_mul(current->value,gsl_matrix_complex_get (onedMat, current->y, 0));
			cursum = gsl_complex_add(cursum,tmp);
		}else{
			gsl_matrix_complex_set(outMat,previous->x,0, cursum);
			cursum = gsl_complex_mul(current->value,gsl_matrix_complex_get (onedMat, current->y, 0));
		}
		previous = current;
		current = current->next;
	}
	gsl_matrix_complex_set(outMat,previous->x,0, cursum);
}



void* LPDC_APIs(){
	int i;
	char *line;
	char *d1, *svptr;
	size_t len = 0;
	/*	ssize_t result;*/

	FILE *fp_csv;
	char *filePath = "lpdc_apps.csv";
	fp_csv = fopen (filePath,"r");

	getdelim (&line, &len, ('\n'), fp_csv); // Remove header
	getdelim (&line, &len, ('\n'), fp_csv);
	d1 = strtok_r(line,",",&svptr);
	total_buses = atoi(d1);
	buses_power = (float*)malloc(total_buses * sizeof(float));

	d1 = strtok_r(NULL,",",&svptr);
	listed_buses = atoi(d1);

	char *line2;
	char *d2, *svptr2;
	size_t len2 = 0;

	getdelim (&line, &len, ('\n'), fp_csv);
	d1 = strtok_r(line,",",&svptr);

	getdelim (&line2, &len2, ('\n'), fp_csv);
	d2 = strtok_r(line2,",",&svptr2);

	for(i=0; i<listed_buses; i++){
		d1 = strtok_r(NULL,",",&svptr);
		d2 = strtok_r(NULL,",",&svptr2);

		buses_power[atoi(d1)-1] = atof(d2);
		/*		printf("%f %s\n",buses_power[atoi(d1)-1],d2);*/
	}

	getdelim (&line, &len, ('\n'), fp_csv);
	d1 = strtok_r(line,",",&svptr);
	d1 = strtok_r(NULL,",",&svptr);
	num_api = atoi(d1);

	active_api_ids = (int*)malloc(num_api * sizeof(int));

	getdelim (&line, &len, ('\n'), fp_csv);
	d1 = strtok_r(line,",",&svptr);

	for(i=0; i<num_api; i++){
		d1 = strtok_r(NULL,",",&svptr);
		int api_id = atoi(d1);
		active_api_ids[i] = api_id;

		switch(api_id)
		{
		case 1:
			COH_GEN=true;
			break;
		case 2:
			STATE_EST=true;
			break;
		case 6:
			ANG_STAB=true;
			break;
		case 7:
			ALL_DATA=true;
			break;
		}
	}

	getdelim (&line, &len, ('\n'), fp_csv); // Remove header
	getdelim (&line, &len, ('\n'), fp_csv);

	d1 = strtok_r(line,",",&svptr);
	int api_info_count = atoi(d1);
	//printf("api_info_count = %d\n",api_info_count);
	//printf("num_api = %d\n",num_api);

	for(i=0; i<api_info_count; i++){
		getdelim (&line, &len, ('\n'), fp_csv); // Remove header
		getdelim (&line, &len, ('\n'), fp_csv);

		d1 = strtok_r(line,",",&svptr);
		int api_id = atoi(d1);
		//printf("%d\n",api_id);

		d1 = strtok_r(NULL,",",&svptr);
		int priority = atoi(d1);
		/*		printf("%d\n",priority);*/

		switch(api_id){
		case 1:	
			printf("API 1 : CM\n");
			pthread_mutex_init(&mutex_App1, NULL);

			app1 = (struct App1_Info*)malloc(sizeof(struct App1_Info));
			app1->app_id = api_id;
			app1->data_priority = priority;
			d1 = strtok_r(NULL,",",&svptr);
			app1->num_modes = atoi(d1);
			app1->current_modes = 0;

			memset(&(app1->hashForApp1Pmus), 0, sizeof(app1->hashForApp1Pmus));
			if( hcreate_r(total_buses, &(app1->hashForApp1Pmus)) == 0 ) {
				perror("hcreate_r");
			}

			app1->modes = (struct mode_info **)malloc(app1->num_modes * sizeof(struct mode_info *));

			int j;
			for(j=0; j<app1->num_modes; j++){
				app1->modes[j] = (struct mode_info *)malloc(sizeof(struct mode_info));

				getdelim (&line, &len, ('\n'), fp_csv); // Remove header
				getdelim (&line, &len, ('\n'), fp_csv);
				d1 = strtok_r(line,",",&svptr);

				app1->modes[j]->mode_freq = atof(d1);
				d1 = strtok_r(NULL,",",&svptr);
				int g1_count = atoi(d1);
				d1 = strtok_r(NULL,",",&svptr);
				int g2_count = atoi(d1);
				d1 = strtok_r(NULL,",",&svptr);
				int pdcid1 = atoi(d1);
				d1 = strtok_r(NULL,",",&svptr);
				int pdcid2 = atoi(d1);
				d1 = strtok_r(NULL,",",&svptr);
				app1->modes[j]->phasor_index = atoi(d1);

				if(pdcid1 == PDC_IDCODE){
					app1->modes[j]->monitor_groupno = 1;
					app1->modes[j]->num_pmus = g1_count;
					getdelim (&line, &len, ('\n'), fp_csv);
					char *tmp;
					size_t len1 = 0;
					getdelim (&tmp, &len1, ('\n'), fp_csv);

				}else if(pdcid2 == PDC_IDCODE){
					app1->modes[j]->monitor_groupno = 2;
					app1->modes[j]->num_pmus = g2_count;
					getdelim (&line, &len, ('\n'), fp_csv);
					getdelim (&line, &len, ('\n'), fp_csv);
				}else{
					printf("CSV Corrupted !!\n");		
				}
				int k;
				d1 = strtok_r(line,",",&svptr);

				memset(&(app1->modes[j]->pmuids), 0, sizeof(app1->modes[j]->pmuids));
				if( hcreate_r(app1->modes[j]->num_pmus, &(app1->modes[j]->pmuids)) == 0 ) {
					perror("hcreate_r");
				}

				app1->modes[j]->power_sum = 0.0;

				/*				printf("Mode = %d\n",j);*/
				for(k=0; k<app1->modes[j]->num_pmus; k++){
					d1 = strtok_r(NULL,",",&svptr);
					/*					printf("%s\n",d1);	*/

					app1->modes[j]->power_sum += buses_power[atoi(d1)-1];

					ENTRY item;
					ENTRY * ret;

					item.key = strdup(d1);
					item.data = NULL;

					if( hsearch_r(item, ENTER, &ret, &(app1->modes[j]->pmuids)) == 0 ) {
						perror("hsearch_r");
						exit(1);
					}

					if( hsearch_r(item, ENTER, &ret, &(app1->hashForApp1Pmus)) == 0 ) {
						perror("hsearch_r");
						exit(1);
					}
				}
				/*				printf("\n");*/
				/*				printf("Power sum : Mode [%d] Sum [%f]\n",j,app1->modes[j]->power_sum);*/

				app1->modes[j]->theta_c_i = 0.0;
				app1->modes[j]->current_pmus = 0;
			}

			break;
		case 2:	printf("API 2 : DSE\n");

		pthread_mutex_init(&mutex_App2, NULL);

		app2 = (struct App2_Info*)malloc(sizeof(struct App2_Info));
		app2->app_id = api_id;
		app2->data_priority = priority;

		d1 = strtok_r(NULL,",",&svptr); // PDC1ID
		int pdc1id = atoi(d1);

		d1 = strtok_r(NULL,",",&svptr); // PDC2ID
		int pdc2id = atoi(d1);

		char *li1,*li2,*li3,*li4;
		char *dil1, *ptr1, *dil2, *ptr2, *dil3, *ptr3, *dil4, *ptr4;
		size_t lth1 = 0, lth2 = 0, lth3 = 0, lth4 = 0;

		if(pdc1id == PDC_IDCODE){
			getdelim (&line, &len, ('\n'), fp_csv); // Eliminate Header

			getdelim (&li1, &lth1, ('\n'), fp_csv);
			getdelim (&li2, &lth2, ('\n'), fp_csv);
			getdelim (&li3, &lth3, ('\n'), fp_csv);
			getdelim (&li4, &lth4, ('\n'), fp_csv);

			getdelim (&line, &len, ('\n'), fp_csv);
			getdelim (&line, &len, ('\n'), fp_csv);
			getdelim (&line, &len, ('\n'), fp_csv);
			getdelim (&line, &len, ('\n'), fp_csv);
			getdelim (&line, &len, ('\n'), fp_csv);

		}else if(pdc2id == PDC_IDCODE){

			getdelim (&line, &len, ('\n'), fp_csv);
			getdelim (&line, &len, ('\n'), fp_csv);
			getdelim (&line, &len, ('\n'), fp_csv);
			getdelim (&line, &len, ('\n'), fp_csv);
			getdelim (&line, &len, ('\n'), fp_csv);

			getdelim (&line, &len, ('\n'), fp_csv); // Eliminate Header
			getdelim (&li1, &lth1, ('\n'), fp_csv);
			getdelim (&li2, &lth2, ('\n'), fp_csv);
			getdelim (&li3, &lth3, ('\n'), fp_csv);
			getdelim (&li4, &lth4, ('\n'), fp_csv);

		}else{
			printf("CSV Corrupted !!\n");
		}

		dil1 = strtok_r(li1,",",&ptr1); // PDC1id
		dil1 = strtok_r(NULL,",",&ptr1); // Num_PMUs
		int pdc_numpmu = atoi(dil1);

		app2->num_pmus = pdc_numpmu;
		app2->current_pmus = 0;

		memset(&(app2->hashForApp2Pmus), 0, sizeof(app2->hashForApp2Pmus));
		if( hcreate_r(app2->num_pmus , &(app2->hashForApp2Pmus)) == 0 ) {
			perror("hcreate_r");
		}

		dil1 = strtok_r(NULL,",",&ptr1); // m (No of rows in M)
		int row = atoi(dil1);
		dil1 = strtok_r(NULL,",",&ptr1); // n (No of columns in M)
		int column = atoi(dil1);

		FILE* fm;

		app2->M = gsl_matrix_complex_alloc (row,column);     // M
		dil1 = strtok_r(NULL,",",&ptr1);
		fm = fopen(dil1,"r");
		gsl_matrix_complex_fscanf(fm, app2->M);
		fclose(fm);

		createSparse(&app2->MsparseHead,&app2->MsparseTail,app2->M);

		app2->Mpinv = gsl_matrix_complex_alloc (column,row); // Mpinv
		dil1 = strtok_r(NULL,",",&ptr1);
		fm = fopen(dil1,"r");
		gsl_matrix_complex_fscanf(fm, app2->Mpinv);
		fclose(fm);

		createSparse(&app2->MpinvsparseHead,&app2->MpinvsparseTail,app2->Mpinv);

		/*				struct sparseMatrix* siter = app2->MpinvsparseHead;*/
		/*				int countersparse =0;*/
		/*				while(siter != NULL){*/
		/*					printf("%d %d (%f %f)\n",siter->x,siter->y,GSL_REAL(siter->value),GSL_IMAG(siter->value));*/
		/*					*/
		/*					siter = siter->next;*/
		/*					countersparse++;*/
		/*				}*/

		/*				printf("In swith : %d\n",countersparse);*/

		app2->Pdiag = gsl_matrix_complex_alloc (row,1);     // Pdiag
		dil1 = strtok_r(NULL,",",&ptr1);
		fm = fopen(dil1,"r");
		gsl_matrix_complex_fscanf(fm, app2->Pdiag);
		fclose(fm);

		app2->MHMinvdiag = gsl_matrix_complex_alloc (column,1);     // MHMinvdiag
		dil1 = strtok_r(NULL,",",&ptr1);
		fm = fopen(dil1,"r");
		gsl_matrix_complex_fscanf(fm, app2->MHMinvdiag);
		fclose(fm);

		dil1 = strtok_r(NULL,",",&ptr1); // Z index Mapping for input and output
		fm = fopen(dil1,"r");

		char *infokey, *infoptr;
		size_t lthinfo = 0;

		int m;
		getdelim (&line, &lthinfo, ('\n'), fm);
		strtok_r(line,",",&infoptr);
		infokey = strtok_r(NULL,",",&infoptr);

		int mapcount = atoi(infokey);

		memset(&(app2->hashForZmatrixIndex), 0, sizeof(app2->hashForZmatrixIndex));
		if( hcreate_r(mapcount , &(app2->hashForZmatrixIndex)) == 0 ) {
			perror("hcreate_r");
		}

		for(m=0; m<mapcount; m++){
			getdelim(&infokey, &lthinfo, ('\n'), fm);
			int size = strlen(infokey);
			infokey[size-1] = '\0'; // Remove \n
			int * index = (int*)malloc(sizeof(int));
			*index = m;
			ENTRY item;
			ENTRY * ret;

			item.key = strdup(infokey);
			item.data = index;
			/*printf("%s\n",infokey);*/
			if( hsearch_r(item, ENTER, &ret, &(app2->hashForZmatrixIndex)) == 0 ) {
				perror("hsearch_r");
				exit(1);
			}
		}

		getdelim (&line, &lthinfo, ('\n'), fm);
		strtok_r(line,",",&infoptr);
		infokey = strtok_r(NULL,",",&infoptr);
		/*printf("here1 %s\n",infokey);	   				*/

		app2->numOutputKeys = atoi(infokey);
		/*   		printf("%d\n",atoi(infokey));*/

		app2->OutputKeys = (char**) malloc(app2->numOutputKeys * sizeof(char*));
		for(m=0; m<app2->numOutputKeys; m++){
			getdelim(&infokey, &lthinfo, ('\n'), fm);
			int size = strlen(infokey);
			infokey[size-1] = '\0';
			app2->OutputKeys[m] = strdup(infokey);
		}


		getdelim (&line, &lthinfo, ('\n'), fm);
		strtok_r(line,",",&infoptr);
		infokey = strtok_r(NULL,",",&infoptr);


		app2->numOutputVhatIndex = atoi(infokey);
		/*printf("here1 %s\n",infokey);	   				*/
		app2->OutputVhatIndex = (int*) malloc(app2->numOutputVhatIndex * sizeof(int));
		for(m=0; m<app2->numOutputVhatIndex; m++){
			getdelim(&infokey, &lthinfo, ('\n'), fm);
			char *ind = strtok_r(infokey,",",&infoptr);
			app2->OutputVhatIndex[m] = atoi(ind);
		}

		fclose(fm);

		app2->Z = gsl_matrix_complex_alloc (row,1);          // Z
		app2->Vhat = gsl_matrix_complex_alloc (column,1);    // Vhat
		app2->Zhat = gsl_matrix_complex_alloc (row,1);       // Zhat
		app2->r = gsl_matrix_complex_alloc (row,1);          // r

		dil2 = strtok_r(li2,",",&ptr2);
		dil3 = strtok_r(li3,",",&ptr3);
		dil4 = strtok_r(li4,",",&ptr4);
		/*printf("here1 %d\n",app2->num_pmus);												*/
		for(m=0; m<app2->num_pmus; m++){
			struct App2_phasor_map* map = (struct App2_phasor_map*) malloc(sizeof(struct App2_phasor_map));

			dil2 = strtok_r(NULL,",",&ptr2);
			dil3 = strtok_r(NULL,",",&ptr3);
			/*printf("here1 %s\n",dil3);												*/
			map->num_phasor = atoi(dil3);
			map->phasor_ids = (int *)malloc(map->num_phasor * sizeof(int));

			dil4 = strtok_r(NULL,",",&ptr4);
			char *phs, *phsptr;
			int n;

			for(n=0; n<map->num_phasor; n++){
				if(n==0){
					phs = strtok_r(dil4,":",&phsptr);
				}
				else{
					phs = strtok_r(NULL,":",&phsptr);
				}
				map->phasor_ids[n] = atoi(phs);
			}
			/*		printf("here\n");								*/

			ENTRY item;
			ENTRY * ret;

			item.key = strdup(dil2);
			item.data = map;

			if( hsearch_r(item, ENTER, &ret, &(app2->hashForApp2Pmus)) == 0 ) {
				perror("hsearch_r");
				exit(1);
			}
		}
		break;
		case 6:	printf("API 6 : ASM\n");
		buildQueryListHashTables();
		break;
		case 7:	printf("API 7 : ALL_DATA\n");
		pthread_mutex_init(&mutex_App7, NULL);
		app7 = (struct App7_Info*)malloc(sizeof(struct App7_Info));
		app7->app_id = api_id;
		app7->data_priority = priority;
		break;


		default :
			printf("Not a Valid APIID !!\n");
		}
	}

	fclose(fp_csv);
	printf("Done\n");
	pthread_exit(NULL);
}

void check(void *datafrm){
	int i;
	for(i=0; i<num_api; i++){
		switch(active_api_ids[i]){
		case 1: 
			application1(datafrm);
			break;
		case 2: 
			application2(datafrm);
			break;
		case 6: 
			application6(datafrm);
			break;
		case 7:
			application7(datafrm);
			break;
		}
	}
}




void application2(void *datafrm){
	struct data_frame *df = (struct data_frame*)datafrm;
	char key[16];
	sprintf(key,"%d",to_intconvertor((unsigned char*)df->idcode));
	ENTRY entry;
	ENTRY *found;
	entry.key = key;
	hsearch_r( entry, FIND, &found, &(app2->hashForApp2Pmus));

	if(found!=NULL){
		//printf("%s\n",key);
		struct App2_phasor_map* map = (struct App2_phasor_map*)found->data;
		int i;

		float fp_r,fp_i,fp_real=0,fp_imaginary=0;
		unsigned char fp_left[4],fp_right[4];
		unsigned char *index;

		char Zmapkey[100];
		ENTRY e;
		ENTRY *f;

		for(i=0; i<map->num_phasor; i++){
			if(df->dpmu[0]->fmt->phasor == '1') { // Float
				index = df->dpmu[0]->phasors[map->phasor_ids[i]];
				memset(fp_left,'\0',5);
				memset(fp_right,'\0',5);
				copy_cbyc (fp_left,index,4);
				fp_left[4] = '\0';
				index += 4;

				copy_cbyc(fp_right,index,4);
				fp_right[4] = '\0';   

				fp_r = decode_ieee_single(fp_left);
				fp_i = decode_ieee_single(fp_right);

				if(df->dpmu[0]->fmt->polar == '1'){
					fp_real = fp_r;
					fp_imaginary = fp_i;
				}else{
					printf("Polar required.. Not applicable on Reactangular\n");
					break;
				}
			} 

			// Insertion in Z matrix
			sprintf(Zmapkey,"%s,%d",key,map->phasor_ids[i]);

			e.key = Zmapkey;
			hsearch_r( e, FIND, &f, &(app2->hashForZmatrixIndex));
			if(f!=NULL){
				int* index = (int*)f->data;
				gsl_complex value = gsl_complex_polar(fp_real,fp_imaginary);
				gsl_matrix_complex_set (app2->Z, *index, 0, value );
				gsl_matrix_complex_set (app2->r, *index, 0, value );
			}else
				printf("Problem in Z matrix Mapping\n");

		}

		pthread_mutex_lock(&mutex_App2);
		app2->current_pmus++;

		if(app2->current_pmus == app2->num_pmus){
			pthread_attr_t attr;
			pthread_attr_init(&attr);
			int err;
			if((err = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED))) {
				perror(strerror(err));
				exit(1);
			}
			if((err = pthread_attr_setschedpolicy(&attr,SCHED_FIFO))) {
				perror(strerror(err));		     
				exit(1);
			}  
			pthread_t t;
			if((err = pthread_create(&t,&attr,DSE_dispatcher,datafrm))) {
				perror(strerror(err));		     
				exit(1);
			}
			app2->current_pmus = 0;
		}
		pthread_mutex_unlock(&mutex_App2);
	}
}

void * DSE_dispatcher(void *datafrm){
	/*	printf("In DSE\n");*/
	struct data_frame *df = (struct data_frame*)datafrm;
	uint16_t chk;				
	unsigned char temp[3],temp5[5];
	unsigned int total_frame_size;

	unsigned char dataframe[5000];

	int z = 0;
	byte_by_byte_copy(dataframe,DATASYNC,z,2); // SYNC Word for data frame
	z += 2;
	z += 2;

	memset(temp,'\0',3);
	int_to_ascii_convertor(PDC_IDCODE,temp);
	byte_by_byte_copy(dataframe,temp,z,2); // PDC ID
	z += 2;

	byte_by_byte_copy(dataframe,(unsigned char *)df->soc,z,4); //SOC
	z += 4;
	byte_by_byte_copy(dataframe,(unsigned char *)df->fracsec,z,4); //FRACSEC 
	z += 4;

	unsigned char stat[2]; 
	stat[0] = 0x09;   // Stat word for Distributed State estimation partally computed data
	stat[1] = 0x00;

	byte_by_byte_copy(dataframe,stat,z,2); // stat
	z += 2;

	memset(temp,'\0',3);
	int_to_ascii_convertor(app2->app_id,temp);
	byte_by_byte_copy(dataframe,temp,z,2); // API ID
	z += 2;

	struct timeval tm;
	//	gettimeofday(&tm, NULL);
	//    long int s = tm.tv_sec, f = tm.tv_usec;

	// Sparse Matrix Multiplication
	multsparse(app2->MpinvsparseHead,app2->Z,app2->Vhat);
	multsparse(app2->MsparseHead,app2->Vhat,app2->Zhat);

	// Dence Matrix Multiplication
	// gsl_blas_zgemm(CblasNoTrans,CblasNoTrans,GSL_COMPLEX_ONE, app2->Mpinv,app2->Z,GSL_COMPLEX_ZERO,app2->Vhat);
	// gsl_blas_zgemm(CblasNoTrans, CblasNoTrans,GSL_COMPLEX_ONE, app2->M, app2->Vhat,GSL_COMPLEX_ZERO, app2->Zhat);

	gsl_matrix_complex_sub(app2->r,app2->Zhat);
	gsl_vector_complex_view rv = gsl_matrix_complex_column(app2->r,0);
	gsl_complex Sigmahat2;
	gsl_blas_zdotc(&rv.vector,&rv.vector,&Sigmahat2);
	Sigmahat2 = gsl_complex_div_real(Sigmahat2, app2->M->size1 - app2->M->size2);

	int resultsize = 0;
	memset(temp5,'\0',5);
	f2c(GSL_REAL(Sigmahat2),temp5);
	byte_by_byte_copy(dataframe,temp5,z,4); 
	z += 4;
	resultsize += 4;

	memset(temp5,'\0',5);
	f2c(GSL_IMAG(Sigmahat2),temp5);
	byte_by_byte_copy(dataframe,temp5,z,4); 
	z += 4;
	resultsize += 4;

	/*	printf("%f %f\n",GSL_REAL(Sigmahat2),GSL_IMAG(Sigmahat2));*/

	int i;
	ENTRY entry;
	ENTRY *found;
	gsl_complex a,b,c;	

	/*for(i =0; i<app2->numOutputKeys; i++)*/
	/*		printf("%s ",app2->OutputKeys[i]);*/
	/*printf("\n");*/

	for(i =0; i<app2->numOutputKeys; i++){
		entry.key = app2->OutputKeys[i];
		/*		printf("%s\n",entry.key);*/

		hsearch_r( entry, FIND, &found, &(app2->hashForZmatrixIndex));
		int *index = (int *)found->data;
		a = gsl_matrix_complex_get (app2->Z, *index, 0);
		b = gsl_matrix_complex_get (app2->Zhat, *index, 0);
		c = gsl_complex_mul(Sigmahat2,gsl_matrix_complex_get (app2->Pdiag, *index, 0));

		memset(temp5,'\0',5);
		f2c(GSL_REAL(a),temp5);
		byte_by_byte_copy(dataframe,temp5,z,4); 
		z += 4;
		resultsize += 4;

		memset(temp5,'\0',5);
		f2c(GSL_IMAG(a),temp5);
		byte_by_byte_copy(dataframe,temp5,z,4); 
		z += 4;
		resultsize += 4;

		memset(temp5,'\0',5);
		f2c(GSL_REAL(b),temp5);
		byte_by_byte_copy(dataframe,temp5,z,4); 
		z += 4;
		resultsize += 4;

		memset(temp5,'\0',5);
		f2c(GSL_IMAG(b),temp5);
		byte_by_byte_copy(dataframe,temp5,z,4); 
		z += 4;
		resultsize += 4;

		memset(temp5,'\0',5);
		f2c(GSL_REAL(c),temp5);
		byte_by_byte_copy(dataframe,temp5,z,4); 
		z += 4;
		resultsize += 4;

		memset(temp5,'\0',5);
		f2c(GSL_IMAG(c),temp5);
		byte_by_byte_copy(dataframe,temp5,z,4); 
		z += 4;
		resultsize += 4;

		//printf("%s , %d , %f , %f, %f , %f, %f , %f\n",app2->OutputKeys[i], *index, GSL_REAL(a), GSL_IMAG(a), GSL_REAL(b), GSL_IMAG(b), GSL_REAL(c), GSL_IMAG(c));
	}
	/*	printf("here\n");*/
	for(i=0; i<app2->numOutputVhatIndex; i++){
		b = gsl_matrix_complex_get (app2->Vhat, app2->OutputVhatIndex[i], 0);
		c = gsl_complex_mul(Sigmahat2,gsl_matrix_complex_get (app2->MHMinvdiag, app2->OutputVhatIndex[i], 0));
		/*printf("here1\n");*/
		memset(temp5,'\0',5);
		f2c(GSL_REAL(b),temp5);
		byte_by_byte_copy(dataframe,temp5,z,4); 
		z += 4;
		resultsize += 4;

		memset(temp5,'\0',5);
		f2c(GSL_IMAG(b),temp5);
		byte_by_byte_copy(dataframe,temp5,z,4); 
		z += 4;
		resultsize += 4;

		memset(temp5,'\0',5);
		f2c(GSL_REAL(c),temp5);
		byte_by_byte_copy(dataframe,temp5,z,4); 
		z += 4;
		resultsize += 4;

		memset(temp5,'\0',5);
		f2c(GSL_IMAG(c),temp5);
		byte_by_byte_copy(dataframe,temp5,z,4); 
		z += 4;
		resultsize += 4;

		/*		printf("%d , %f , %f, %f , %f\n",app2->OutputVhatIndex[i], GSL_REAL(b), GSL_IMAG(b), GSL_REAL(c), GSL_IMAG(c));*/
	}

	for(i=0; i<app2->Vhat->size1; i++){
		b = gsl_matrix_complex_get (app2->Vhat, i, 0);

		memset(temp5,'\0',5);
		f2c(GSL_REAL(b),temp5);
		byte_by_byte_copy(dataframe,temp5,z,4); 
		z += 4;
		resultsize += 4;

		memset(temp5,'\0',5);
		f2c(GSL_IMAG(b),temp5);
		byte_by_byte_copy(dataframe,temp5,z,4); 
		z += 4;
		resultsize += 4;
	}

	/*	gettimeofday(&tm, NULL);*/
	/*	printf("sparse time diff = %ld,%ld\n",tm.tv_sec-s,tm.tv_usec-f);		*/

	total_frame_size = 20 + resultsize;

	/*	printf("%d\n",total_frame_size);*/

	memset(temp,'\0',3);
	int_to_ascii_convertor(total_frame_size,temp);
	byte_by_byte_copy(dataframe,temp,2,2); // FRAME SIZE

	chk = compute_CRC((unsigned char *)dataframe,z);
	dataframe[z] = (chk >> 8) & ~(~0<<8);  	// CHKSUM high byte; 
	dataframe[z+1] = (chk ) & ~(~0<<8);     // CHKSUM low byte;  
	z += 2;

	struct Upper_Layer_Details *spdc = ULfirst;

	unsigned long esoc,efsec;
	int indx;
	unsigned char temp4[4];

	gettimeofday(&tm, NULL);
	esoc = tm.tv_sec;
	efsec = tm.tv_usec;

	indx = z;

	long_int_to_ascii_convertor(esoc,temp4);
	byte_by_byte_copy(dataframe,temp4,indx,4);
	indx = indx + 4;

	long_int_to_ascii_convertor(efsec,temp4);
	byte_by_byte_copy(dataframe,temp4,indx,4);

	pthread_mutex_lock(&mutex_Upper_Layer_Details);

	while(spdc != NULL ) {

		if(spdc->UL_upper_pdc_cfgsent == 1){
			if(spdc->port == UDPPORT) {
				if (sendto(spdc->sockfd,dataframe,z+8,0,(struct sockaddr *)&spdc->pdc_addr,sizeof(spdc->pdc_addr)) == -1)
					perror("sendto");					

			} else if((spdc->port == TCPPORT) && (spdc->tcpup == 1)) {

				if(send(spdc->sockfd,dataframe,z+8, 0)== -1) {
					perror("send");	
					spdc->tcpup = 0;
					pthread_cancel(spdc->thread_id);		
				}				
			}
		}
		spdc = spdc->next;			
	}
	pthread_mutex_unlock(&mutex_Upper_Layer_Details);
	/*	Analysing(21,0,esoc,efsec); */
	app2_Analysing(21,0,0,to_long_int_convertor((unsigned char *)df->fracsec),esoc,efsec); 

	pthread_exit(NULL);
}



void application1(void *datafrm){
	struct data_frame *df = (struct data_frame*)datafrm;
	char key[16];
	sprintf(key,"%d",to_intconvertor((unsigned char*)df->idcode));
	ENTRY entry;
	ENTRY *found;
	entry.key = key;
	hsearch_r( entry, FIND, &found, &(app1->hashForApp1Pmus));

	if(found!=NULL){
		int i;
		pthread_mutex_lock(&mutex_App1);
		/*	pthread_mutex_unlock(&mutex_App1);*/

		for(i=0; i<app1->num_modes; i++){
			/*	printf("mode no. = %d , num_pmus= %d,  current_pmus = %d \n",i,app1->modes[i]->num_pmus, app1->modes[i]->current_pmus);*/
			ENTRY entry;
			ENTRY *found;
			char key[16];
			sprintf(key,"%d",to_intconvertor((unsigned char*)df->idcode));

			entry.key = key;
			hsearch_r( entry, FIND, &found, &(app1->modes[i]->pmuids));

			if(found != NULL) {
				/*		printf("Found key = %s\n",key);*/
				float fp_i,fp_imaginary;
				unsigned char fp_left[4],fp_right[4];

				unsigned char *index;

				if(df->dpmu[0]->fmt->phasor == '1') { // Float
					index = df->dpmu[0]->phasors[app1->modes[i]->phasor_index];
					memset(fp_left,'\0',5);
					memset(fp_right,'\0',5);
					copy_cbyc (fp_left,index,4);
					fp_left[4] = '\0';
					index += 4;

					copy_cbyc(fp_right,index,4);
					fp_right[4] = '\0';   

					/*					fp_r = decode_ieee_single(fp_left);*/
					fp_i = decode_ieee_single(fp_right);

					if(df->dpmu[0]->fmt->polar == '1'){
						/*						fp_real = fp_r;*/
						fp_imaginary = fp_i;
					}else{
						printf("Polar required.. Not applicable on Reactangular\n");
						break;
					}

				} 
				/*				else { // Fixed Point*/
				/*					index = df->dpmu[0]->phasors[app1->modes[i]->phasor_index];*/
				/*					memset(fp_left,'\0',3);*/
				/*					memset(fp_right,'\0',3);*/
				/*					copy_cbyc(fp_left,index,2);*/
				/*					fp_left[2] = '\0';*/
				/*					index += 2;*/

				/*					copy_cbyc(fp_right,index,2);*/
				/*					fp_right[2] = '\0';   */

				/*					fp_r = to_intconvertor(fp_left);*/
				/*					fp_i = to_intconvertor(fp_right);*/
				/*					if(df->dpmu[0]->fmt->polar == '1'){*/
				/*		       				fp_imaginary = fp_i*1e-4; // Angle is in 10^4 radians*/
				/*					}else{*/
				/*						printf("Polar required.. Not applicable on Reactangular\n");*/
				/*						break;*/
				/*					}*/
				/*				}*/

				/*			printf("Mode = %d, PMU = %s, buses_power= %f, fp_imaginary %f\n",i,key,buses_power[atoi(key)-1],fp_imaginary);*/
				//fp_imaginary = fp_imaginary*180/3.14; // Convert to degree
				//printf("Degree fp_imaginary %f\n",fp_imaginary);

				/*			fp_imaginary = fp_imaginary<0 ? fp_imaginary+6.28 : fp_imaginary;*/

				app1->modes[i]->theta_c_i += buses_power[atoi(key)-1] * fp_imaginary;
				app1->modes[i]->current_pmus ++;

				if(app1->modes[i]->num_pmus == app1->modes[i]->current_pmus){
					app1->current_modes ++;
					/*				printf("current_modes = %d\n",app1->current_modes);*/
					/*				int k;*/
					/*				for(k=0; k<app1->num_modes; k++){*/
					/*					printf("mode no. = %d , num_pmus= %d,  current_pmus = %d \n",k,app1->modes[k]->num_pmus, app1->modes[k]->current_pmus);*/
					/*				}*/
				}

			}
		}

		if(app1->num_modes == app1->current_modes){
			//create and send packet

			uint16_t chk;				
			unsigned char temp[3],temp5[5];
			unsigned int total_frame_size;

			total_frame_size = 20 + 4 * app1->num_modes; 

			unsigned char dataframe[total_frame_size+8];

			int z = 0;
			byte_by_byte_copy(dataframe,DATASYNC,z,2); // SYNC Word for data frame
			z += 2;

			memset(temp,'\0',3);
			int_to_ascii_convertor(total_frame_size,temp);
			byte_by_byte_copy(dataframe,temp,z,2); // FRAME SIZE
			z += 2;

			memset(temp,'\0',3);
			int_to_ascii_convertor(PDC_IDCODE,temp);
			byte_by_byte_copy(dataframe,temp,z,2); // PDC ID
			z += 2;

			byte_by_byte_copy(dataframe,(unsigned char *)df->soc,z,4); //SOC
			z += 4;
			byte_by_byte_copy(dataframe,(unsigned char *)df->fracsec,z,4); //FRACSEC 
			z += 4;

			unsigned char stat[2]; 
			stat[0] = 0x08;   // Stat word for partally computed data
			stat[1] = 0x00;


			byte_by_byte_copy(dataframe,stat,z,2); // stat
			z += 2;

			memset(temp,'\0',3);
			int_to_ascii_convertor(app1->app_id,temp);
			byte_by_byte_copy(dataframe,temp,z,2); // API ID
			z += 2;

			for(i=0; i<app1->num_modes; i++){
				app1->modes[i]->theta_c_i /= app1->modes[i]->power_sum;

				memset(temp5,'\0',5);
				f2c(app1->modes[i]->theta_c_i,temp5);
				byte_by_byte_copy(dataframe,temp5,z,4); // Phasor Angle
				z += 4;

				/*				printf("Mode = %d, theta_c_i = %f, power_sum = %f\n",i,app1->modes[i]->theta_c_i,app1->modes[i]->power_sum);*/

				app1->modes[i]->current_pmus = 0;	
				app1->modes[i]->theta_c_i = 0.0;
			}


			chk = compute_CRC((unsigned char *)dataframe,z);
			dataframe[z] = (chk >> 8) & ~(~0<<8);  	// CHKSUM high byte; 
			dataframe[z+1] = (chk ) & ~(~0<<8);     // CHKSUM low byte;  
			z += 2;

			app1->current_modes = 0;


			struct Upper_Layer_Details *spdc = ULfirst;

			unsigned long esoc,efsec;
			int indx;
			unsigned char temp4[4];
			struct timeval tm;
			gettimeofday(&tm, NULL);
			esoc = tm.tv_sec;
			efsec = tm.tv_usec;


			indx = z;

			long_int_to_ascii_convertor(esoc,temp4);
			byte_by_byte_copy(dataframe,temp4,indx,4);
			indx = indx + 4;

			long_int_to_ascii_convertor(efsec,temp4);
			byte_by_byte_copy(dataframe,temp4,indx,4);

			pthread_mutex_lock(&mutex_Upper_Layer_Details);

			while(spdc != NULL ) {

				if(spdc->UL_upper_pdc_cfgsent == 1){
					if(spdc->port == UDPPORT) {
						if (sendto(spdc->sockfd,dataframe,z+8,0,(struct sockaddr *)&spdc->pdc_addr,sizeof(spdc->pdc_addr)) == -1)
							perror("sendto");					

					} else if((spdc->port == TCPPORT) && (spdc->tcpup == 1)) {

						if(send(spdc->sockfd,dataframe,z+8, 0)== -1) {
							perror("send");	
							spdc->tcpup = 0;
							pthread_cancel(spdc->thread_id);		
						}				
					}
				}
				spdc = spdc->next;			
			}
			pthread_mutex_unlock(&mutex_Upper_Layer_Details);
			Analysing(12,0,esoc,efsec); 
		}
		pthread_mutex_unlock(&mutex_App1);
	}
}


void buildQueryListHashTables() {
	size_t max_PMU = 2000;
	memset(&hashForPMUQuery, 0, sizeof(hashForPMUQuery));
	if( hcreate_r(max_PMU, &hashForPMUQuery) == 0 ) {
		perror("hcreate_r");
	}	
}

void updateQueryList(char * elmt_key,struct PMUQueryInfo *elmt_data, struct hsearch_data * table) {

	ENTRY entry;
	ENTRY *found;
	entry.key = elmt_key;
	hsearch_r( entry, FIND, &found, table);

	/*	pthread_mutex_lock(&mutex_query);*/

	if(found != NULL) {
		struct PMUQueryInfo * nxt = (struct PMUQueryInfo *)found->data;
		while((nxt->api_id != elmt_data->api_id || nxt->query_id != elmt_data->query_id) && nxt->nextQuery != NULL){
			nxt = nxt->nextQuery;
		}

		if(nxt->api_id == elmt_data->api_id && nxt->query_id == elmt_data->query_id){ // Update old entry
			nxt->pmuid = elmt_data->pmuid;
			/*			nxt->api_id = elmt_data->api_id;*/
			/*			nxt->query_id = elmt_data->query_id;*/
			strcpy((char *)nxt->phasorname,(char*)elmt_data->phasorname);
			nxt->bounds_slope = elmt_data->bounds_slope;
			nxt->displace_lb = elmt_data->displace_lb;
			nxt->displace_ub = elmt_data->displace_ub;

			nxt->invoke_soc = elmt_data->invoke_soc;
			nxt->invoke_fracsec = elmt_data->invoke_fracsec;
			nxt->spdc = elmt_data->spdc;
			/*printf("element pmuid = %d, apiid = %d, queryid = %d\n",elmt_data->pmuid,elmt_data->api_id,elmt_data->query_id);*/
			/*printf("nxt pmuid = %d, apiid = %d, queryid = %d\n",nxt->pmuid,nxt->api_id,nxt->query_id);*/
			free(elmt_data);
		}
		else{ //Make new entry
			nxt->nextQuery = elmt_data;
			nxt = nxt->nextQuery;
			nxt->nextQuery = NULL;
		}
	}
	else{
		ENTRY item;
		ENTRY * ret;

		item.key = strdup(elmt_key);
		elmt_data->nextQuery = NULL;
		item.data = elmt_data;  

		if( hsearch_r(item, ENTER, &ret, table) == 0 ) {
			perror("hsearch_r");
			exit(1);
		}

		Analysing(60,atoi(elmt_key),0,0);
	}

	/*	pthread_mutex_unlock(&mutex_query);*/
	return;

}

/**************************************** My Functions *******************************************************/
void create_send(void *info,void *datafrm,int type){
	/*if(type==2)	printf("In create_send\n");*/

	struct data_frame *df = (struct data_frame*)datafrm;
	struct PMUQueryInfo * nxt = (struct PMUQueryInfo *)info;
	uint16_t chk;				
	unsigned char temp[3],temp5[5];
	unsigned int total_frame_size;
	//unsigned char* dataframe;

	switch(nxt->api_id){
	case 6: //printf("Sending....\n");
		total_frame_size = 32;
		unsigned char dataframe[total_frame_size];
		// Start the data frame creation 
		/*printf("here1\n");*/
		int z = 0;
		byte_by_byte_copy(dataframe,DATASYNC,z,2); // SYNC Word for data frame
		z += 2;
		/*printf("here2\n");*/
		memset(temp,'\0',3);
		int_to_ascii_convertor(total_frame_size,temp);
		byte_by_byte_copy(dataframe,temp,z,2); // FRAME SIZE
		z += 2;

		memset(temp,'\0',3);
		int_to_ascii_convertor(PDC_IDCODE,temp);
		byte_by_byte_copy(dataframe,temp,z,2); // PDC ID
		z += 2;

		byte_by_byte_copy(dataframe,(unsigned char *)df->soc,z,4); //SOC
		z += 4;
		byte_by_byte_copy(dataframe,(unsigned char *)df->fracsec,z,4); //FRACSEC 
		z += 4;
		/*printf("here3\n");*/
		unsigned char stat[2]; 
		switch(type){
		case 1:	stat[0] = 0x07;   // Stat for filtered data on violation
		stat[1] = 0x00;
		break;
		case 2: stat[0] = 0x07;	   // Stat for result of request for model creation
		stat[1] = 0x01;
		break;
		}

		byte_by_byte_copy(dataframe,stat,z,2); // stat
		z += 2;

		byte_by_byte_copy(dataframe,(unsigned char *)df->idcode,z,2); // PMU ID
		z += 2;

		memset(temp,'\0',3);
		int_to_ascii_convertor(nxt->api_id,temp);
		byte_by_byte_copy(dataframe,temp,z,2); // API ID
		z += 2;

		memset(temp,'\0',3);
		int_to_ascii_convertor(nxt->query_id,temp);
		byte_by_byte_copy(dataframe,temp,z,2); // QUERY ID
		z += 2;
		/*printf("here4\n");						*/


		float fp_i,fp_imaginary;
		unsigned char fp_left[4],fp_right[4];
		unsigned char *index;
		if(df->dpmu[0]->fmt->phasor == '1') { // Float
			/*printf("here4.1\n");						*/
			index = df->dpmu[0]->phasors[nxt->phasorNumber];
			memset(fp_left,'\0',5);
			memset(fp_right,'\0',5);
			copy_cbyc (fp_left,index,4);
			fp_left[4] = '\0';
			index += 4;

			copy_cbyc(fp_right,index,4);
			fp_right[4] = '\0';   

			/*			fp_r = decode_ieee_single(fp_left);*/
			fp_i = decode_ieee_single(fp_right);

			if(df->dpmu[0]->fmt->polar == '1'){
				/*				fp_real = fp_r;*/
				fp_imaginary = fp_i;
			}else{
				printf("Polar required.. Not applicable on Reactangular\n");
				break;
			}
		} 
		/*		else { // Fixed Point*/
		/*			index = df->dpmu[0]->phasors[nxt->phasorNumber];*/
		/*			memset(fp_left,'\0',3);*/
		/*			memset(fp_right,'\0',3);*/
		/*			copy_cbyc(fp_left,index,2);*/
		/*			fp_left[2] = '\0';*/
		/*			index += 2;*/

		/*			copy_cbyc(fp_right,index,2);*/
		/*			fp_right[2] = '\0';   */

		/*			fp_r = to_intconvertor(fp_left);*/
		/*			fp_i = to_intconvertor(fp_right);*/
		/*			if(df->dpmu[0]->fmt->polar == '1'){*/

		/*			struct cfg_frame *temp_cfg = cfgfirst;*/
		/*			// Check for the data frame IDCODE in Configuration Frame?*/
		/*			while(temp_cfg != NULL){*/
		/*				if(!ncmp_cbyc((unsigned char *) df->idcode,temp_cfg->idcode,2)) {*/
		/*					break;	*/
		/*				} else {*/
		/*					temp_cfg = temp_cfg->cfgnext;*/
		/*				}*/
		/*			}*/
		/*			fp_real = *temp_cfg->pmu[0]->phunit[nxt->phasorNumber] *fp_r;*/

		/*                	fp_imaginary = fp_i*1e-4; // Angle is in 10^4 radians*/
		/*			}else{*/
		/*				printf("Polar required.. Not applicable on Reactangular\n");*/
		/*				break;*/
		/*			}*/
		/*		}*/

		fp_imaginary = fp_imaginary*180/3.14; // Convert to degree
		fp_imaginary = fp_imaginary<0 ? fp_imaginary + 360 : fp_imaginary;

		memset(temp5,'\0',5);
		f2c(fp_imaginary,temp5);
		byte_by_byte_copy(dataframe,temp5,z,4); // Phasor Angle
		z += 4;

		byte_by_byte_copy(dataframe,df->dpmu[0]->freq,z,4); // Frequency
		z += 4;
		/*printf("here5\n");						*/
		chk = compute_CRC((unsigned char *)dataframe,z);
		dataframe[z] = (chk >> 8) & ~(~0<<8);  	// CHKSUM high byte; 
		dataframe[z+1] = (chk ) & ~(~0<<8);     // CHKSUM low byte;  
		z += 2;
		/*	if(type==2)	printf("Dispetching\n");			*/

		unsigned long esoc,efsec;
		int indx;
		unsigned char temp4[4];
		struct timeval tm;
		gettimeofday(&tm, NULL);
		esoc = tm.tv_sec;
		efsec = tm.tv_usec;

		indx = z;

		long_int_to_ascii_convertor(esoc,temp4);
		byte_by_byte_copy(dataframe,temp4,indx,4);
		indx = indx + 4;

		long_int_to_ascii_convertor(efsec,temp4);
		byte_by_byte_copy(dataframe,temp4,indx,4);

		dispatcher(dataframe,z+8,(void*)nxt->spdc);
		break;
		/*	default:*/
		/*		printf("Wrong API_ID to create\n");*/

	}

}


void application6(void *datafrm){
	struct data_frame *df = (struct data_frame*)datafrm;
	char key[16];
	sprintf(key,"%d",to_intconvertor((unsigned char*)df->idcode));
	ENTRY entry;
	ENTRY *found;
	entry.key = key;
	hsearch_r( entry, FIND, &found, &hashForPMUQuery);

	/*	pthread_mutex_lock(&mutex_query);*/

	if(found!=NULL){
		/*	printf("Found it..!!!\n");*/
		/*	printf("PMUID = %s\n",key);*/
		struct PMUQueryInfo * nxt = (struct PMUQueryInfo *)found->data;
		while(nxt != NULL){
			/*			printf("(%s %d %d) ",key,nxt->api_id,nxt->query_id);*/

			float fp_i,fp_imaginary;
			unsigned char fp_left[4],fp_right[4];

			unsigned char *index;

			/* Check according to API */
			switch(nxt->api_id){
			case 6 :	/* Angular Instability Check */

				if(df->dpmu[0]->fmt->phasor == '1') { // Float
					index = df->dpmu[0]->phasors[nxt->phasorNumber];
					memset(fp_left,'\0',5);
					memset(fp_right,'\0',5);
					copy_cbyc (fp_left,index,4);
					fp_left[4] = '\0';
					index += 4;

					copy_cbyc(fp_right,index,4);
					fp_right[4] = '\0';

					/*						fp_r = decode_ieee_single(fp_left);*/
					fp_i = decode_ieee_single(fp_right);

					if(df->dpmu[0]->fmt->polar == '1'){
						/*							fp_real = fp_r;*/
						fp_imaginary = fp_i;
					}else{
						printf("Polar required.. Not applicable on Reactangular\n");
						break;
					}

				}
				/*					else { // Fixed Point*/
				/*						index = df->dpmu[0]->phasors[nxt->phasorNumber];*/
				/*						memset(fp_left,'\0',3);*/
				/*						memset(fp_right,'\0',3);*/
				/*						copy_cbyc(fp_left,index,2);*/
				/*						fp_left[2] = '\0';*/
				/*						index += 2;*/

				/*						copy_cbyc(fp_right,index,2);*/
				/*						fp_right[2] = '\0';   */

				/*						fp_r = to_intconvertor(fp_left);*/
				/*						fp_i = to_intconvertor(fp_right);*/
				/*						if(df->dpmu[0]->fmt->polar == '1'){*/
				/*	                    				fp_imaginary = fp_i*1e-4; // Angle is in 10^4 radians*/
				/*						}else{*/
				/*							printf("Polar required.. Not applicable on Reactangular\n");*/
				/*							break;*/
				/*						}*/
				/*					}*/

				fp_imaginary = fp_imaginary*180/3.14; // Convert to degree

				long int currSoc = to_long_int_convertor(df->soc);
				long int currFracsec = to_long_int_convertor(df->fracsec);
				int secdiff = currSoc - (nxt->invoke_soc);
				int fracsecdiff = ((currFracsec - (nxt->invoke_fracsec)));
				float T = (float)secdiff + fracsecdiff*1e-6;

				float lower_bound = fmod(((nxt->bounds_slope) * T + nxt->displace_lb ),(360));
				float upper_bound = fmod(((nxt->bounds_slope) * T + nxt->displace_ub ),(360));

				/* Round off in positive angle */
				lower_bound = lower_bound<0 ? lower_bound + 360 : lower_bound;
				upper_bound = upper_bound<0 ? upper_bound + 360 : upper_bound;
				fp_imaginary = fp_imaginary<0 ? fp_imaginary + 360 : fp_imaginary;

				/*					lower_bound = lower_bound * 180/3.14;*/
				/*					upper_bound = upper_bound * 180/3.14;*/
				/*					fp_imaginary = fp_imaginary * 180/3.14;*/

				int flag = 1;
				if(upper_bound<90 && lower_bound>270){
					if(fp_imaginary>=0 && fp_imaginary<=upper_bound)
						flag = 0;
					else if(fp_imaginary<=360 && fp_imaginary>=lower_bound)
						flag = 0;
				}else{
					if(fp_imaginary>=lower_bound && fp_imaginary<=upper_bound)
						flag = 0;
				}

				if(flag==1){
					/* Dispatch the frame to SPDC*/
					/* Do some changes in Dataframe*/
					/*printf("Local Violation [pmuid = %d], [lower_bound = %f], [upper_bound= %f], [fp_imaginary = %f], [T = %f], [Soc = %ld], [fracsec = %ld] \n",nxt->pmuid,lower_bound,upper_bound,fp_imaginary,T,currSoc,currFracsec);*/
					/*printf("\nViol pid = %d, lb = %f, ud= %f, ang = %f, T = %f, currSoc = %ld, currfsec = %ld, invoSoc = %ld, invofsec = %ld, secdiff = %d, fracsecdiff = %d. \n",nxt->pmuid,lower_bound,upper_bound,fp_imaginary,T,currSoc,currFracsec,nxt->invoke_soc,nxt->invoke_fracsec,secdiff,fracsecdiff);*/
					/*						printf("Local Violation\n");*/

					pthread_mutex_lock(&mutex_App6Analysis);
					total_pkt_arrivals++;
					total_local_violation++;
					pthread_mutex_unlock(&mutex_App6Analysis);

					create_send((void*)nxt,datafrm,1);
					/*						exit(0);*/
					/*						writeTimeToLog(2,nxt->pmuid,currSoc,currFracsec);*/
				}
				else{
					/*printf("******** [pmuid = %d], [lower_bound = %f], [upper_bound= %f], [fp_imaginary = %f], [T = %f], [Soc = %ld], [fracsec = %ld] \n",nxt->pmuid,lower_bound,upper_bound,fp_imaginary,T,currSoc,currFracsec);*/

					/*printf("\nNo Viol pid = %d, lb = %f, ud= %f, ang = %f, T = %f, currSoc = %ld, currfsec = %ld, invoSoc = %ld, invofsec = %ld, secdiff = %d, fracsecdiff = %d. \n",nxt->pmuid,lower_bound,upper_bound,fp_imaginary,T,currSoc,currFracsec,nxt->invoke_soc,nxt->invoke_fracsec,secdiff,fracsecdiff);*/
					pthread_mutex_lock(&mutex_App6Analysis);
					total_pkt_arrivals++;
					total_filtered++;
					pthread_mutex_unlock(&mutex_App6Analysis);
				}


				break;
			default:
				printf("Wrong API_ID\n");
			}
			nxt = nxt->nextQuery;
		}
		/*		printf("\n");	*/

		Analysing(63,atoi(key),0,0); 		

		pthread_mutex_lock(&mutex_App6Analysis);
		app6CurrentPmuCount++;
		if(app6CurrentPmuCount == app6PmuCount){
			app6CurrentPmuCount = 0;
			if(flagapp6 == 0 && total_pkt_arrivals > 5*1300){
				Analysing(69,0,0,0);
			}
		}
		pthread_mutex_unlock(&mutex_App6Analysis);

	}
	/*	pthread_mutex_unlock(&mutex_query);*/

}


void dispatcher(unsigned char* dataframe, int size, void *s){
	struct Upper_Layer_Details * spdc = (struct Upper_Layer_Details *)s;
	/*				printf("here1\n");*/
	if(spdc->port == UDPPORT) {
		/*printf("here2\n");*/
		if (sendto(spdc->sockfd,dataframe,size,0,(struct sockaddr *)&spdc->pdc_addr,sizeof(spdc->pdc_addr)) == -1)
			perror("sendto");
	} else if((spdc->port == TCPPORT) && (spdc->tcpup == 1)) {

		if(send(spdc->sockfd,dataframe,size, 0)== -1) {
			perror("send");
			spdc->tcpup = 0;
			pthread_cancel(spdc->thread_id);
		}
	}

}


void* execute_query(void * q){
	//printf("In execute query\n");
	struct PMUQueryInfo* temp_query = (struct PMUQueryInfo*)q;

	/*	writeTimeToLog(5,temp_query->pmuid,temp_query->invoke_soc,temp_query->invoke_fracsec);*/

	while(1){
		int flag = 0;
		int i;
		unsigned long int tsb_soc,tsb_fracsec;
		pthread_mutex_lock(&mutex_on_TSB);

		/*		pthread_mutex_lock(&mutex_on_thread);	*/

		if (front == -1) { // TSB is empty
			printf("Its too early..");
			break;
		}else{
			unsigned char *tsb_fsec;
			tsb_fsec = malloc(3*sizeof(unsigned char));
			tsb_fsec[0] = TSB[front].fracsec[1];
			tsb_fsec[1] = TSB[front].fracsec[2];
			tsb_fsec[2] = TSB[front].fracsec[3];

			tsb_soc = to_long_int_convertor((unsigned char *)TSB[front].soc);

			tsb_fracsec = to_long_int_convertor1(tsb_fsec);	

			if((old_df_front != -1) &&  ((temp_query->invoke_soc < ODFT[old_df_front].soc) || ((temp_query->invoke_soc == ODFT[old_df_front].soc) && (temp_query->invoke_fracsec < ODFT[old_df_front].fracsec)))) {
				printf("Its too old...See in Database\n"); // Least Possible condition
				break;
			}
			/*printf("temp_query->invoke_soc = %ld, old_df_front = %d, ODFT[old_df_front].soc = %d\n",temp_query->invoke_soc,old_df_front,ODFT[old_df_front].soc);*/
			/*printf("temp_query->invoke_fracsec= %ld, ODFT[old_df_front].fracsec= %d\n",temp_query->invoke_fracsec,ODFT[old_df_front].fracsec);*/
			/*printf("Required (%ld %ld)  Old_df_front (%d %d)\n",temp_query->invoke_soc,temp_query->invoke_fracsec,ODFT[old_df_front].soc,ODFT[old_df_front].fracsec);*/

			for(i = 0; i<MAXTSB; i++) {
				//	memset(dLog,'\0',2500);
				if((TSB[i].soc != NULL ) &&(TSB[i].first_data_frame != NULL)) {

					tsb_soc = to_long_int_convertor((unsigned char *)TSB[i].soc);
					tsb_fsec[0] = TSB[i].fracsec[1];
					tsb_fsec[1] = TSB[i].fracsec[2];
					tsb_fsec[2] = TSB[i].fracsec[3];
					tsb_fracsec = to_long_int_convertor1(tsb_fsec);				

					/*			printf("(%ld %ld)->",tsb_soc,tsb_fracsec);*/
					if(temp_query->invoke_soc == tsb_soc) 
					{
						if(tsb_fracsec == temp_query->invoke_fracsec) {	

							flag = 1;						
							break;
						} 
					}
				}
			}
			/*			printf("maxtsb= %d Tsb_Count= %d i=%d num=%d, pmuid = %d,  apiid = %d, queryid = %d\n",MAXTSB,Tsb_Count,i,TSB[i].num,temp_query->pmuid,temp_query->api_id,temp_query->query_id);*/
			free(tsb_fsec);
		} // if other than the front = -1
		if(flag) {
			// Found in TSB
			//printf("Found in TSB...\n");

			struct data_frame *check_df;

			/* Need to check if df with same idcode and soc is already assigned to 
			   the TSB[index] */
			check_df = TSB[i].first_data_frame;			
			while(check_df != NULL) {
				int df_idcode = to_intconvertor(check_df->idcode);
				if(df_idcode == temp_query->pmuid) {
					/*					printf("Found pmu\n");*/
					create_send((void*)temp_query,(void*)check_df,2);
					break;
				} else {

					check_df = check_df->dnext;

				}							
			}
			pthread_mutex_unlock(&mutex_on_TSB);
			/*			pthread_mutex_unlock(&mutex_on_thread);*/
			if(check_df != NULL)
				break;
		}else{
			//printf("Not arrived Yet...\n");
			/*			pthread_mutex_unlock(&mutex_on_thread);	*/
			pthread_mutex_unlock(&mutex_on_TSB);
		}

	}
	free(temp_query);
	pthread_exit(NULL);
}

//mayur change
void application7(void *datafrm){

/*
	pthread_mutex_lock(&mutex_App7Analysis);
	total_pkt_arrivals7++;
	pthread_mutex_unlock(&mutex_App7Analysis);
*/


}




/* ----------------------------------------------------------------------------	*/
/* FUNCTION decode_ieee_single():                                	     	*/
/* pass unsigned char[4].											*/
/* ----------------------------------------------------------------------------	*/

float c2f_ieee(const void *v) 
{
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


