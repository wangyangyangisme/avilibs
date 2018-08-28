/*
 * Implementation of the functions that support frame
 * seeking/reading etc...
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
#include <stdlib.h>

#include "avicodes.h"
#include "avi_structures.h"
#include "riffread.h"
#include "sanity.h"
#include "aviread.h"
#include "avi_handle.h"
#include "defines.h"




/************************************************************/
/* A frame is a chunk of recognizable type (dc, wb, db) inside 
   the movi list (or inside a rec list, in the movi list).
   Unknown chunks are silently IGNORED by the following functions.
   Use the libriff functions to read them. */
/* Linear view is enabled if we have an index. */

/* Return values are size in bytes. <0 means failure */

/************************************************************/
/* This function proceeds forward, either in linear view
   (see the libriff documentation!) or in hierarchical view
   until it meets a chunk that belongs to the given stream.

   Input: avi_handle, the stream desired.
   Output: size of the chunk in bytes. 0 is VALID!!! -1 means failure
   Side effects: - In linear view once you move forward you may not go
                   back to the start of the list. It is a "forward-only"
		   situation. Normally this is not a problem because
		   the index allows us to fseek().
		 - File position is changed. The file pointer moves
		   to the start of the next frame.
		 - This function will automatically update the index
		   pointers IF an index exists so that
		   AVRnext_frame_idx() will not be before AVRnext_frame()
		   (it may be after AVRnext_frame, this is normal)
		   
   NOTE: if MUST_USE_INDEX flag is set, this function will warn you.
*/
int 
AVRnext_frame(avi_handle *input, int stream)
{
  uint32_t scc;
  int retv;
  uint32_t scale, size, nBlock, rate;

  SCHK(input!=NULL, ACT_ERROR);
  SCHK((((uint32_t)stream<input->total_streams)&&(stream>=0)), ACT_ERROR);
  SCHK(input->stream_file[stream]!=NULL, ACT_FAIL);

  /* If the header says we must use the index, you should not
     be calling this function! */
  if ((input->avi_header.ulFlags&AVIF_MUSTUSEINDEX)!=0){
    fprintf(stderr,"%s:%d %s(): WARNING! AVIF_MUSTUSEINDEX=1, \nyou shouldn't be reading with this function.\n",
	    __FILE__, __LINE__, __FUNCTION__);
  }

  scc = _AVRn2fcc(stream);

#ifdef VERY_VERBOSE
  /* We have to make sure the codes are right... */
  fprintf(stderr,"%s:%d %s(): Trying with code %s, from %d (should be %d)\n",
	  __FILE__, __LINE__, __FUNCTION__, _RFWfcc2s(scc), _AVRfcc2n(scc), stream);
#endif

  /* We don't use the index.
     This may take some time. In most cases it will not. */
  do {     
    /* What happens if we don't find any more chunks belonging to that stream? */
    if (RFReof(input->stream_file[stream])==1){
#ifdef VERY_VERBOSE
      fprintf(stderr,"%s:%d %s(): No more chunks for stream %d in the file! \n",
	      __FILE__, __LINE__, __FUNCTION__, i);
#endif
      return(-1);
    }
    
    if (input->stream_file[stream]->block_type==opLIST) {      
      retv=RFRenter_list(input->stream_file[stream]);
      if (retv<0) {
	fprintf(stderr,"%s:%d %s(): Cannot skip list. Corrupt file?\n",
		__FILE__, __LINE__, __FUNCTION__);
	return(-1);
      }
    } else {
      retv=RFRskip_chunk(input->stream_file[stream]);       
      if (retv<0) {
	fprintf(stderr,"%s:%d %s(): Cannot skip chunk. Premature end of file?\n",
		__FILE__, __LINE__, __FUNCTION__);
	return(-1);
      }
    }

  } while ((input->stream_file[stream]->block_type==opLIST) ||
	   ((input->stream_file[stream]->block_type&0xffff)!=scc));


#if 0
  /* Increase our time counter 
     NOTE: I'm not sure how to time video & audio streams right now. */
  if (input->stream_header[stream]->fccType==opvids) {
    input->stream_time[stream]+=input->stream_header[stream]->ulScale/
      (double)input->stream_header[stream]->ulRate;
  } else {
    scale = input->stream_header[stream]->ulScale;
    rate = input->stream_header[stream]->ulRate;
    nBlock = ((WAVEFORMATEX *)input->stream_format[stream])->wBlockAlign;
    size = input->stream_file[stream]->block_size;

    fprintf(stderr, "scale %d, rate %d, nBlock %d, size %d.\n",
	    scale, rate, nBlock, size);

    /* Let's hope this actually works... */
    input->stream_time[stream]+= (double)rate/scale*size;
  }
  
  /* Verbose reporting! */
  fprintf(stderr,"[%2d]: position is %f sec. at %lu.\n", 
	  stream, input->stream_time[stream], input->stream_file[stream]->file_position);
  
#endif 

  return(input->stream_file[stream]->block_size);
}



