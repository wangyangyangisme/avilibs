
/******* EXAMPLE 1 **********/
/* libavi usage example */

#include <stdio.h>
#include <stdlib.h>
#include "aviread.h"


int
main(int argc, char *argv[])
{
  avi_handle *avifile;
  int i;
  
  avifile=AVRopen_avi_file(argv[argc-1]);
  
  /* Print information regarding the main avi header */ 
  AVRprint_header_info(avifile, stdout);
  
  /* Print information for all streams */
  for (i=0; i<avifile->total_streams; i++) {
    AVRprint_stream_info(avifile, stdout, i);
  }

  AVRclose_avi_file(avifile);
  return(0);
}
