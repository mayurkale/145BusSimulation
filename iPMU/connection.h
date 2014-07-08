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



/* ---------------------------------------------------------------- */
/*                         global variables                         */
/* ---------------------------------------------------------------- */


/* ------------------------------------------------------------------ */
/*                       Global Datastructure                         */
/* ------------------------------------------------------------------ */



/* ------------------------------------------------------------------ */
/*                       Function prototypes                          */
/* ------------------------------------------------------------------ */

void* sender(void * info);

void server(int id, int uport, int tport, int index);

void* SEND_DATA();

void* UDP_PMU();

void* TCP_PMU();

void* dataWriteInFile();
void writeTimeToLog(int index,unsigned int IDcode,unsigned long int l_soc,unsigned long int l_fracsec);

/**************************************** End of File *******************************************************/
