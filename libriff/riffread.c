/*
 * Implementation of basic riff reading routines 
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "defines.h"
#include "riffcodes.h"
#include "sanity.h"
#include "riffread.h"

/* NOTE: RFR prefix stands for "riff read" functions. This is 
   here to remind you that you should not use an RFR function
   when writing. (RFW prefix) */ 

/***************************************************************/
/* Skip the next chunk. Please note that this WILL NOT skip
   a list. If a list is found, it will return 1. If it
   skips a chunk succesfully it returns 0 and finally, <0
   on various failures. On end of file, it returns 2 */
EXPORT int
RFRskip_chunk(riff_handle *input)
{
  SCHK(input!=0, ACT_ERROR);

  /* We don't operate on lists */
  if (input->block_type==opLIST) return(1);

  fseek(input->input, input->block_size, SEEK_CUR);
  input->file_position+=input->block_size;

  XCHK(input->file_position==ftell(input->input),ACT_ERROR);

  /* Word alignment is implicit */
  if (input->file_position&0x1) {
    fgetc(input->input);
    input->file_position++;
  }

  /* We can safely assume that we are at the start
     of a new block (chunk or list) */
  if (_RFRget_next_info(input)<0)
    return(-1);

  return(0);
}


/********************************************************************/
/* Skip the next list. Please note that this WILL NOT skip 
   a chunk, if that is the next item. You should know whether
   the next item is a chunk or a list by using RFRget_next_info.
   Returns 1 if a chunk is found, 0 on success, <0 on various 
   failures. Returns 2 on end of file. 
*/
EXPORT int
RFRskip_list(riff_handle *input)
{
  int retv;

  SCHK(input!=0, ACT_ERROR);

  if (input->block_type!=opLIST) return(1);

  /* Is this a phoney block that signifies list end? */
  if (input->block_subtype==opLIST) {
    retv=_RFRget_next_info(input);
    if (retv<0) return(-1);
    return(0);
  }
   
  retv=fseek(input->input, input->block_size-4, SEEK_CUR);
  
  /* Did fseek fail? */
  if (retv!=0) return(-1); 

  input->file_position+=input->block_size-4;

  XCHK(input->file_position==ftell(input->input),ACT_ERROR);

  /* Word alignment is implicit */
  if (input->file_position&0x1) {
    fgetc(input->input);
    input->file_position++;
  }

  /* We can safely assume that we are at the start
     of a new block (chunk or list) */
  retv=_RFRget_next_info(input);
  if (retv<0) return(-1);

  return(0);
}


/****************************************************************************/
/* Get the chunk data. 
   CAUTION: the buffer MUST BE ABLE TO CONTAIN AN AMOUNT OF DATA EQUAL TO
   THE CHUNK SIZE. NO CHECKING IS DONE. IF IT CRASHES, YOU ARE THE ONE TO
   BLAME! 
   Returns the number of bytes read. 
*/
EXPORT int
RFRget_chunk(riff_handle *input, unsigned char *buffer)
{
  int i, chunk_size;

  SCHK(input!=0, ACT_ERROR);
  SCHK(buffer!=0, ACT_ERROR);

  /* We cannot read a list! */
  if (input->block_type==opLIST) return(0);

  chunk_size=input->block_size;

#ifdef VERBOSE
  /* Some applications may produce avi files with chunks of length zero. What is the
     use of that? It does not prevent libriff from working. */
  if (chunk_size==0)
    fprintf(stderr,"%s:%d %s(): Warning! Found a zero length chunk in file %s. Ignoring it.\n",
	    __FILE__, __LINE__, __FUNCTION__, input->filename);
#endif

#ifdef VERY_VERBOSE
  fprintf(stderr,"At position %ld. Getting chunk of size %d, type %s.\n",
	  input->file_position, input->block_size,_RFWfcc2s(input->block_type));
#endif 

  i=fread(buffer, input->block_size, 1, input->input);

  SCHK(i*input->block_size==input->block_size, ACT_FAIL);

  input->file_position+=chunk_size*i;

  /* Word alignment */
  if ((input->file_position&0x1)!=0) {
    fgetc(input->input);
    input->file_position++;
  }

  _RFRget_next_info(input);

  return(i*chunk_size);

}
  
