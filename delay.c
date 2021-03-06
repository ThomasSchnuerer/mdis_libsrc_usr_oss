/*********************  P r o g r a m  -  M o d u l e ***********************/
/*!  
 *        \file  delay.c
 *
 *      \author  klaus.popp@men.de
 * 
 * 	   \project  MDISforLinux USR_OSS lib
 *
 *  	 \brief  Delay function
 *
 *    \switches  none
 */
/*
 *---------------------------------------------------------------------------
 * Copyright 2000-2019, MEN Mikro Elektronik GmbH
 ******************************************************************************/
/*
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
 
#include "uos_int.h"

/**********************************************************************/
/** Let a process sleep for a specified time
 *
 * \copydoc usr_oss_specification.c::UOS_Delay()
 *
 * \sa UOS_MikroDelay
 */
int32 UOS_Delay(u_int32 msec)
{
	struct timespec request, remain;
	u_int32 current, start, expires;
	int32 left;

	start = current = UOS_MsecTimerGet();
	expires = current + msec;

	while( (left = expires - current) > 0 ) { 

		/*printf("UOS_Delay: msec=%d\n", msec );*/
		request.tv_sec = left / 1000;
		request.tv_nsec = (left % 1000) * 1000000;

		nanosleep( &request, &remain );
		
		current = UOS_MsecTimerGet();
	} 

	return current-start;
}


