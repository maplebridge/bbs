/*-------------------------------------------------------*/
/* util/show_dotUSR.c	( NTHU CS MapleBBS Ver 3.20 )	 */
/*-------------------------------------------------------*/
/* target : 印出 .USR 的內容                             */
/* create : 07/09/30					 */
/* update :   /  /  					 */
/* author : ckm.bbs@bbs.cs.nthu.edu.tw			 */
/*-------------------------------------------------------*/


#include "bbs.h"


int
main(argc, argv)
  int argc;
  char **argv;
{
  int fd;
  char buf[4096];
  int userno, size;
  SCHEMA *sp;			/* record length 16 可整除 4096 */

  chdir(BBSHOME);

  userno = 1;
  if ((fd = open(FN_SCHEMA, O_RDONLY)) >= 0)
  {
    while ((size = read(fd, buf, sizeof(buf))) > 0)
    {
      sp = (SCHEMA *) buf;
      do
      {
	if (sp->userid[0] == '\0')
	  printf("%-6d : empty!\n", userno);
	else
	  printf("%-6d : %s\n", userno, sp->userid);

	userno++;
	size -= sizeof(SCHEMA);
	sp++;
      } while (size);
    }
  }
  else
    printf("error! Cannot open %s", FN_SCHEMA);

  return 0;
}
