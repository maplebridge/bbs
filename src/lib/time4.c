/* ----------------------------------------------------------------------------- */
/* This file is porting from ptt bbs						 */
/* Origin file: https://opensvn.csie.org/pttbbs/trunk/pttbbs/common/sys/time.c	 */
/* ----------------------------------------------------------------------------- */


#include <time.h>
#include <stdio.h>
#include "dao.h"


#ifdef TIMET64

// XXX TODO change this to localtime_r style someday.
struct tm *
localtime4(t)
  const time4_t *t;
{
  if (t == NULL)
    return localtime(NULL);
  else
  {
    time_t temp = (time_t) *t;
    return localtime(&temp);
  }
}


time4_t
time4(ptr)
  time4_t *ptr;
{
  if (ptr == NULL)
    return time(NULL);
  else
    return *ptr = (time4_t) time(NULL);
}
#endif

