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
#include <stdint.h>

#include "defines.h"
#include "riffcodes.h"
#include "sanity.h"
#include "riffread.h"


/* NOTE: RFR prefix stands for "riff read" functions. This is 
   here to remind you that you should not use an RFR function
   when writing. (RFW prefix) */ 


/* Returns the next item's (chunk or LIST) size by default, <0 on various
   failures. If type, size and list_type are null it will only return the
   next item's size without failing. */
EXPORT int
RFRget_block_info(riff_handle *input, 
	      uint32_t *type, 
	      uint32_t *size,
	      uint32_t *list_type) 
{
  SCHK(input!=0, ACT_ERROR);

  if (type!=0) *type=input->block_type;
  if (size!=0) *size=input->block_size;
  if (*type==opLIST) {
    if (list_type!=0) *list_type=input->block_subtype;
  } else {
    if (list_type!=0) *list_type=input->list_type[input->list_count];
#ifdef VERY_VERBOSE
    fprintf(stderr,"%s:%d: %s: Container list is %s.\n",
	    __FILE__, __LINE__, __FUNCTION__, _RFWfcc2s(*list_type));
#endif
  }
  return(input->block_size);

}







