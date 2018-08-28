/*  
 * Set of functions that ease the creation of an avi file.
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



#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define _INSIDE_AVIWRITE /* Do not declare extern! */

#include "aviwrite.h"

/*****************************************************************/
/* This simple function copies the MainAVIHeader parameter that it
gets to the internal handle data structure. One can also 
choose to do this by directly accessing the avi_handle structure, 
but this is bad programming. The proper way to do it is to use
modify_avi_header().

Returns 0 on success, <0 on failure */

int
AVWmodify_avi_header(avi_handle *handle, MainAVIHeader *header)
{

#ifdef CHECKS
  if (handle==NULL) {
    fprintf(stderr,"modify_avi_header(0x%x, 0x%x) : Failure! : no handle given! \n",
	    (unsigned int)handle, (unsigned int)header);
    return(-1);
  }
    
  if (header==NULL) {
    fprintf(stderr,"modify_avi_header(0x%x, 0x%x) : Failure! : no header given! \n",
	    (unsigned int)handle, (unsigned int)header);
    return(-2);
  }
#endif

  /* Updating handle information */
  memcpy((void *)&handle->avi_header, (void *)header, sizeof(MainAVIHeader));

  return(0); /* That's all folks! */
}

/**************************************************************/
/* Creates the avi file from scratch, also opens the stream list. */
/* After create_avi_header() we are ready to add stream headers */
/* Needs an avi_handle AND a MainAVIHeader */
/* Returns avi_handle * if OK, 0 on various errors. */
avi_handle *
AVWcreate_avi_file(char *filename, MainAVIHeader *header)
{
  avi_handle *output;
  riff_handle *handle;

  handle=RFWcreate_riff_handle(filename, FOURCC('A','V','I',' '));

#ifdef CHECKS
  if (handle==NULL) {
    fprintf(stderr," %s:%d: %s: Failure! : could not create riff handle! \n",
	    __FILE__, __LINE__, __FUNCTION__);
    return(0);
  }
#endif

  output=(avi_handle *)malloc(sizeof(avi_handle));
  
#ifdef CHECKS
  if (output==0) {
    fprintf(stderr,"create_avi_header() : cannot allocate memory for avi_handle!\n");
    return(0);
  }
#endif

  output->idx_list=QDScreate_queue();

#ifdef CHECKS
  if (output->idx_list==0){
    fprintf(stderr," %s:%d: %s: Failure! : could not create idx_list queue! \n",
	    __FILE__, __LINE__, __FUNCTION__);
    return(0);
  }
#endif

  output->riff_file=handle;

  if (AVWmodify_avi_header(output, header)!=0){
#ifdef VERBOSE
    fprintf(stderr,"create_avi_header(0x%x, 0x%x) : modify_avi_header failed! \n",
	    (unsigned int)handle, (unsigned int)header);
    fprintf(stderr,"create_avi_header(0x%x, 0x%x) : Trying to continue with bad header...\n",
	    (unsigned int)handle, (unsigned int)header);
#endif
    /* We don't fail if this happens because the header will be updated when the
       file closes. */
  }


  /* Open the header list */
  if (RFWcreate_list(handle, ophdrl)!=12) {
#ifdef VERBOSE
    fprintf(stderr,"create_avi_header() : unexpected result! create_list(hdrl) failed?\n");
    fprintf(stderr,"create_avi_header() : trying to continue... \n");
#endif
  }

  /* Write the header */
  /* We don't care if it is valid right now. */
  if (RFWwrite_chunk(handle, opavih, sizeof(MainAVIHeader), (unsigned char *)header)!=
      sizeof(MainAVIHeader)+8) {
#ifdef VERBOSE
    fprintf(stderr,"create_avi_header() : writing the MainAVIHeader failed!?!? \n");
    fprintf(stderr,"create_avi_header() : trying to continue...\n");
#endif
  }
  
  /* Now the stream list must follow. This can be done either manually,
     or with the create_stream functions. */
  
  return(output);

}

  


