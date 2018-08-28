
#ifndef _SANITY_H
#define _SANITY_H

#include "defines.h"


/* Action can be one of : */
#define ACT_FAIL  0
#define ACT_ERROR 1
#define ACT_WARN  2


/* PSCHK has return value pointer to suppress warnings in functions returning a pointer. */

#ifdef CHECKS

#define SCHK(condition, action) if ((condition)==0) { \
switch(action) { \
 case ACT_FAIL : fprintf(stderr,"%s:%3d %s(): FAILURE! Assertion broken! \n",__FILE__,__LINE__,__FUNCTION__); exit(EXIT_FAILURE); \
 case ACT_ERROR: fprintf(stderr,"%s:%3d %s(): ERROR! Assertion broken! \n",__FILE__,__LINE__,__FUNCTION__); return(-1); \
 case ACT_WARN : fprintf(stderr,"%s:%3d %s(): WARNING! Assertion broken! \n",__FILE__,__LINE__,__FUNCTION__); break;\
 }}
#define PSCHK(condition, action) if ((condition)==0) { \
switch(action) { \
 case ACT_FAIL : fprintf(stderr,"%s:%3d %s(): FAILURE! Assertion broken! \n",__FILE__,__LINE__,__FUNCTION__); exit(EXIT_FAILURE); \
 case ACT_ERROR: fprintf(stderr,"%s:%3d %s(): ERROR! Assertion broken! \n",__FILE__,__LINE__,__FUNCTION__); return(NULL); \
 case ACT_WARN : fprintf(stderr,"%s:%3d %s(): WARNING! Assertion broken! \n",__FILE__,__LINE__,__FUNCTION__); break;\
 }}
#else

#define SCHK(condition, action) 
#define PSCHK(condition, action)

#endif /* CHECKS */

#ifdef EXTENDED_CHECKS
#define XCHK(condition, action) if ((condition)==0) { \
switch(action) { \
 case ACT_FAIL : fprintf(stderr,"%s:%3d %s(): FAILURE! Assertion broken! \n",__FILE__,__LINE__,__FUNCTION__); exit(EXIT_FAILURE) ; \
 case ACT_ERROR: fprintf(stderr,"%s:%3d %s(): ERROR! Assertion broken! \n",__FILE__,__LINE__,__FUNCTION__); return(-1) ; \
 case ACT_WARN : fprintf(stderr,"%s:%3d %s(): WARNING! Assertion broken! \n",__FILE__,__LINE__,__FUNCTION__); break;\
 }}
#define PXCHK(condition, action) if ((condition)==0) { \
switch(action) { \
 case ACT_FAIL : fprintf(stderr,"%s:%3d %s(): FAILURE! Assertion broken! \n",__FILE__,__LINE__,__FUNCTION__); exit(EXIT_FAILURE) ; \
 case ACT_ERROR: fprintf(stderr,"%s:%3d %s(): ERROR! Assertion broken! \n",__FILE__,__LINE__,__FUNCTION__); return(NULL) ; \
 case ACT_WARN : fprintf(stderr,"%s:%3d %s(): WARNING! Assertion broken! \n",__FILE__,__LINE__,__FUNCTION__); break;\
 }}
#else
#define XCHK(condition, action) 
#define PXCHK(condition, action)
#endif

#endif


