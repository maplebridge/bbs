/*-------------------------------------------------------*/
/* util/snap2hdr.c	( NTHU CS MapleBBS Ver ·¬¾ôÅæ¯¸)	 */
/*-------------------------------------------------------*/
/* target : M3 ·¬¾ô rss Áý°eµ{¦¡			 */
/* create : 08/02/03					 */
/* author : smiler.bbs@lexel.twbbs.org			 */
/*-------------------------------------------------------*/
/* syntax : rss_feed				 */
/*-------------------------------------------------------*/


#include "snap.h"
#include <netinet/in.h>


int boardnumber=0;

int
main(argc, argv)
  int argc;
  char *argv[];
{
  FILE *fp;
//  HDR hdr;
  if (argc > 2)
  {
    printf("Usage: %s [board]\n", argv[0]);
    return -1;
  }

    char buf[64];

    struct dirent *de;
    DIR *dirp;

    sprintf(buf, BBSHOME"/brd");
    chdir(buf);

    dirp = opendir(".");

    while (de = readdir(dirp))
    {
      int fd;
      char *str;


      str = de->d_name;
      if (*str <= ' ' ||  *str == '.')
	continue;

      if ((argc == 2) && str_cmp(str, argv[1]))
	continue;

      sprintf(buf, "%s/" FN_RSS, str);

      /* perl main function */
	 if ((fp = fopen(buf, "r")))
	 {
//	   printf(" this is %s\n",str);
          char rss[64];
          sprintf(rss,BBSHOME"/bin/rss %s",str);
          system(rss);
       fclose(fp);
	 }
	 //boardnumber++;
	 //printf("%d . %s \n",boardnumber,buf);


	}

    closedir(dirp);


  return 0;
}
