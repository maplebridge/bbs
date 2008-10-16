/*-------------------------------------------------------*/
/* util/snap2hdr.c	( NTHU CS MapleBBS Ver 楓橋驛站)	 */
/*-------------------------------------------------------*/
/* target : M3 楓橋 DIR 轉換程式				 */
/* create : 07/01/25					 */
/* author : smiler.bbs@lexel.twbbs.org			 */
/*-------------------------------------------------------*/
/* syntax : snap2hdr [board]				 */
/*-------------------------------------------------------*/


#include "snap.h"
#include <netinet/in.h>

#define FN_DIR_TMP	"./.DIR.tmp"
#define FN_DIR_TMP2

int boardnumber = 0;

/* ----------------------------------------------------- */
/* 轉換主程式						 */
/* ----------------------------------------------------- */

static void
trans_hdr(old, new)
  MAPLECS_HDR *old;
  HDR *new;
{
  memset(new, 0, sizeof(HDR));

  new->chrono = ntohl(old->chrono);
  new->xmode = ntohl(old->xmode);
  //修改xmode//
  if ((new->xmode & 0x00000004)==0x00000004)  //刪除楓橋的Post_GEM(G文)標記
    new->xmode=new->xmode - 0x00000004;
  if ((new->xmode & 0x00000040)==0x00000040)  //刪除楓橋的Post_CANCEL標記
    new->xmode=new->xmode - 0x00000040;
  if ((new->xmode & 0x00000200)==0x00000200)  //刪除楓橋的Post_EMAIL標記
    new->xmode=new->xmode - 0x00000200;
  if ((new->xmode & 0x00001000)==0x00001000)  //將楓橋的Post_BOTTOM(置底)標記改至ITOC相對應的位置
    new->xmode=new->xmode - 0x00001000 + 0x00000040;

  new->xid = ntohl(old->xid);

  str_ncpy(new->xname, old->xname, sizeof(new->xname));
  str_ncpy(new->owner, old->owner, sizeof(new->owner));
  str_ncpy(new->nick, old->nick, sizeof(new->nick));

  new->score = 0;

  str_ncpy(new->date, old->date, sizeof(new->date));
  str_ncpy(new->title, old->title, sizeof(new->title));
}


int
main(argc, argv)
  int argc;
  char *argv[];
{
  FILE *fp;
  HDR hdr;
  if (argc > 2)
  {
    printf("Usage: %s [board]\n", argv[0]);
    return -1;
  }

  char buf[64];
  char buf2[64];

  struct dirent *de;
  DIR *dirp;

  sprintf(buf, BBSHOME"/brd");
  chdir(buf);

  dirp = opendir(".");

  while (de = readdir(dirp))
  {
    MAPLECS_HDR old;
    char *str;

    str = de->d_name;
    if (*str <= ' ' ||  *str == '.')
      continue;

    if ((argc == 2) && str_cmp(str, argv[1]))
      continue;

    sprintf(buf, "%s/" FN_DIR, str);
    sprintf(buf2, "%s/" FN_DIR_TMP, str);

    if ((fp = fopen(buf, "r")))
    {
      while (fread(&old, sizeof(old), 1, fp) == 1)
      {
	trans_hdr(&old, &hdr);
 
	rec_add(buf2, &hdr, sizeof(HDR));
      }
      fclose(fp);

      /* 刪除舊的，把新的更名 */
      unlink(buf);
      rename(buf2, buf);
    }
    boardnumber++;
    printf("%d . %s \n",boardnumber,buf);
    //getchar();
  }

  closedir(dirp);
  return 0;
}