/*************************************************************/
/* Same as above, but get the next frame as reported from the
   index and not the next one in the file. This may skip some frames
   or may change the order of frames. (esp. note the MUST_USE_INDEX
   flag!!!).

   Input: avi_handle, the stream desired
   Output: size of the chunk in bytes, -1 means failure
   Side effects: - The file position changes, automatically
                   keeps synchronization with AVRnext_frame().
   Prerequisites: - Linear view!!!
                  - Index structure

   Note that if we don't have an index structure this will not work.
*/		  		  
int
AVRnext_frame_idx(avi_handle *input, int stream)
{

  uint32_t scc;
  AVIINDEXENTRY *idx;
  int *strp;
  uint32_t idxsize;
  int trustck;
  long movi_offset;

  SCHK(input!=NULL, ACT_ERROR);
  SCHK(input->idx_array!=NULL, ACT_ERROR);
  SCHK(input->idx_array_size!=0, ACT_ERROR);
  SCHK((((uint32_t)stream<input->total_streams)&&(stream>=0)), ACT_ERROR);
  SCHK(input->stream_file[stream]->linear_view==1, ACT_WARN);

  scc = _AVRn2fcc(stream);

  /* Local copy of the pointer to idx array */
  idx = input->idx_array;
  
  /* Local copy of idxsize */
  idxsize = input->idx_array_size;

  /* Pointer to input->stream_idxp[stream] (counter for the index) */
  strp = &input->stream_idxp[stream];
  XCHK((*strp)>=0, ACT_FAIL);
  XCHK((*strp)<=idxsize, ACT_WARN);
  
  /* Can we trust the index chunk type? */
  trustck = (int)((input->avi_header.ulFlags&AVIF_TRUSTCKTYPE)!=0);

  /* What is the offset of the movi list? */
  movi_offset = input->movi_offset;

  if ((*strp)==idxsize) return(-1);

  /* Go to the next frame */
  (*strp)++;

  while ((*strp)<=idxsize) {
    
    if ((uint32_t)(*strp)==idxsize) {
#ifdef VERY_VERBOSE
      fprintf(stderr,"%s:%d %s(): No more chunks for stream %d in the index! \n",
	      __FILE__, __LINE__, __FUNCTION__, stream);
#endif
      return(-1);
    }

    /* TRUST CK TYPE? */
    if (trustck!=0) {
      if ((scc==(idx[*strp].lckid&0xffff)) && (idx[*strp].lckid!=opLIST)){
	/* The current frame in the index belongs to this stream... Find it. */	
	/* We seek relative to the start of movi list and subtract 4 for the
	   chunk header (fourcc) */
	(void)_RFRraw_seek(input->stream_file[stream], 
		     (long int)idx[*strp].lChunkOffset+movi_offset-4);

	return(input->stream_file[stream]->block_size);
      }
    } else {
      /* We do not trust the index type. Seek in the file (SLOWER) */
      (void)_RFRraw_seek(input->stream_file[stream],
		   (long int)idx[*strp].lChunkOffset+movi_offset-4);

      if ((scc==(input->stream_file[stream]->block_type&0xffff))&&
	  input->stream_file[stream]->block_type!=opLIST) {
	
	SCHK(input->stream_file[stream]->block_size==idx[*strp].lChunkLength, ACT_FAIL);
	
	return(input->stream_file[stream]->block_size);
      }
    }
    
    (*strp)++;

  };
  
  return(-1);
   
}


