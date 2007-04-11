/*-------------------------------------------------------*/
/* util/snap2hdr.c	( NTHU CS MapleBBS Ver WRETCH )	 */
/*-------------------------------------------------------*/
/* target : M3 WRETCH HDR 轉換程式			 */
/* create : 07/01/26					 */
/* author : smiler.bbs@lexel.twbbs.org			 */
/*-------------------------------------------------------*/
/* syntax : snap2hdr [board]				 */
/*-------------------------------------------------------*/


#include "snap.h"
#include <netinet/in.h>

#define FN_DIR_TMP	"./.DIR.tmp"   /*----------------*/

void
str_ncpy_e(dst, src, n, e)
  char *dst;
  char *src;
  int n;
  int e;
{
  char tmp=' ';
  char *end;
  char *end2;

  end = e;
  end2 =end + n - 1;

  do
  {
    n = (dst >= end) ? 0 : tmp;
    *dst++ = n;
  } while (n);
  do
  {
    n = (dst >= end2) ? 0 : *src++;
    *dst++ = n;
  } while (n);
}

/* ----------------------------------------------------- */
/* 轉換主程式						 */
/* ----------------------------------------------------- */


static void
trans_hdr(old, new)
  WRETCH_HDR *old;
  HDR *new;
{
  memset(new, 0, sizeof(HDR));

  new->chrono = old->chrono;
  new->xmode  = old->xmode;
  new->xid = old->xid;

  str_ncpy(new->xname, old->xname, sizeof(new->xname));
  str_ncpy(new->owner, old->owner, sizeof(new->owner)-4);
  str_ncpy(new->nick, old->nick, sizeof(new->nick));


  //handle score
	  new->score = 0;


  str_ncpy(new->date, old->date, sizeof(new->date));
  str_ncpy(new->title, old->title, sizeof(new->title));
}


int
main(argc, argv)
  int argc;
  char *argv[];
{
  HDR new;
  FILE *fp;                      /*-----------*/
  HDR hdr;                       /*-----------*/
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
    
    /*if (!(dirp = opendir(".")))
      continue;*/
    dirp = opendir(".");

    while (de = readdir(dirp))
    {
      WRETCH_HDR old;
      int fd;
      char *str;
      printf("ok\n");

      str = de->d_name;
      if (*str <= ' ' || *str == '.')
	continue;

      if ((argc == 2) && str_cmp(str, argv[1]))
	continue;

      sprintf(buf, "%s/" FN_DIR, str);
//      if ((fd = open(buf, O_RDONLY)) < 0)
  //   	continue;

    //  read(fd, &old, sizeof(LEXEL_HDR));
      //close(fd);
    //  unlink(buf);			/* itoc.010831: 砍掉原來的 FN_ACCT */

    //  trans_hdr(&old, &new);

      //fd = open(buf, O_WRONLY | O_CREAT, 0600);	/* itoc.010831: 重建新的 FN_ACCT */
      //write(fd, &new, sizeof(HDR));
      //close(fd);
     if (!(fp = fopen(buf, "r")))
	 {
        return -1;
	 }
     while (fread(&old, sizeof(old), 1, fp) == 1)
	 {


       trans_hdr(&old, &hdr);
 
       rec_add(FN_DIR_TMP, &hdr, sizeof(HDR));
	 }

     fclose(fp);

      /* 刪除舊的，把新的更名 */
     unlink(buf);
     rename(FN_DIR_TMP, FN_DIR);

	}

    closedir(dirp);    


  return 0;
}
