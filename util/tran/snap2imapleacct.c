/*-------------------------------------------------------*/
/* util/snap2imapleacct.c( NTHU CS MapleBBS Ver Maple )	 */
/*-------------------------------------------------------*/
/* target : iMaple ACCT 轉換程式				 */
/* create : 98/12/15					 */
/* update : 02/04/29					 */
/* author : mat.bbs@fall.twbbs.org			 */
/* modify : itoc.bbs@bbs.tnfsh.tn.edu.tw		 */
/*          ckm.bbs@lexel.twbbs.org			 */
/*          smiler.bbs@bbs.cs.nthu.edu.tw		 */
/*-------------------------------------------------------*/
/* syntax : snap2usr [userid]				 */
/*-------------------------------------------------------*/


#include "snap.h"
#include <netinet/in.h>

#define FN_DIR_TMP      ".DIR.tmp"
#define FN_ACCT_080827	".ACCT_080827"


#if 0
static usint
transfer_ufo(oldufo)
  usint oldufo;
{
  usint ufo;

  ufo = 0;

  if (oldufo & HABIT_MOVIE)
    ufo |= UFO_MOVIE;

  if (oldufo & HABIT_BNOTE)
    ufo |= UFO_BRDNOTE;

  if (oldufo & HABIT_VEDIT)
    ufo |= UFO_VEDIT;

//  if (oldufo & HABIT_MOTD)
//    ufo |= UFO_MOTD;

  if (oldufo & HABIT_PAGER)
    ufo |= UFO_PAGER;

  if (oldufo & HABIT_QUIET)
    ufo |= UFO_QUIET;

  if (oldufo & HABIT_PAL)
    ufo |= UFO_PAL;

  if (oldufo & HABIT_ALOHA)
    ufo |= UFO_ALOHA;

  /*if (oldufo & HABIT_NWLOG)
    ufo |= UFO_NWLOG;*/

  /*if (oldufo & HABIT_NTLOG)
    ufo |= UFO_NTLOG;*/

  if (oldufo & HABIT_CLOAK)
    ufo |= UFO_CLOAK;

  if (oldufo & HABIT_ACL)
    ufo |= UFO_ACL;

  return ufo;
}
#endif


/* ----------------------------------------------------- */
/* 轉換主程式						 */
/* ----------------------------------------------------- */


static void
trans_acct(old, new)
  ACCT_080827 *old;
  ACCT *new;
{
  memset(new, 0, sizeof(ACCT));

  new->userno = old->userno;

  str_ncpy(new->userid, old->userid, sizeof(new->userid));
  str_ncpy(new->passwd, old->passwd, sizeof(new->passwd));
  str_ncpy(new->realname, old->realname, sizeof(new->realname));
  str_ncpy(new->username, old->username, sizeof(new->username));

  new->userlevel = old->userlevel;
  new->ufo = old->ufo;
  new->signature = old->signature;

  new->year = old->year;
  new->month = old->month;
  new->day = old->day;
  new->sex = old->sex;
  new->money = old->money;
  new->gold = old->gold;

  new->numlogins = old->numlogins;
  new->numposts = old->numposts;
  new->numemails = old->numemails;

  new->firstlogin = old->firstlogin;
  new->lastlogin = old->lastlogin;
  new->tcheck = old->tcheck;
  new->tvalid = old->tvalid;

  new->staytime = old->staytime;

  str_ncpy(new->lasthost, old->lasthost, sizeof(new->lasthost));
  str_ncpy(new->email, old->email, sizeof(new->email));

  /* smiler.080827:以下為 080827 新增欄位 */
  new->good_article = 0;
  new->poor_article = 0;
  new->violation = 0;
  new->reserved_1 = 0;
  new->reserved_2 = 0;

  new->reserved[0] = '\0';
}


#if 0
static void
trans_hdr(old, new)
  MAPLECS_HDR *old;
  HDR *new;
{
  memset(new, 0, sizeof(HDR));

  new->chrono = ntohl(old->chrono);
  new->xmode  = ntohl(old->xmode);
  new->xid = ntohl(old->xid);

  str_ncpy(new->xname, old->xname, sizeof(new->xname));
  str_ncpy(new->owner, old->owner, sizeof(new->owner));
  str_ncpy(new->nick, old->nick, sizeof(new->nick));


  /*handle score
  old->score = ntohl(old->score);
  if (old->score > 126)
    new->score = 127;
  else if (old->score < -127)
    new->score = -128;
  else
    new->score = old->score;*/

  new->score = 0;


  str_ncpy(new->date, old->date, sizeof(new->date));
  str_ncpy(new->title, old->title, sizeof(new->title));
}


