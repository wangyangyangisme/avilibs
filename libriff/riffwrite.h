/*
 * Header file for basic riff I/O routines 
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



#ifndef _RIFFWRITE_H
#define _RIFFWRITE_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "riffcodes.h" 
#include "defines.h"
#include "riff_handle.h"


/* NOTE : all functions in these file have the RFW prefix, meaning
   Riff Write. Please do not change this convention. Functions with
   the RFR prefix are meant for READING riff files. */
EXPORT uint32_t 
_RFWs2fcc(char i0, char i1, char i2, char i3);

EXPORT char *
_RFWfcc2s(uint32_t input);

/*******************************************************************/
/* Aligns the file by adding a junk chunk of the appropriate size. */
/* This is particularly useful when the file is going to be read */
/* off a cd, where choosing a chunk alignment of 2048 (=sector size) */
/* can be quite beneficial. This is of course a JUNK chunk. */
/* Input is a riff_handle and the desired boundary, output is
   the number of bytes written. */

/* Please note that to achieve 2048 byte alignment for the chunk
   data, boundary is 2048 minus 8 for the chunk header and
   therefore you should use a default alignment of 2040 which
   will make your stream data start at 2048. */
EXPORT int
RFWalign_riff(riff_handle *handle, int boundary);

/*******************************************************************/
/* Returns number of bytes written. NOTE : length+8, normally      */
EXPORT int
RFWwrite_chunk(riff_handle *handle, uint32_t type, uint32_t length, unsigned char *data);

/*******************************************************************/
/* Returns number of bytes written. NOTE : length+8, normally      */
/* Does not increase file size. Does NOT do alignment. */

EXPORT int
RFWrewrite_chunk(riff_handle *handle, uint32_t type, uint32_t length, unsigned char *data, long file_pos);

/*******************************************************************/
/* Returns 12 on success, number of bytes written (<12) on failure */
EXPORT int
RFWcreate_list(riff_handle *handle, uint32_t type);

/********************************************************************/
/* 1 on success, 0 on failure */
EXPORT int
RFWclose_list(riff_handle *handle);

/*******************************************************************/
/* This writes a fake header with whatever information we'd like.
   It is used in conjunction with write_raw_chunk to make any
   sort of chunk.
   
   Returns 0 on success, <0 on various failures.
*/

EXPORT int
RFWwrite_chunk_header(riff_handle *handle, uint32_t type, uint32_t length);

/*******************************************************************/ 
/* Returns number of bytes written. NOTE : length normally      */
/* Does not write "legal" chunks. Only writes raw data.  */
EXPORT int
RFWwrite_raw_chunk(riff_handle *handle, uint32_t length, unsigned char *data);

/***************************************************************/
/* Returns an avi_handle pointer. Also allocates data structures. */
/* This structure is used to keep the FILE * and also keep track */
/* of lists. DOES MEMORY ALLOCATION. */

EXPORT riff_handle *
RFWcreate_riff_handle(char *filename, uint32_t riff_type);

/***********************************************************/
/* Closes the RIFF file. FREES MEMORY.*/
/* Returns -1 on failure, 0 if OK */
EXPORT int
RFWclose_riff(riff_handle *handle);





#endif


