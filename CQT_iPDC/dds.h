/* ----------------------------------------------------------------------------- 
 * dds.h
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

char dds2Key[5],*dds2K,ddsTempKey[5],*ddsTempK;
void cfgparser_For_DDS2(struct cfg_frame *cfg,int j, ENTRY *found,int multiFlag);
int create_DDS2_dataframe(struct data_frame *df);
void attacheEachdataframeInDDS2(ENTRY *found,struct data_frame *temp_df,int z,int multiFlag);
struct data_frame* generate_dummy_dataframe_DDS2(ENTRY *found,unsigned char *idcode);