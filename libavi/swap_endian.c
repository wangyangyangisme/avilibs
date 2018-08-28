

#include "swap_endian.h"

/* Accepts a pointer to uint32_t data. The data is unconditionally
   swapped from 1234 to 4321, changing endiannes. Size is in 32-bit 
   pieces. This is by default skipped if the machine is little-endian
   (i386 and friends).
   Proper interface is the SWAP_ENDIAN() macro (see swap_endian.h).
*/
uint32_t
*swap_endian_by4(uint32_t *input, uint32_t size)
{
  register int i;
  register int j;
  register int l;

  for (i=0; i<size; i++){
    j=input[i];
    l=((j&0xff)<<24);
    j>>=8;
    l|=((j&0xff)<<16);
    j>>=8;
    l|=((j&0xff)<<8);
    j>>=8;
    l|=(j);    
    input[i]=l;
  }

  return(input);
}


/* This is slow. It takes special care to 
   convert a BITMAPINFOHEADER (modified
   in place) to little-endian. */

BITMAPINFOHEADER *
swap_bmh(BITMAPINFOHEADER *input)
{

#ifdef CHECKS
  if (input==0) return(0);
#endif

#define __ATOMIC32(x) ((x) = INT_TO_LE_INT((x)))
#define __ATOMIC16(x) ((x) = WORD_TO_LE_WORD((x)))

  __ATOMIC32(input->lSize);
  __ATOMIC32(input->lWidth);
  __ATOMIC32(input->lHeight);
  __ATOMIC16(input->wPlanes);
  __ATOMIC16(input->wBitCount);
  __ATOMIC32(input->lCompression);
  __ATOMIC32(input->lSizeImage);
  __ATOMIC16(input->wXPelsPerMeter);
  __ATOMIC16(input->wYPelsPerMeter);
  __ATOMIC32(input->lClrUsed);
  __ATOMIC32(input->lClrImportant);

  return(input);
  
}


/*
int
main(void)
{
  int *k;
  int *l;
  int i;

  k=(int *)malloc(1048576*4);
  l=(int *)malloc(1048576*4);

  
  for (i=0; i<1000; i++)
    swap_endian(k,1048576);
  

  for (i=0; i<1000; i++)
    memcpy(k,l,1048576*4);

  return(0);
}

*/

