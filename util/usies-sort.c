/*-------------------------------------------------------*/
/* util/usies-sort.c	( NTHU CS MapleBBS Ver 3.10 )	 */
/*-------------------------------------------------------*/
/* target : brd_usies 竒 acl-sort 禲セ祘Α羆璸 */
/*          狾ㄏノΩ                               */
/* create : 01/02/24				 	 */
/* update :   /  /				 	 */
/* author : itoc.bbs@bbs.tnfsh.tn.edu.tw                 */
/*-------------------------------------------------------*/
/* syntax : usies-sort <file>				 */
/*-------------------------------------------------------*/


//#include <stdio.h>
#include "bbs.h"

#define NUM_BADBOARD	100000	/*  100000 Ω狾ㄏノ瞯 */
#define NUM_DROPBOARD	20	/*  20 Ω狾ぃ癘魁 */
#define BRD_TIMES	(BBSHOME "/run/brd_times.log")	/*qazq:狾綷弄Ω计魁*/


static void
usies_sort(fpath)
  char *fpath;
{
  int num;
  char brdname[60], buf[60];
  char *str;
  FILE *fp, *ftimes;

  if (!(fp = fopen(fpath, "r")))
    return;

  ftimes = fopen(BRD_TIMES, "w");

  if (!fgets(brdname, sizeof(buf), fp))	/* 材狾 */
  {
    fclose(fp);
    return;
  }

  printf("狾           Ω  (参璸ぶ %d  0 Ωぃ)\n", NUM_BADBOARD);

  str = brdname;
  while (*++str != ' ')
    ;
  str_ncpy(brdname, brdname, str - brdname + 1);
  num = 1;

  while (fgets(buf, sizeof(buf), fp))
  {
    str = buf;

    while (*++str != ' ')
      ;

    str_ncpy(buf, buf, str - buf + 1);

    if (!strcmp(brdname, buf))	/* 竒瞷狾綷弄Ω */
    {
      num++;
    }
    else			/* 穝瞷狾 */
    {
      if ((num >= NUM_DROPBOARD) && (num < NUM_BADBOARD))      	/* 参璸玡狾ㄏノ碭Ω */
      {
        printf("%-15s%d\n", brdname, num);
        fprintf(ftimes, "%s %d\n", brdname, num);
      }
      strcpy(brdname, buf);
      num = 1;
    }
  }
  if ((num >= NUM_DROPBOARD) && (num < NUM_BADBOARD))		/* 参璸程狾ㄏノ碭Ω */
    printf("%-15s%d\n", brdname, num);
  fclose(fp);
}


int
main(argc, argv)
  int argc;
  char *argv[];
{
  if (argc != 2)
  {
    printf("Usage:\t%s file\n", argv[0]);
  }
  else
  {
    usies_sort(argv[1]);	/* "brd_usies.log" */
  }
  exit(0);
}
