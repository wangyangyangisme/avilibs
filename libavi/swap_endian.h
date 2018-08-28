/*  
 * Set of functions that change data endiannes.
 *  
 * Copyright (C) 2001 Petros Tsantoulis <ptsant@otenet.gr>
 *
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 *
 *     This program is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU General Public License for more details.
 *
 *     You should have received a copy of the GNU General Public License
 *     along with this program; if not, write to the Free Software
 *     Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111, USA.
 */



/********************************************************************/
/* NOTE : I DO NOT OWN A BIG-ENDIAN MACHINE. THIS CODE MIGHT BE
   TERRIBLY WRONG. IF YOU CARE ABOUT IT, EMAIL ME WITH DETAILS
   OR FIX IT YOURSELF. 
*/

#ifndef _SWAP_H
#define _SWAP_H

#include <endian.h>
#include <stdint.h>

#include "defines.h"
#include "avi_structures.h"

#if __BYTE_ORDER == __LITTLE_ENDIAN

#ifndef INT_TO_LE_INT
#define INT_TO_LE_INT(x) (x)
#endif

#ifndef WORD_TO_LE_WORD
#define WORD_TO_LE_WORD(x) (x)
#endif

#else


#ifndef INT_TO_LE_INT
/* Please note: this is not fast. */
#define INT_TO_LE_INT(x) ((x&0xff)<<24+((x>>8)&0xff)<<16+((x>>16)&0xff)<<8+((x>>24)&0xff)))
#endif

#ifndef WORD_TO_LE_WORD
#define WORD_TO_LE_WORD(x) ((x&0xff)<<8 + ((x>>8)&0xff))
#endif

#endif /* __BYTE_ORDER == __LITTLE_ENDIAN */

/* Accepts a pointer to uint32_t data. The data is unconditionally
   swapped from 1234 to 4321, changing endiannes. You should not
   use this function because it does NOT check for machine
   endiannes. */
/* CAUTION!!!! THIS FUNCTION OPERATES ON 32-bit CHUNKS!!! 
   IF A STRUCTURE CONTAINS 16-bit VALUES IT WILL BE MESSED
   UP. (e.g. BITMAPINFOHEADER)
   YOU WERE WARNED. */
uint32_t
*swap_endian_by4(uint32_t *input, uint32_t size);


/* This is so ugly!!! Well, whatever... */

BITMAPINFOHEADER *
swap_bmh(BITMAPINFOHEADER *input);

/* This is the proper interface to swap_endian(). If
   the machine is little-endian, swap_endian is skipped. */
#if __BYTE_ORDER == __LITTLE_ENDIAN 
#define SWAP_ENDIAN_BY4(input_pointer, size)  (input_pointer)
#else
#define SWAP_ENDIAN_BY4(input_pointer, size)  (swap_endian_by4((input_pointer),(size)))
#endif


#if __BYTE_ORDER == __LITTLE_ENDIAN
#define SWAP_ENDIAN_BMH(input_pointer) (input_pointer)
#else
#define SWAP_ENDIAN_BMH(input_pointer) (swap_bmh(input_pointer))
#endif

/* Avi header */
#define SWAP_ENDIAN_AVIH(input_pointer) (SWAP_ENDIAN_BY4((input_pointer),sizeof(MainAVIHeader)))

/* Stream header */
#define SWAP_ENDIAN_STRH(input_pointer) (SWAP_ENDIAN_BY4((input_pointer),sizeof(AVIStreamHeader)))

/* BITMAPINFO header */
#define SWAP_ENDIAN_BMI(input_pointer)  (SWAP_ENDIAN_BMH(input_pointer))

/* WAVEFORMAT ? ? ? */

#endif 







