/*  
 * Set of functions that manipulate a genric queue data structure
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

#include "queue.h"

#ifdef CHECKS
#include <stdio.h>
#endif
#include <string.h>

/* This file uses the prefix QDS to mark all functions. This 
   is done in order to make sure that they won't match with other
   libraries that may be linked. QDS=Queue Data Structure. */

/*************************************************************************/
/* Returns pointer on success, 0 on failure */
queue_handle *
QDScreate_queue(void)
{
  queue_handle *ret;

  ret=(queue_handle *)malloc(sizeof(queue_handle));
  
#ifdef CHECKS
  if (ret==NULL) {
    fprintf(stderr,"create_queue() : failed to allocate memory! \n");
    return(0);
  }
#endif

  memset((void *)ret, 0x0, sizeof(ret));

  ret->current_link=&(ret->chain);

  return(ret);
}


/*************************************************************************/
/* This adds a data chunk to the queue. Returns 0 on success, <0 on 
   failures. */
int
QDSqadd(queue_handle *handle, void *data, int size, int data_type)
{
  queue_type *p;

#ifdef CHECKS
  if (handle==0) {
    fprintf(stderr,"qadd() : handle pointer is NULL!!! Cannot continue. \n");
    return(-1);
  }
#endif

#ifdef CHECKS
  if (data==0) {
    fprintf(stderr,"qadd() : data pointer is NULL!!! Cannot continue. \n");
    return(-2);
  }
#endif

#ifdef CHECKS
#ifdef VERBOSE
  if (size==0) {
    fprintf(stderr,"qadd() : Warning: data size is 0. \n");
    return(-1);
  }
#endif
#endif
	    
  p=(queue_type *)malloc(sizeof(queue_type));

#ifdef CHECKS
  if (p==0) {
    fprintf(stderr,"qadd(,,%d,%d) : Could not allocate memory! Must fail! \n",
	    size, data_type);
    return(-4);
  }
#endif

  p->data=malloc(size);

#ifdef CHECKS
  if (p->data==0) {
    fprintf(stderr,"qadd(,,%d,%d) : Could not allocate memory for p->data. Must fail! \n",
	    size, data_type);
    return(-5);
  }
#endif
  
  memcpy(p->data,data,size);

#ifdef EXTENDED_CHECKS
  if (handle->current_link==0) {
    fprintf(stderr,"qadd(,,%d,%d) : handle->current_link == 0! This should not happen! \n",
	    size, data_type);
    return(-6);
  }
#endif

  handle->current_link->next=p;
  p->next=0;
  handle->current_link=p;
  p->data_size=size;
  p->data_type=data_type;

  handle->queue_size++;

  return(0);
  
}


/*************************************************************************/
/* This seeks to a certain queue position. Returns 0 on success, <0 on
   failures. */
int
QDSqseek(queue_handle *handle, int member)
{
  queue_type *t;
  register int i;

#ifdef CHECKS
  if (handle==0){
    fprintf(stderr,"qseek() : Failure! : received a NULL handle! \n");
    return(-1);
  }

  if (member<0) {
    fprintf(stderr,"qseek() : Failure! : cannot seek to position %d!\n",
	    member);
    return(-2);
  } else if (member>handle->queue_size) {
        fprintf(stderr,"qseek() : Failure! : cannot seek to position %d. Only %d members \n",
	    member, handle->queue_size);
    return(-3);
  }
#endif
    
  t=handle->chain.next;
  for (i=0; i<member; i++)
    t=t->next;

  handle->current_link=t;

  return(0);

}
  
  
    


/*************************************************************************/
/* Returns data size at the current link. <0 on various failures. */
/* You >MUST< use qlinksize before qget so that
   link data will fit in the provided pre-allocated
   buffer. qget() will not do any checks other
   than verify that data is not NULL
*/
int
QDSqgetlinksize(queue_handle *handle)
{

#ifdef CHECKS
  if (handle==0){
    fprintf(stderr,"qlinksize() : Failure! : cannot work with a NULL handle!\n");
    return(-1);
  }
#endif

  return(handle->current_link->data_size);
}



/*********************************************************************/
/* You >MUST< use qlinksize before qget so that
   link data will fit in the provided pre-allocated
   buffer. qget() will not do any checks other
   than verify that data is not NULL

   qget() will write current links data contents to the provided 
   data buffer. You MUST allocate enough space! 
*/
int
QDSqgetdata(queue_handle *handle, void *data)
{

#ifdef CHECKS
  if (handle==0){
    fprintf(stderr,"qget() : Failure! : cannot work with a NULL handle!\n");
    return(-1);
  }

  if (handle->current_link==0) {
    fprintf(stderr,"qget() : Failure! : current_link is a NULL pointer! \n");
    return(-2);
  }

 if (handle->current_link->data==0) {
    fprintf(stderr,"qget() : Failure! : current_link->data is a NULL pointer! \n");
    return(-3);
  }
#endif

 memcpy(data, handle->current_link->data, handle->current_link->data_size);

 return(0);

}


/*****************************************************************************/
/* Get the data_type at the current link. This almost certainly won't fail. 
   Returns data_type on success. On failure <0 is returned. This is only
   useful if data_type cannot be <0. 
*/
int
QDSqgetdatatype(queue_handle *handle)
{
#ifdef CHECKS
  if (handle==0){
    fprintf(stderr,"qgetdatatype() : Failure! : cannot work with a NULL handle!\n");
    return(-1);
  }

  if (handle->current_link==0) {
    fprintf(stderr,"qgetdatatype() : Failure! : current_link is a NULL pointer! \n");
    return(-2);
  }
#endif
  
  return(handle->current_link->data_type);
}


/*****************************************************************************/
int
QDSqadvance(queue_handle *handle)
{
#ifdef CHECKS
  if (handle==0){
    fprintf(stderr,"qadvance() : Failure! : cannot work with a NULL handle!\n");
    return(-1);
  }

  if (handle->current_link==0) {
    fprintf(stderr,"qadvance() : Failure! : current_link is a NULL pointer! \n");
    return(-2);
  }
#endif

 /* Advance to the next link. This is the default action. */
 handle->current_link=handle->current_link->next;

 return(0);
  
}


/*****************************************************************************/
/* Returns 0 on success, <0 on various failures. */
/* PLEASE NOTE THAT IT WILL ADVANCE TO THE NEXT LINK. NO NEED TO USE qadvance()! */
int
QDSqdel(queue_handle *handle)
{
  register queue_type *tmp;

#ifdef CHECKS
  if (handle==0){
    fprintf(stderr,"qdel() : Failure! : cannot work with a NULL handle!\n");
    return(-1);
  }

  if (handle->current_link==0) {
    fprintf(stderr,"qdel() : Failure! : current_link is a NULL pointer! \n");
    return(-2);
  }
#endif

  /* Remove the links */
  tmp=handle->current_link;
  handle->current_link=tmp->next;
  
  /* Free the memory */
  free(tmp->data);
  free(tmp);

  /* The queue size has shrunk. */
  handle->queue_size--;
  
#ifdef EXTENDED_CHECKS
  if (handle->queue_size<0){
    fprintf(stderr,"qdel() : handle->queue_size is %d after deletion. This is weird. \n",
	    handle->queue_size);
    return(-3);
  }
#endif

  /* Done! */

  return(0);

}
