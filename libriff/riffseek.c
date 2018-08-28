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

/* Will seek forward, entering and exiting lists until
   this type of chunk is found. If it cannot be found, it
   will stay at the end of the file. Returns 1 if found,
   0 if not found and <0 on various failures. */
EXPORT int
RFRseek_forward_to_chunk_type(riff_handle *input, uint32_t type)
{

  SCHK(input!=0, ACT_ERROR);
  
  XCHK(input->file_position==ftell(input->input), ACT_FAIL);

  for (; input->file_position<input->file_size; ) {
    if (input->block_type==type) return(1);
    else if (input->block_type==opLIST){
      if (input->block_subtype==opLIST)
	RFRskip_list(input);
      else
	RFRenter_list(input);
    }
    else RFRskip_chunk(input);
  }

  return(0);

}
    
    

/* Will seek forward, entering and exiting lists until
   this type of list is found. */
/* Returns 1 if found, 0 if not found, <0 on various failures. */
EXPORT int
RFRseek_forward_to_list_type(riff_handle *input, uint32_t type)
{
  
  SCHK(input!=0, ACT_ERROR);

  XCHK(input->file_position==ftell(input->input), ACT_FAIL);

  for (; input->file_position<input->file_size; ) {
    if (input->block_type==opLIST){
      if (input->block_subtype==type) return(1);
      if (input->block_subtype==opLIST)
	RFRskip_list(input);
      else
	RFRenter_list(input);
    }
    else RFRskip_chunk(input);
  }

  return(0);

}


/*****************************************************************/
/* 
   Will search >ONLY< INSIDE THE CURRENT LIST for the requested
   chunk. This function will not enter or exit lists and is therefore
   safer than the previous functions.

   Returns 1 if found, 0 if not found and <0 on various failures.
*/

EXPORT int
RFRfind_chunk(riff_handle *handle, uint32_t type)
{
  long position;

  SCHK(handle!=0, ACT_ERROR);
  XCHK(handle->file_position==ftell(handle->input), ACT_FAIL);

  position=handle->file_position;
 
  for (; handle->file_position<handle->file_size; ) {
    if (handle->block_type==type) return(1);
    else if (handle->block_type==opLIST) return(0);
    RFRskip_chunk(handle);
  }

  fseek(handle->input, position, SEEK_SET);

  return(0);

}
  

/*****************************************************************/
/* This function will take you to the requested position in a 
   safe way, keeping the internal library structures valid. 
   This may be really slow, if you try to seek to a 

   Returns 0 on success, <0 on various failures. */
EXPORT int
RFRseek_to_position(riff_handle *input, long file_position)
{
  register int i;

  SCHK(input!=0, ACT_ERROR);
  SCHK(file_position<input->file_size, ACT_ERROR);
  SCHK(file_position>=12L, ACT_ERROR);

  for (i=input->list_count; i>0; i--)
    if (file_position<input->list_start[i]) RFRgoto_list_start(input);
    else if (file_position>input->list_end[i]) RFRgoto_list_end(input);
    else break;

  if (input->file_position>file_position) RFRrewind(input);

  /* What we want to do is traverse the file from our current position, that
     should be before the requested position to the requested position. This 
     should be relatively
     fast. We jump over everything, stopping at the chunk that CONTAINS the
     requested position. (this may not be exactly equal to that position!) */
  while (input->file_position<file_position){
#ifdef VERY_VERBOSE
    fprintf(stderr,"Current block info : %s, %d,",
	    _RFWfcc2s(input->block_type), input->block_size);
    fprintf(stderr,"%s\n",_RFWfcc2s(input->block_subtype));
#endif
    if (input->block_type==opLIST){
      if (input->block_subtype==opLIST) RFRskip_list(input);
      else {
	if (input->block_size+input->file_position<file_position)
	  RFRskip_list(input);
	else
	  RFRenter_list(input);
      }
    } else {
      /* It is a chunk */
      if (input->file_position+input->block_size<=file_position) RFRskip_chunk(input);
      
    }
  }
     
  return(0);

}   


/******************************************************************/
/* This function will take you at the >exact< specified point
   immediately. Use only if you know what you are doing. Make sure
   you don't cross list boundaries (unless if you are using
   "linear view".

   NOTE: you should move to a word aligned position for 
   _RFRget_next_info() to work. _RFRraw_seek() does not enforce
   word alignment but most functions expect the file to be
   word aligned and may fail or warn you.
   
*/
EXPORT int
_RFRraw_seek(riff_handle *input, long file_position)
{
  long test;

  SCHK(input!=0, ACT_ERROR);

  test=fseek(input->input, file_position, SEEK_SET);

  XCHK(test==0, ACT_ERROR);

  input->file_position=file_position;

  /* Be careful here. This may not work if
     you seek to an inappropriate place
     in the code! */
  test=_RFRget_next_info(input);
  
  XCHK(test>=0, ACT_WARN);

  return(0);

}