/**************************************************************/
/* Creates the avi header, also opens the stream list. */
/* After create_avi_header() we are ready to add stream headers */
/* Needs an avi_handle AND a MainAVIHeader */
/* Returns avi_handle * if OK, 0 on various errors. */
avi_handle *
AVWcreate_avi_header(riff_handle *handle, MainAVIHeader *header)
{
  avi_handle *output;

#ifdef CHECKS
  if (handle==NULL) {
    fprintf(stderr,"create_avi_header(0x%x, 0x%x) : Failure! : no handle given! \n",
	    (unsigned int)handle, (unsigned int)header);
    return(0);
  }
#endif

  output=(avi_handle *)malloc(sizeof(avi_handle));
  
#ifdef CHECKS
  if (output==0) {
    fprintf(stderr,"create_avi_header() : cannot allocate memory for avi_handle!\n");
    return(0);
  }
#endif

  output->idx_list=QDScreate_queue();

#ifdef CHECKS
  if (output->idx_list==0){
    fprintf(stderr," %s:%d: %s: Failure! : could not create idx_list queue! \n",
	    __FILE__, __LINE__, __FUNCTION__);
    return(0);
  }
#endif


  output->riff_file=handle;

  if (AVWmodify_avi_header(output, header)!=0){
#ifdef VERBOSE
    fprintf(stderr,"create_avi_header(0x%x, 0x%x) : modify_avi_header failed! \n",
	    (unsigned int)handle, (unsigned int)header);
    fprintf(stderr,"create_avi_header(0x%x, 0x%x) : Trying to continue with bad header...\n",
	    (unsigned int)handle, (unsigned int)header);
#endif
    /* We don't fail if this happens because the header will be updated when the
       file closes. */
  }


  /* Open the header list */
  if (RFWcreate_list(handle, ophdrl)!=12) {
#ifdef VERBOSE
    fprintf(stderr,"create_avi_header() : unexpected result! create_list(hdrl) failed?\n");
    fprintf(stderr,"create_avi_header() : trying to continue... \n");
#endif
  }

  /* Write the header */
  /* We don't care if it is valid right now. */
  if (RFWwrite_chunk(handle, opavih, sizeof(MainAVIHeader), (unsigned char *)header)!=
      sizeof(MainAVIHeader)+8) {
#ifdef VERBOSE
    fprintf(stderr,"create_avi_header() : writing the MainAVIHeader failed!?!? \n");
    fprintf(stderr,"create_avi_header() : trying to continue...\n");
#endif
  }
  
  /* Now the stream list must follow. This can be done either manually,
     or with the create_stream functions. */
  
  return(output);

}


/**********************************************************************/
/* Closes the hdrl list. Also writes a JUNK chunk of padding*/
/* until the nearest 4k boundary (4k alignment, for fastest access)   */
/* You must rewrite (update) the header when closing the file */
/* JUNK padding is by default modulo 4k. This should give some 1-2k   */
/* JUNK on most files. You may specify a minimum junk size if you plan*/
/* to add streams later. (currently not supported!) */
/* ALLOCATES MEMORY, THAT IS LATER FREED. */
/* 0 on success, -1 on failure */
int
AVWclose_avi_header(avi_handle *handle, uint32_t min_junk_size)
{
  long file_position;
  int junk_bytes;
  unsigned char *pad;


#ifdef CHECKS
  if (handle==0){
    fprintf(stderr,"%s : close_avi_header() : line %d : input handle is NULL\n",
	    __FILE__, __LINE__);
    return(-1);
  }
#endif

  /* Close the hdrl list */
  if (RFWclose_list(handle->riff_file)==0){
#ifdef VERBOSE
    fprintf(stderr,"close_avi_header() : cannot close the hdrl list!\n");
#endif
  }
  
  file_position=ftell(handle->riff_file->output);
  
  /* This is a bit tricky. We cannot write less
     than 10 bytes and we don't want to write more
     than 4096. We also want to ensure that more than
     min_junk_size get written and stay aligned to 4k.
  */

  junk_bytes=4096-((int)file_position&4095);

  /* We can't write less than 10 bytes, 8 for
   the header, a single junk byte and 1 byte alignment */
  if (junk_bytes<10) junk_bytes+=4096; 

  junk_bytes-=8; /* minus 8 for "JUNK" and 4-byte chunk size */

  if (min_junk_size==0){
    /* No junk needed, data are already aligned. */
    if (junk_bytes==4088) junk_bytes=0;
  } else {
    /* Add another 4k chunk to ensure min_junk_size */
    if (junk_bytes<min_junk_size) junk_bytes+=4096;
  }
    
#ifdef VERBOSE
  fprintf(stderr,"close_avi_header() : %ld file position, need %d junk bytes\n",
	  file_position, junk_bytes);
#endif


  /* No need to continue if we don't need any JUNK. 
     It's optional, after all. Return succesfully.
  */  
  if (junk_bytes==0) return(0);

#ifdef EXTENDED_CHECKS
  if (junk_bytes>1048576) {
    fprintf(stderr,"close_avi_header() : need an extreme amount of JUNK!\n");
    fprintf(stderr,"close_avi_header() : %d min junk size, %d junk required!\n",
	    min_junk_size, junk_bytes);
    return(-1); 
  }
#endif

  pad=(unsigned char *)malloc(junk_bytes+32);

#ifdef CHECKS
  if (pad==NULL) {
    fprintf(stderr,"close_avi_header() : cannot allocate memory!\n");
    return(-1);
  }
#endif

  memset(pad, 0x0, junk_bytes);

  if (RFWwrite_chunk(handle->riff_file, opJUNK, junk_bytes, pad)!=junk_bytes+8){
#ifdef VERBOSE
    fprintf(stderr,"close_avi_header() : write_chunk() seems to have failed!\n");
    fprintf(stderr,"close_avi_header() : trying to continue!\n");
#endif
  }
  
  free(pad);

  return(0);

}

