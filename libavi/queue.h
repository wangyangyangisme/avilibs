/*  
 * Header file for a generic queue data structure.
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

#ifndef _QUEUE_H
#define _QUEUE_H

#include <stdlib.h>
#include "defines.h"

typedef struct _queue_type {
  struct _queue_type *next;
  unsigned char *data;
  int data_size;
  /* This is for the caller. We don't care about it. 
     It's just so that the caller can know what is in
     here. */
  int data_type; 
} queue_type;

typedef struct _queue_handle {
  queue_type chain;
  int queue_size;
  queue_type *current_link;
} queue_handle;


/* This file uses the prefix QDS to mark all functions. This 
   is done in order to make sure that they won't match with other
   libraries that may be linked. QDS=Queue Data Structure. */

/*************************************************************************/
/* Returns pointer on success, 0 on failure */
queue_handle *
QDScreate_queue(void);


/*************************************************************************/
/* This adds a data chunk to the queue. Returns 0 on success, <0 on 
   failures. */
int
QDSqadd(queue_handle *handle, void *data, int size, int data_type);

/*************************************************************************/
/* This seeks to a certain queue position. Returns 0 on success, <0 on
   failures. */
int
QDSqseek(queue_handle *handle, int member);


/*****************************************************************************/
/* Get the data_type at the current link. This almost certainly won't fail. 
   Returns data_type on success. On failure <0 is returned. This is only
   useful if data_type cannot be <0.
*/
int
QDSqgetdatatype(queue_handle *handle);


/*************************************************************************/
/* Returns the link size. Will return <0 if something does not make
   sense. (e.g. handle=NULL) */
int
QDSqgetlinksize(queue_handle *handle);



/*****************************************************************************/
/* You >MUST< use qlinksize before qget so that
   link data will fit in the provided pre-allocated
   buffer. qget() will not do any checks other
   than verify that data is not NULL

   qget() will write current links data contents to the provided 
   data buffer. You MUST allocate enough space! 
*/
int
QDSqgetdata(queue_handle *handle, void *data);


/*****************************************************************************/
/* Returns 0 on success, <0 on various failures. 
   Advances to the next link. */
int 
QDSqadvance(queue_handle *handle);


/*****************************************************************************/
/* Returns 0 on success, <0 on various failures. 
   PLEASE NOTE THAT IT WILL ADVANCE TO THE NEXT LINK. NO NEED TO USE qadvance()! */
int
QDSqdel(queue_handle *handler);

#endif


