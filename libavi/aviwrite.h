/*
 * Header file for basic AVI I/O routines and custom structures ;-)
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

#ifndef _AVIWRITE_H
#define _AVIWRITE_H

#include <stdio.h>
#include <stdint.h>
#include <endian.h>

#include "fourcc.h"
#include "wformat.h"
#include "avicodes.h"
#include "riff_handle.h"
#include "riffwrite.h"
#include "defines.h"
#include "avi_structures.h"
#include "queue.h"
#include "avi_handle.h"

/* strd chunk : pass "as-is" to decompressor */
/* strn chunk : name of codec, zero terminated text string */


/*****************************************************************/
/* This simple function copies the MainAVIHeader parameter that it
gets to the internal handle data structure. One can also 
choose to do this by directly accessing the avi_handle structure, 
but this is bad programming. The proper way to do it is to use
modify_avi_header(). All it does is memcpy().

Returns 0 on success, <0 on failure */

int
AVWmodify_avi_header(avi_handle *handle, MainAVIHeader *header);

/**************************************************************/
/* Creates the avi file from scratch, also opens the stream list. */
/* After create_avi_file() we are ready to add stream headers */
/* Needs a filename AND a MainAVIHeader (that may be nonsense) */
/* Returns avi_handle * if OK, 0 on various errors. */
/* Please note : This function REPLACES create_avi_header(). */
/* You DO NOT NEED TO CALL create_avi_header afterwards. */
avi_handle *
AVWcreate_avi_file(char *filename, MainAVIHeader *header);

/**************************************************************/
/* Creates the avi header, also opens the stream list. */
/* After create_avi_header() we are ready to write list headers. */
/* Needs an avi_handle AND a MainAVIHeader */
/* Returns avi_handle * if OK, 0 on various errors. */
avi_handle *
AVWcreate_avi_header(riff_handle *handle, MainAVIHeader *header);

/**********************************************************************/
/* Closes the hdrl and strl lists. Also writes a JUNK chunk of padding*/
/* until the nearest 4k boundary (4k alignment, for fastest access)   */
/* You must rewrite (update) the header when closing the file */
/* JUNK padding is by default modulo 4k. This should give some 1-2k   */
/* JUNK on most files. You may specify a minimum junk size if you plan*/
/* to add streams later. (currently not supported!) */
/* ALLOCATES MEMORY, THAT IS LATER FREED. */
/* 0 on success, -1 on failure */
int
AVWclose_avi_header(avi_handle *handle, uint32_t min_junk_size);

/*****************************************************************/
/* Updates the avi header after the file has been completed.     */
/* What it does is write the number of frames etc. This is       */
/* necessary to create a valid .avi file. */
/* NOTE : update_avi_header uses the handle->avi_header          */
/* This should be maintained internally by the library. */
/* Returns 0 on success, <0 on failure. */
int 
AVWupdate_avi_header(avi_handle *handle);


/*****************************************************************/
/* Keeps idx information by storing it in the queue. */
int
AVWadd_idx(avi_handle *handle, AVIINDEXENTRY *index);


/****************************************************************/
/* This is called when no more frames are going to be written.
   This function writes the idx1 chunk to the file, closes the
   movi list and effectively finalises it.

   Returns number of indices written. <0 on various failures.
*/
int
AVWwrite_index_chunk(avi_handle *handle);

#endif