/*************************************************************/
/* We can only go backwards if we have an index. Get the previous
   frame, as reported by the index.

   I/O: see previous functions
   Side effects: - The file position changes
   
   As with all the _idx functions you have to enable linear
   view.
*/
int
AVRprevious_frame_idx(avi_handle *input, int stream)
{
  uint32_t scc;
  AVIINDEXENTRY *idx;
  int *strp;
  uint32_t idxsize;
  int trustck;
  long movi_offset;

  SCHK(input!=NULL, ACT_ERROR);
  SCHK(input->idx_array!=NULL, ACT_ERROR);
  SCHK(input->idx_array_size!=0, ACT_ERROR);
  SCHK((((uint32_t)stream<input->total_streams)&&(stream>=0)), ACT_ERROR);
  SCHK(input->stream_file[stream]->linear_view==1, ACT_WARN);

  scc = _AVRn2fcc(stream);

  /* Local copy of the pointer to idx array */
  idx = input->idx_array;

  /* Local copy of idxsize */
  idxsize = input->idx_array_size;

  /* Pointer to input->stream_idxp[stream] (counter for the index) */
  strp = &input->stream_idxp[stream];
  
  /* Can we trust the index type? */
  trustck = (int)((input->avi_header.ulFlags&AVIF_TRUSTCKTYPE)!=0);

  /* What is the offset of the movi list? */
  movi_offset = input->movi_offset;


  if ((*strp)<=0) {
    return(-1);
  } else (*strp)--;

  while ((*strp)>=0) {
    
    /* TRUST CK TYPE? */
    if (trustck!=0) {
      if ((scc==(idx[*strp].lckid&0xffff)) && (idx[*strp].lckid!=opLIST)){
	/* The current frame in the index belongs to this stream... Find it. */	
	/* We seek relative to the start of movi list and subtract 4 for the
	   chunk header (fourcc) */
	(void)_RFRraw_seek(input->stream_file[stream], 
		     (long int)idx[*strp].lChunkOffset+movi_offset-4);
	return(input->stream_file[stream]->block_size);
      }
    } else {
      /* We do not trust the index type. Seek in the file (SLOWER) */
      (void)_RFRraw_seek(input->stream_file[stream],
		   (long int)idx[*strp].lChunkOffset+movi_offset-4);

      if ((scc==(input->stream_file[stream]->block_type&0xffff))&&
	  input->stream_file[stream]->block_type!=opLIST) {
	
	SCHK(input->stream_file[stream]->block_size==idx[*strp].lChunkLength, ACT_FAIL);
	
	/* This is the frame we want. */	
	return(input->stream_file[stream]->block_size);
      }
    }
    
    if ((*strp)==0)
      return(-1);

    (*strp)--;

  };
  
  return(-1);
   
}



/*************************************************************/
/* This is the slow version. It is only used if we DO NOT 
   have linear view+index. Try to avoid this. Intra-frame
   may not be decodable (you should seek to keyframes).
*/
int
AVRseek_to_frame(avi_handle *input, int stream, uint32_t frame);


/************************************************************/
/* Jump to frame "frame" as found in the index. Note that
   this may be different than frame "frame"  in the file 
   because the index is not always complete. In this case
   we trust the index.
*/
int
AVRseek_to_frame_idx(avi_handle *input, int stream, uint32_t frame);



/************************************************************/
/* Get the current frame!!!!!!  If the current chunk does
   NOT belong to the requested stream the function will
   NOT read it. You will have to call AVRnext_frame
   of AVRnext_frame_idx to go to the next eligible frame.

   To put it simply this function will get the current
   chunk IF it belongs to the stream we want.

   Input: avi_handle, stream number, unsigned char buffer for the
          data (MUST BE ABLE TO CONTAIN THE FULL FRAME!!!!!)
	  and pointer to variable where the frame type (fcc)
	  will be stored. If you don't want this you may use
	  NULL and frame_type will be ignored.
   Output: number of bytes read, -1 on various failures, 0 if the
           current chunk does not belong to the stream (please note
           that you may also get 0 if the size of the chunk is
	   0)
   Side effects: File pointer advances to the next block,
                 buffer gets written, frame_type gets written
		 The index pointer does NOT change.


*/
int
AVRget_frame_idx(avi_handle *input, int stream, unsigned char *buffer, 
	     uint32_t *frame_type)
{
  uint32_t scc;
  int retv;
  
  /* Make sure the caller is sane */
  SCHK(input!=NULL, ACT_ERROR);
  SCHK(stream<input->total_streams, ACT_ERROR);
  SCHK(stream>=0, ACT_ERROR);
  SCHK(buffer!=NULL, ACT_ERROR);

  /* Make sure the environment is sane */
  SCHK(input->stream_file[stream]!=NULL, ACT_ERROR);

  scc = _AVRn2fcc(stream);

  /*
  fprintf(stderr,"scc = %s\n", _RFWfcc2s(scc));
  fprintf(stderr,"block type = %s\n", _RFWfcc2s(input->stream_file[stream]->block_type));
  */  

  if (input->stream_idxp[stream]==input->idx_array_size)
    return(-1);

  /* This should not happen */
  if (input->stream_idxp[stream]<0){
    input->stream_idxp[stream]=0;
    return(-1);
  }

  /* Synchronise the file with the index */
  _RFRraw_seek(input->stream_file[stream], input->idx_array[input->stream_idxp[stream]].lChunkOffset+
	       input->movi_offset-4);

  /* AVRget_frame always trusts the file, NOT the index, so if
     the file and the index get out of sync you must check
     the real chunk type as reported from the file */

  if (((input->stream_file[stream]->block_type & 0xffff) == scc) &&
      (input->stream_file[stream]->block_type != opLIST)) {

    if (frame_type!=NULL){
      *frame_type=input->stream_file[stream]->block_type;
#ifdef VERY_VERBOSE
      fprintf(stderr,"%s:%d %s(): Going to read frame of type %s\n",
	      __FILE__, __LINE__, __FUNCTION__, _RFWfcc2s(*frame_type));
#endif
    }
    
    retv=RFRget_chunk(input->stream_file[stream], buffer);
    
    if (retv<0) {
      fprintf(stderr,"%s:%d %s(): WARNING! RFRget_chunk() reported failure. Return value %d.\n",
	      __FILE__, __LINE__, __FUNCTION__, retv);
    } 
      
    return(retv);
    
  } 

  if (frame_type!=NULL)
    *frame_type=0;

  return(0);

}