/*******************************************************************/
/* Enter the list, that is go to the first item in that list. */
/* Returns 0 on success, -1 on various failures. */
EXPORT int
RFRenter_list(riff_handle *input)
{
  SCHK(input!=0, ACT_ERROR);
  XCHK(input->list_count!=MAX_LISTS-1, ACT_ERROR);
  
  /* You cannot enter this type of
     list! This is an imaginary list block
     that only serves to remind you that
     you are leaving a list.
     Fail gracefully. */

  if (input->block_subtype==opLIST){
#ifdef VERY_VERBOSE
    fprintf(stderr,"W: Trying to enter a list_end block.\n");
#endif
    _RFRget_next_info(input);
    return(0);
  }
  
  input->list_count++;
  
  input->list_start[input->list_count]=input->file_position-12;
  input->list_end[input->list_count]=input->file_position+input->block_size-4;
  input->list_type[input->list_count]=input->block_subtype;

#ifdef VERY_VERBOSE
  fprintf(stderr,"Entering list of type %s, size %d, range [%ld-%ld].\n",
	  _RFWfcc2s(input->block_subtype),input->block_size,
	  input->file_position-12,
	  input->file_position+input->block_size-4);
#endif

  /* Grab the next block (that resides IN the list) */
  _RFRget_next_info(input);
  
  return(0);
}


/********************************************************************/
/* Exit the list, that is go to the START OF THE LIST. Not 
   AFTER the list.

   NOTE: a special "list_end" block is NOT generated!
   You are supposed to KNOW that you are leaving the current list!
*/
EXPORT int
RFRgoto_list_start(riff_handle *handle)
{
  SCHK(handle!=0, ACT_ERROR);
  SCHK(handle->list_count!=0, ACT_ERROR);

  fseek(handle->input, handle->list_start[handle->list_count], SEEK_SET);

  handle->list_count--;

  _RFRget_next_info(handle);

  return(0);

}

/*********************************************************************/
/* Exit the list, that is go to the END OF THE LIST, positioning
   the file pointer at the first item AFTER the list.

   NOTE: a special "list_end" block is NOT generated!
   You are supposed to KNOW that you are leaving the current list!
*/
EXPORT int
RFRgoto_list_end(riff_handle *handle)
{
  SCHK(handle!=0, ACT_ERROR);
  SCHK(handle->list_count!=0, ACT_ERROR);

  fseek(handle->input, handle->list_end[handle->list_count], SEEK_SET);

  handle->list_count--;

  _RFRget_next_info(handle);

  return(0);

}


/***************************************************************************/
/* Exit the list. Stay where you are. This is potentially dangerous. 
   It should be used when you have reached the end of the list. It does
   not fseek() and therefore it is faster than RFRgoto_list_end() 

   PLEASE NOTE: Do NOT use unless you are at the end of a list or
   at least at a block start. */
EXPORT int
RFRexit_list(riff_handle *handle)
{
  SCHK(handle!=0, ACT_ERROR);
  SCHK(handle->list_count!=0, ACT_ERROR);
  
  handle->list_count--;

  _RFRget_next_info(handle);

  return(0);
}



/********************************************************************************/
/* Instructs the library to skip all JUNK chunks by default,
 ignoring them when skipping. An exception is RFRseek_forward_to_chunk_type(JUNK)
 that will seek until the next JUNK chunk. 
 1 means skip JUNK chunks, 0 means stop at them. */
EXPORT int
RFRskip_junk(riff_handle *input, int skip_junk)
{
  SCHK(input!=0, ACT_ERROR);

  input->skip_junk=skip_junk;

  return(input->skip_junk);
}


/*******************************************************************/
/* Will save current position in a "temporary holder" so that
   we can return to it immediately. The holder variable will only
   keep the last saved position. */
EXPORT int
RFRsave_position(riff_handle *input)
{
  SCHK(input!=0, ACT_ERROR);

  input->temporary_position=input->file_position;

  return(0);
}

/*******************************************************************/
/* Return to the saved position. */
EXPORT int
RFRrestore_position(riff_handle *input)
{
  SCHK(input!=0, ACT_ERROR);

  if (RFRseek_to_position(input, input->temporary_position)==0) return(0);
  else return(-1);

  return(0);

}

/********************************************************************/
/* Returns the start of the current block. This is what allows us to
 come back to the current block. Note that this is NOT the same as
 ftell().*/
EXPORT long
RFRget_position(riff_handle *input)
{
  
  SCHK(input!=0, ACT_ERROR);
  
  XCHK(input->file_position==ftell(input->input), ACT_ERROR);
  
  return(input->file_position);

}

/**********************************************************/
/* A simple wrapper function that tells us when we have reached
   the end of the file. */
EXPORT int
RFReof(riff_handle *input)
{
  SCHK(input!=0, ACT_ERROR);

  if (input->file_position>=input->file_size)
    return(1);
  
  return(0);
}

/****************************************************/
/* Returns to position 12L (this is where the avi file
   starts) and clears the list stack */
EXPORT int
RFRrewind(riff_handle *handle)
{
  SCHK(handle!=0, ACT_ERROR);

  fseek(handle->input, 12L, SEEK_SET);

  XCHK(ftell(handle->input)==12L, ACT_FAIL);

  handle->file_position=ftell(handle->input);
  handle->list_count=0;

  _RFRget_next_info(handle);

  return(0);

}
