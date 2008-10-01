/*-------------------------------------------------------*/
/* lib/time4.h          ( NTHU CS MapleBBS Ver 3.00 )    */
/*-------------------------------------------------------*/
/* target : TIMET64 definition                           */
/* create : 08/09/30                                     */
/* update :   /  /                                       */
/*-------------------------------------------------------*/


#ifndef	_TIME4_H_
#define	_TIME4_H_


#include <sys/types.h>


#undef TIMET64

#ifdef TIMET64
typedef int32_t time4_t;
#else
typedef time_t time4_t;
#define time4(a)	time(a)
#define localtime4(a)	localtime(a)
#define time4(a)	time(a)
#define time4(a)	time(a)
#endif


#endif	/* _TIME4_H_ */