/************************************************************/
/* Get the current frame!!!!!!  If the current chunk does
   NOT belong to the requested stream the function will
   NOT read it. You will have to call AVRnext_frame
   of AVRnext_frame_idx to go to the next eligible frame.

   To put it simply this function will get the current
   chunk IF it belongs to the stream we want.

   THIS FUNCTION DOES NOT USE THE INDEX.

   Input: avi_handle, stream number, unsigned char buffer for the
          data (MUST BE ABLE TO CONTAIN THE FULL FRAME!!!!!)
	  and pointer to variable where the frame type (fcc)
	  will be stored. If you don't want this you may use
	  NULL and frame_type will be ignored.
   Output: number of bytes read, -1 on various failures, 0 if the
           current chunk does not belong to the stream (please note
           that you may also get 0 if the size of the chunk is
	   0)
   Side effects: File pointer advances to the next block, 
                 buffer gets written, frame_type gets written


*/
int
AVRget_frame(avi_handle *input, int stream, unsigned char *buffer, 
	     uint32_t *frame_type)
{
  uint32_t scc;
  int retv;
  
  /* Make sure the caller is sane */
  SCHK(input!=NULL, ACT_ERROR);
  SCHK(stream<input->total_streams, ACT_ERROR);
  SCHK(stream>=0, ACT_ERROR);
  SCHK(buffer!=NULL, ACT_ERROR);

  /* Make sure the environment is sane */
  SCHK(input->stream_file[stream]!=NULL, ACT_ERROR);

  scc = _AVRn2fcc(stream);

  /* AVRget_frame always trusts the file, NOT the index, so if
     the file and the index get out of sync you must check
     the real chunk type as reported from the file */

  if (((input->stream_file[stream]->block_type & 0xffff) == scc) &&
      (input->stream_file[stream]->block_type != opLIST)) {

    if (frame_type!=NULL){
      *frame_type=input->stream_file[stream]->block_type;
#ifdef VERY_VERBOSE
      fprintf(stderr,"%s:%d %s(): Going to read frame of type %s\n",
	      __FILE__, __LINE__, __FUNCTION__, _RFWfcc2s(*frame_type));
#endif
    }
    
    retv=RFRget_chunk(input->stream_file[stream], buffer);
    
    if (retv<0) {
      fprintf(stderr,"%s:%d %s(): WARNING! RFRget_chunk() reported failure. Return value %d.\n",
	      __FILE__, __LINE__, __FUNCTION__, retv);
    } 
      
    return(retv);
    
  } 

  if (frame_type!=NULL)
    *frame_type=0;

  return(0);

}

//add intaoism 2012.10.8
int
AVRget_next_frame_size(avi_handle *input, int stream)
{
  uint32_t scc;
  int retv;
  
  /* Make sure the caller is sane */
  SCHK(input!=NULL, ACT_ERROR);
  SCHK(stream<input->total_streams, ACT_ERROR);
  SCHK(stream>=0, ACT_ERROR);

  /* Make sure the environment is sane */
  SCHK(input->stream_file[stream]!=NULL, ACT_ERROR);

  scc = _AVRn2fcc(stream);

  /* AVRget_frame always trusts the file, NOT the index, so if
     the file and the index get out of sync you must check
     the real chunk type as reported from the file */

  if (((input->stream_file[stream]->block_type & 0xffff) == scc) &&
      (input->stream_file[stream]->block_type != opLIST)) {
      
    retv = input->stream_file[stream]->block_size;
    return(retv);
  } 
}

