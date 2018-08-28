/*
 * Implementation of the _AVRbuild_idx function that builds
 * an index from scratch. 
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



#include "avi_structures.h"
#include "riffread.h"
#include "sanity.h"
#include "aviread.h"


/************************************************************/
/* This function builds an index structure (in MEMORY ONLY)
   from the avi file itself. This function can be called if:
   a) an index structure is necessary for the calling application
   b) the index structure is corrupt or cannot be trusted (?!)
   
   NOTE: this is time consuming because it scans through the
   whole file. However, the function has not been specially
   optimized because the process is DISK bound and not
   CPU bound. A speedup is of course possible, but this
   is a special case function and not one that is
   frequently called. On my computer (700MHz Athlon),
   for a 500MB file CPU time is 6 sec and REAL time
   is 28. Even if we reduce CPU time to 2 sec with
   hardcore optimization we still get 24 sec total, so
   there is no point in doing this.

   Input : avi_handle
   Output : 0 on success, -1 on various errors
   Side effects : - The file pointer is not modified.
                  - Memory is allocated for the queue structure
		  AND the idx_array. The idx chunk is initially 
		  created in the queue structure and then copied
		  in the idx_array. NOTE that the queue structure
		  is NOT de-allocated automatically.
		  		  
 */
int
_AVRbuild_idx(avi_handle *input)
{
  int junk_status;
  uint32_t movi_list_offset;
  uint32_t movi_list_end;
  uint32_t scc;
  AVIINDEXENTRY index;
  int retv;
  register int list_size;
  register int i;

  SCHK(input!=NULL, ACT_ERROR);
  SCHK(input->riff_file!=NULL, ACT_FAIL);

  if (input->idx_array!=0) {
    if ((input->avi_header.ulFlags|AVIF_HASINDEX)!=0){
      fprintf(stderr,"%s:%d %s(): The file already has an index (also, AVIF_HASINDEX=1).\n",
	      __FILE__, __LINE__, __FUNCTION__);
      fprintf(stderr,"\tWe have an index structure (size=%d items, at %p). I will ignore it.\n",
	      input->idx_array_size, (void *)input->idx_array);
    } else {
      fprintf(stderr,"%s:%d %s(): The file is NOT supposed to carry an index (AVIF_HASINDEX=0).\n",
	      __FILE__, __LINE__, __FUNCTION__);
      fprintf(stderr,"\tHowever, we DO have an index structure in memory (size=%d items, at %p). I will ignore it.\n",
	      input->idx_array_size, (void *)input->idx_array);
	      
    }      

    /* If we don't free it we get a nice memory leak. */
      free(input->idx_array);
      input->idx_array_size=0;
  }
  
  if (input->riff_file->block_type!=opLIST) {
    fprintf(stderr,"%s:%d %s(): FAILURE! The file is not positioned at the",
	    __FILE__, __LINE__, __FUNCTION__);
    fprintf(stderr,"\t beggining of a list. Corrupt avi header (?)\n");
    exit(EXIT_FAILURE);
  }
  
  if (input->riff_file->block_subtype!=opmovi) {
    fprintf(stderr, "%s:%d %s(): FAILURE! The current list is not a movi list.\n", 
	    __FILE__, __LINE__, __FUNCTION__);
    fprintf(stderr, "%s:%d %s(): Got type %s instead. \n",
	    __FILE__, __LINE__, __FUNCTION__, _RFWfcc2s(input->riff_file->block_subtype));
    exit(EXIT_FAILURE);
  }

  /* Now we know we are at the beggining of the movi list and we may start
     scanning the chunks and making an index. Note that EVERY CHUNK AND LIST
     inside the movi list is indexed (expect for JUNK).
     Playback may not work from EVERY frame (not all are keyframes, and we
     don't know which are). */

  /* We use the master file pointer (not stream based) */
  junk_status=input->riff_file->skip_junk;
  
  /* Skip junk */
  RFRskip_junk(input->riff_file, 1);

  /* Offsets are recorded with respect to movi list start */
  movi_list_offset=input->riff_file->file_position;
  
#ifdef VERY_VERBOSE
  fprintf(stderr,"%s:%d %s(): Building index with respect to start of movi list at %u.\n",
	  __FILE__, __LINE__, __FUNCTION__, movi_list_offset);
#endif

  /* This is the end of the list */
  movi_list_end=movi_list_offset+input->riff_file->block_size;

  /* Create the queue */
  input->idx_list=QDScreate_queue();
  if (input->idx_list==NULL) {
    fprintf(stderr,"%s:%d %s(): Cannot build queue data structure! Malloc failed? \n",
	    __FILE__, __LINE__, __FUNCTION__);
    return(-1);
  }
  
  /* Note that we use a 0,0,'p','c' and NOT a '0','0','p','c' 
     fourcc. These are completely different. We store the fourcc
     in a temporary variable to avoid recomputing. */
  scc=_RFWs2fcc(0,0,'p','c');
  scc&=0xffff0000;

  /*  fprintf(stdout,"scc is %d (%s)\n", scc, _RFWfcc2s(scc>>16)); */

  /* Enter the movi list */
  RFRenter_list(input->riff_file);

  /* Stop at end of file, normally we may not get there.
     We stop at the end of the movi list, indicated by the
     list end block */
  while (RFReof(input->riff_file)!=1) {
    if (input->riff_file->block_type==opLIST){
      if (input->riff_file->block_subtype==opLIST){
	/* This is a list end block. Does it belong to 
	   the movi list? */

	/* Subtle hack, does work, but you may not like it.
	   End seeking if this is the list end block of the 
	   movi list. */
	if (input->riff_file->block_size==opmovi) 
	  break;

	/* Skip the list end block */
	RFRskip_list(input->riff_file);

      } else {
	/* This is a real list, add it... */
	index.lckid=input->riff_file->block_subtype;
	index.lChunkLength=input->riff_file->block_size;
	index.lChunkOffset=input->riff_file->file_position-movi_list_offset-4;
	index.lFlags=AVIIF_LIST;
	retv=QDSqadd(input->idx_list, (char *)&index, sizeof(AVIINDEXENTRY), 1);

	if (retv!=0) {
	  fprintf(stderr,"%s:%d %s(): QDSqadd() seems to have failed. I will ignore this. \n",
		  __FILE__, __LINE__, __FUNCTION__);
	}
	
	/* Enter the list */
	RFRenter_list(input->riff_file);

      }
    } else {  
      /* This is a chunk. We don't know if it is
	 a keyframe. We mark ##pc chunks as AVIIF_NOTIME.
	 All others get flags=0. Please help if you know
	 something about this. */
      if ((input->riff_file->block_type&0xffff0000)==scc)
	index.lFlags=AVIIF_NOTIME;
      else 
	index.lFlags=0;

      index.lckid=input->riff_file->block_type;
      index.lChunkLength=input->riff_file->block_size;
      index.lChunkOffset=input->riff_file->file_position-movi_list_offset-4;
      /* 2 is a "type" marker that the queue data structure offers, it does
	 not have anything to do with the avi file format, you may set this
	 to whatever you like. I chose 1 for lists, 2 for chunks. */

#ifdef VERY_VERBOSE
      fprintf(stdout,"Adding chunk %s at offset %u, size %u, flags %u.\n",
	      _RFWfcc2s(index.lckid), index.lChunkOffset, index.lChunkLength,
	      index.lFlags);
#endif

      retv=QDSqadd(input->idx_list, (char *)&index, sizeof(AVIINDEXENTRY), 2);

      if (retv!=0) {
	fprintf(stderr,"%s:%d %s(): QDSqadd() seems to have failed. I will ignore this. \n",
		__FILE__, __LINE__, __FUNCTION__);
      }
      
      RFRskip_chunk(input->riff_file);
    }  
    
#if 0
    /* A little redundancy never hurt... */
    if (input->riff_file->file_position>=movi_list_end)
      break;
#endif
      
  }

  /* Now we have the index in list form. We will copy it in the array. */
    retv=QDSqseek(input->idx_list,0);
  
#ifdef CHECKS
  if (retv!=0){
    if (input->idx_list==0) {
      fprintf(stderr,"%s:%d: %s(): QDSqseek() failed!\n",
              __FILE__, __LINE__, __FUNCTION__);
      return(-1);
    }
  }
#endif
    
  list_size=input->idx_list->queue_size;
  
  input->idx_array_size=list_size;
  input->idx_array=malloc(sizeof(AVIINDEXENTRY)*list_size);

  if (input->idx_array==NULL) {
    fprintf(stderr,"%s:%d %s(): Could not allocate %d bytes for the index! Out of memory?!\n",
	    __FILE__, __LINE__, __FUNCTION__, sizeof(AVIINDEXENTRY)*list_size);
    exit(EXIT_FAILURE);
  }


  for (i=0; i<list_size; i++){
    /* Copy data */
    retv=QDSqgetdata(input->idx_list, &input->idx_array[i]);
    if (retv!=0) {
      fprintf(stderr,"%s:%d %s(): QDSqgetdata(item %d of %d) failed! Trying to ignore this.\n",
	      __FILE__, __LINE__, __FUNCTION__, i, list_size);
    }
    QDSqadvance(input->idx_list);
  }

  /* NOTE: we DO NOT delete the queue structure! I'll add a seperate function for that.
     The reason is that the AVW functions may use it directly to write a copy of the
     file WITH an index (AVW functions use the idx_list by default). I'll think
     about that. Until then, you'll have to carry the extra list_size*sizeof(AVIINDEXENTRY)
     bytes in memory */

  /* Restore previous status */
  RFRskip_junk(input->riff_file, junk_status);
  RFRrewind(input->riff_file);

  /* This should not take too much time */
  RFRseek_forward_to_list_type(input->riff_file, opmovi);

  /* Now set the trust CK type flag, because we know what we are doing.
     (the index can be trusted). Note that this improves performance
     dramatically.  */
  input->avi_header.ulFlags|=AVIF_TRUSTCKTYPE;

  return(0);
}
