
/******* EXAMPLE 2 **********/
/* libavi usage example */
/* Call with "example2 <avi filename>" */

#include <stdio.h>
#include <stdlib.h>
#include "aviread.h"


int
main(int argc, char *argv[])
{
  avi_handle *avifile;
  int i;
  int frame_size;
  uint32_t frame_type;

  /* a 128KB buffer */
  char bigbuffer[131000];
  

  /* Open the file that is passed as argument #1 */
  avifile=AVRopen_avi_file(argv[argc-1]);
  
  /* Print information regarding the main avi header */ 
  AVRprint_header_info(avifile, stdout);
  
  /* Print information for all streams */
  for (i=0; i<avifile->total_streams; i++) {
    AVRprint_stream_info(avifile, stdout, i);
  }

  /* Get three frames... If size 0 then either the
   chunk is indeed 0 bytes long or the file pointer
   is not positioned on a chunk belonging to 
   that stream. This may happen under some conditions
   but it will not disrupt the library. The seeking
   functions should be able to re-position the stream
   properly.
  */
  frame_size=AVRget_frame_idx(avifile, 0, bigbuffer, &frame_type);
  AVRnext_frame_idx(avifile, 0);
  printf("First frame from stream 0. Size %d, type [%s]\n", 
	 frame_size, _RFWfcc2s(frame_type));

  frame_size=AVRget_frame_idx(avifile, 0, bigbuffer, &frame_type);
  AVRnext_frame_idx(avifile, 0);
  printf("Second frame from stream 0. Size %d, type [%s]\n", 
	 frame_size, _RFWfcc2s(frame_type));

  frame_size=AVRget_frame_idx(avifile, 0, bigbuffer, &frame_type);
  AVRnext_frame_idx(avifile, 0);
  printf("Third frame from stream 0. Size %d, type [%s]\n", 
	 frame_size, _RFWfcc2s(frame_type));

  /* Skip 2 frames */
  AVRnext_frame_idx(avifile, 0);
  AVRnext_frame_idx(avifile, 0);

  /* Read from the other stream */
  frame_size=AVRget_frame_idx(avifile, 1, bigbuffer, &frame_type);
  AVRnext_frame_idx(avifile, 1);
  printf("Got the first frame from stream 1. Size is %d.\n", 
	 frame_size);

  /* We now choose to ignore frame type information 
     (this is OK, if you now what sort of frame you 
     are going to get). Simply pass a NULL argument 
     and AVRget_frame() will ignore it. */
  frame_size=AVRget_frame_idx(avifile, 1, bigbuffer, NULL);
  AVRnext_frame_idx(avifile, 1);
  printf("Got the second frame from stream 1. Size is %d.\n",
	 frame_size);

  /* Go back 1 frame in stream 0 */
  AVRprevious_frame_idx(avifile, 0);
  
  /* That's all */
  
  /* Close the file and free memory */
  AVRclose_avi_file(avifile);
  return(0);
}
