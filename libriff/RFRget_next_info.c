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


/* This is the most frequently used function. */
/* Returns the next item's (chunk or LIST) size by default, <0 on various
   failures. 

   NOTE: RFRget_next_info advances the file_position.

   This function fills internal data structures (namely block_type,
   block_size and block_subtype)
 */
EXPORT int
_RFRget_next_info(riff_handle *input)
{
  int test; 

  SCHK(input!=0, ACT_ERROR);

  /* We should receive the file at word alignment! */
  XCHK((ftell(input->input)&0x1)!=1, ACT_FAIL);

  /* If we wish to enforce linear view we keep input->list_count constantly = 0 */
  if (input->linear_view==1) 
    input->list_count=0;

 top:
  /* If we have some open lists... */
  if (input->list_count>0)
    /* Check to see whether we are leaving any of them */
    if (input->file_position==input->list_end[input->list_count]){
#ifdef VERY_VERBOSE
      fprintf(stderr,"Creating a special exit block. %d,%d\n",input->list_exit,
	      input->list_count);
#endif
      input->block_type=opLIST;
      input->block_size=input->list_type[input->list_count];
      input->block_subtype=opLIST;
      input->list_count--;
      return(0);
    }
  

  /* We can't go after the end of the file! */
  if (input->file_position>=input->file_size)
    return(-1);

  /* Another way to make sure (for truncated files
     with incorrect header information) */
  if (feof(input->input)!=0){
    fprintf(stderr,"%s:%d %s(): Premature end of file reached!\n",
	    __FILE__, __LINE__, __FUNCTION__);
    return(-1);
  }

  test=fread(&(input->block_type), 4, 1, input->input);
  XCHK(test==1, ACT_ERROR);
  input->file_position+=test<<2;

  test=fread(&(input->block_size), 4, 1, input->input);
  XCHK(test==1, ACT_ERROR);
  input->file_position+=test<<2;

  if (input->block_type==opLIST){
    test=fread(&(input->block_subtype), 4, 1, input->input);
    XCHK(test==1, ACT_ERROR);
    input->file_position+=test<<2;
  }    

  /* Automatically skip junk chunks if it has been requested... */
  if (input->skip_junk==1)
    if (input->block_type==opJUNK){
      /* Skip the actual chunk */
      fseek(input->input, input->block_size, SEEK_CUR);
      input->file_position+=input->block_size;

      /* Align, if necessary */
      if ((input->block_size&0x1)==1) {
	fgetc(input->input);
	input->file_position++;
      }
      
      goto top;
    }

  return(input->block_size);

}