/*****************************************************************/
/* Updates the avi header after the file has been completed.     */
/* What it does is write the number of frames etc. This is       */
/* necessary to create a valid .avi file. */
/* NOTE : update_avi_header uses the handle->avi_header          */
/* This should be maintained internally by the library. */
/* Returns 0 on success, <0 on failure. */
int 
AVWupdate_avi_header(avi_handle *handle)
{

  /* Write the header */
  if (RFWrewrite_chunk(handle->riff_file, opavih, 
		    sizeof(MainAVIHeader), 
		    (unsigned char *)&handle->avi_header,
		    24L)!=sizeof(MainAVIHeader)+8) {
#ifdef VERBOSE
    fprintf(stderr,"update_avi_header() : writing the MainAVIHeader failed!?!? \n");
    fprintf(stderr,"update_avi_header() : trying to continue...(return value is OK=0)\n");
#endif
  }

  return(0);
}


/*****************************************************************/
/* Keeps idx information by storing it in the queue. */
/* Returns <0 on various failures! */
int
AVWadd_idx(avi_handle *handle, AVIINDEXENTRY *index)
{

#ifdef CHECKS
  if (handle==0) {
    fprintf(stderr,"%s:%d: %s: avi handle is NULL!\n",
	    __FILE__, __LINE__, __FUNCTION__);
    return(-10);
  }

  if (handle->idx_list==0) {
    fprintf(stderr,"%s:%d: %s: queue handle is NULL!\n",
	    __FILE__, __LINE__, __FUNCTION__);
    return(-11);
  }
    
#endif

  return(QDSqadd(handle->idx_list, (void *)index, sizeof(AVIINDEXENTRY), index->lckid));
}



/****************************************************************/
/* This is called when no more frames are going to be written.
   This function writes the idx1 chunk to the file, closes the
   movi list and effectively finalises it.

   Returns number of indices written. <0 on various failures.
*/
int
AVWwrite_index_chunk(avi_handle *handle)
{
  int size;
  int i;
  int bytesum=0;
  AVIINDEXENTRY tmp;

  size=handle->idx_list->queue_size;

#ifdef CHECKS
  if (size==0) return(0);

  if (handle==0) {
    fprintf(stderr,"%s:%d: %s: avi handle is NULL!\n",
	    __FILE__, __LINE__, __FUNCTION__);
    return(-1);
  }

  if (handle->idx_list==0) {
    fprintf(stderr,"%s:%d: %s: queue handle is NULL!\n",
	    __FILE__, __LINE__, __FUNCTION__);
    return(-2);
  }

#endif

  i=QDSqseek(handle->idx_list,0);
  
#ifdef CHECKS
  if (i!=0){
    if (handle->idx_list==0) {
      fprintf(stderr,"%s:%d: %s: qseek() failed!\n",
	      __FILE__, __LINE__, __FUNCTION__);
      return(i);
    }
  }
#endif
    
  i=RFWwrite_chunk_header(handle->riff_file, opidx1, size*sizeof(AVIINDEXENTRY));

#ifdef CHECKS
  if (i!=8) fprintf(stderr,"%s:%d: %s: Error! idx1 chunk header is propably broken!\n",
		    __FILE__, __LINE__, __FUNCTION__);
#endif

  bytesum=i;

  for (i=0; i<size; i++){
    QDSqgetdata(handle->idx_list, &tmp);
    bytesum+=RFWwrite_raw_chunk(handle->riff_file, sizeof(AVIINDEXENTRY), (unsigned char *)&tmp);
    QDSqdel(handle->idx_list);
  }

#ifdef VERY_VERBOSE
  fprintf(stderr,"%s:%d: %s : Wrote %d bytes. \n",
	  __FILE__, __LINE__, __FUNCTION__, bytesum);
#endif

#ifdef CHECKS
  if (bytesum!=size*sizeof(AVIINDEXENTRY)+8){
    fprintf(stderr,"%s:%d: %s : Warning! : Wrote %d bytes, %d expected. \n",
	    __FILE__, __LINE__, __FUNCTION__, bytesum, 
	    size*sizeof(AVIINDEXENTRY)+8);
    return(-3);
  }
#endif

  return(0);

}