static void
trans_pal(old, new)
  MAPLE_PAL *old;
  PAL *new;
{
  memset(new, 0, sizeof(PAL));

  str_ncpy(new->userid, old->userid, sizeof(new->userid));
  new->ftype = old->ftype;  
  str_ncpy(new->ship, old->ship, sizeof(new->ship));
  new->userno = ntohl(old->userno);
}
#endif


int
main(argc, argv)
  int argc;
  char *argv[];
{
  ACCT new;
  char c;
//  HDR hdr;
//  PAL new_pal;
//  FILE *fp;

  /*printf("%d\n", argc);*/

  if (argc > 2)
  {
    printf("Usage: %s [userid]\n", argv[0]);
    return -1;
  }

  for (c = 'a'; c <= 'z'; c++)
  {
    char buf[64];
//    char buf2[64];
    char buf3[64];
    struct dirent *de;
    DIR *dirp;

    sprintf(buf, BBSHOME "/usr/%c", c);
    chdir(buf);
    //printf("chdir:%s\n",buf);

    if (!(dirp = opendir(".")))
       continue;

    while (de = readdir(dirp))
    {
      ACCT_080827 old;
//      MAPLECS_HDR old_hdr;
//      MAPLE_PAL old_pal;
      int fd;
      char *str;

      str = de->d_name;
      if (*str <= ' ' || *str == '.')
	      continue;

      if ((argc == 2) && str_cmp(str, argv[1]))
	      continue;

/* smiler.080827: 此次更新 .ACCT 不需建 folder */
#if 0

#ifdef MAK_DIRS
      sprintf(buf, "%s/MF", str);
      mkdir(buf, 0700);
      sprintf(buf, "%s/gem", str);
      mak_links(buf);
#endif
#endif

      sprintf(buf, "%s/" FN_ACCT, str);
	  sprintf(buf3, "%s/" FN_ACCT_080827, str);
      if ((fd = open(buf, O_RDONLY)) < 0)
	     continue;

      read(fd, &old, sizeof(ACCT_080827));
      close(fd);
      //unlink(buf);			/* itoc.010831: 砍掉原來的 FN_ACCT */
      rename(buf, buf3);        /* smiler.080827: 將原本的 .ACCT 備份為 .ACCT_080827 */

      trans_acct(&old, &new);

      fd = open(buf, O_WRONLY | O_CREAT, 0600);	/* itoc.010831: 重建新的 FN_ACCT */
      write(fd, &new, sizeof(ACCT));
      close(fd);

/* smiler.080827: 此次僅更新 .ACCT ，以下不動 */
#if 0
      // ckm.Apr22: for FRIEND
      sprintf(buf, "%s/friend", str);
      sprintf(buf2, "%s/friend.tmp", str);
      if(fp = fopen(buf, "r"))
	  {
	    while (fread(&old_pal, sizeof(old_pal), 1, fp) == 1)
		{
	      trans_pal(&old_pal, &new_pal);
	      rec_add(buf2, &new_pal, sizeof(PAL));
		}
	    fclose(fp);
	    unlink(buf);
	    rename(buf2, buf);
	  }
	  else
	    printf("This user has no :\n",buf);


      // ckm.Apr22,07: for signture
      sprintf(buf, "%s/sign", str);
      sprintf(buf2, "%s/sign.1",  str);
      if(fp = fopen(buf, "r"))
	  {
	    rename(buf, buf2);
	    fclose(fp);
      }
	  else
	    printf("This user has no :\n",buf);


      /* ckm.Feb01,07: 修改.DIR */
      sprintf(buf, "%s/" FN_DIR, str);
      sprintf(buf2, "%s/" FN_DIR_TMP, str);

     if (fp = fopen(buf, "r")){
        // {
	   //printf("open {%s} failed\n",buf);
        //return -1;
        // }
     while (fread(&old_hdr, sizeof(old_hdr), 1, fp) == 1)
         {


       trans_hdr(&old_hdr, &hdr);

       rec_add(buf2, &hdr, sizeof(HDR));
         }

     fclose(fp);
     }

      /* 刪除舊的，把新的更名 */
     unlink(buf);
     rename(buf2, buf);
#endif
    }

    closedir(dirp);    
  }

  return 0;
}
