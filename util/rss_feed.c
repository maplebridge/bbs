/*-------------------------------------------------------*/
/* util/rss_fed.c	( NTHU CS MapleBBS Ver 楓橋驛站) */
/*-------------------------------------------------------*/
/* target : M3 楓橋 rss 餵送程式			 */
/* create : 08/08/23					 */
/* author : smiler.bbs@bbs.cs.nthu.edu.tw		 */
/*-------------------------------------------------------*/
/* syntax : rss_feed					 */
/*-------------------------------------------------------*/


#include "bbs.h"
#include <netinet/in.h>

#define FN_RSS_BAK		"./.RSS.bak"	
#define FN_RSS_TMP		"./.RSS.tmp"

/* smiler.080823: store entry->id */
#define FN_RSS_ENTRY		"./.RSS.ENTRY"

/* smiler.080823: store total entry->id number */
#define FN_RSS_ENTRY_NUM	"./.RSS.ENTRY_NUM"

/* smiler.080823: BBS<->RSS conversation */
#define FN_BBS_TO_RSS		"./.RSS.BBS_TO_RSS"
#define FN_RSS_TO_BBS		"./.RSS.RSS_TO_BBS"


/* ----------------------------------------------------- */
/* board：shm 部份須與 cache.c 相容			 */
/* ----------------------------------------------------- */


static BCACHE *bshm;


static void
init_bshm()
{
  /* itoc.030727: 在開啟 bbsd 之前，應該就要執行過 account，
     所以 bshm 應該已設定好 */

  bshm = shm_new(BRDSHM_KEY, sizeof(BCACHE));

  if (bshm->uptime <= 0)	/* bshm 未設定完成 */
    exit(0);
}


static BRD *
brd_get(bname)
  char *bname;
{
  BRD *bhdr, *tail;

  bhdr = bshm->bcache;
  tail = bhdr + bshm->number;
  do
  {
    if (!str_cmp(bname, bhdr->brdname))
      return bhdr;
  } while (++bhdr < tail);
  return NULL;
}


/* ----------------------------------------------------- */
/* 餵送主程式						 */
/* ----------------------------------------------------- */

static void
send_rss(old, new, brdname)
  RSS *old;
  RSS *new;
  char *brdname;
{
  FILE *fp;
  char file_path[64];
  char cmd[256];

  memset(new, 0, sizeof(RSS));
  strcpy(new->bookmark, old->bookmark);
  new->chrono = old->chrono;
  new->xmode = old->xmode;
  new->xid = old->xid;
  strcpy(new->owner, old->owner);
  strcpy(new->date, old->date);
  strcpy(new->url, old->url);
  strcpy(new->info, old->info);
  strcpy(new->code_type, old->code_type);
  strcpy(new->modified, old->modified);

  /* smiler.080823: 重送前，先將之前暫存的 BBS<->RSS message 清掉 */
  sprintf(file_path, "%s/" FN_BBS_TO_RSS, brdname);
  unlink(file_path);
  sprintf(file_path, "%s/" FN_RSS_TO_BBS, brdname);
  unlink(file_path);

  if (new->xmode & RSS_RESTART)
    strcpy(new->modified, "start");	/* smiler.080823: 設定重送時，將modified清為"start" */

  if (new->xmode & RSS_START)
  {
    /* smiler.080823: 送出 BBS_TO_RSS message  */
    sprintf(file_path, "%s/" FN_BBS_TO_RSS, brdname);
    fp = fopen(file_path, "w");

    fprintf(fp,"%s\n", new->owner);

    fprintf(fp, "%s\n", (new->bookmark[0] != '\0') ? new->bookmark : "RSS");

    fprintf(fp, "%s\n", new->url);

    fprintf(fp, "%s\n", new->modified);

    fprintf(fp, "%c\n", (new->xmode & RSS_TXT) ? 'y' : 'n');

    fprintf(fp, "%c\n", (new->xmode & RSS_RESTART) ? 'r' : 'n');

    fclose(fp);

    /* smiler.080823: 送出 command */
    sprintf(cmd, BBSHOME "/bin/rss_b003 %s %s;", BBSHOME, brdname);
    system(cmd);

    /* smiler.080823: 取回 RSS_TO_BBS message */
    sprintf(file_path, "%s/" FN_RSS_TO_BBS, brdname);
    if (fp = fopen(file_path, "r"))
    {
      fgets(new->modified , 64 , fp);
      fclose(fp);
    }
    else
      strcpy(new->modified, "start");	/* 若 feed->modified 未回傳，則modified重設為start */
  }

  if (new->xmode & RSS_RESTART)
     new->xmode &= (~RSS_RESTART);	/* 最後將 RSS_RESTART 歸 0 */
}


int
main(argc, argv)
  int argc;
  char *argv[];
{
  FILE *fp;
  RSS rss;

#if 0
  if (argc > 2)
  {
    printf("Usage: %s [board]\n", argv[0]);
    return -1;
  }
#endif

  char buf[64];
  char buf2[64];
  char buf3[64];

  struct dirent *de;
  DIR *dirp;

  sprintf(buf, BBSHOME "/brd");
  chdir(buf);

  init_bshm();

  dirp = opendir(".");

  while (de = readdir(dirp))
  {
    RSS old;
    char *str;

    str = de->d_name;
    if (*str <= ' ' ||  *str == '.')
      continue;

    if ((argc == 2) && str_cmp(str, argv[1]))
      continue;

    sprintf(buf, "%s/" FN_RSS, str);
    sprintf(buf2, "%s/" FN_RSS_TMP, str);
    sprintf(buf3, "%s/" FN_RSS_BAK, str);

    if (dashf(buf))
    {
      char cmd[64];
      sprintf(cmd, "cp %s %s", buf, buf3);
      system(cmd);

      fp = fopen(buf3, "r");

      /* smiler.080823: 若 .RSS.ENTRY_NUM 不存在，就先開檔，並寫入0 */
      FILE *fp_entry_num;
      char file_path[64];

      sprintf(file_path, "%s/" FN_RSS_ENTRY_NUM, str);
      if (!dashf(file_path))
      {
	fp_entry_num = fopen (file_path, "w");
	fputc('0', fp_entry_num);
        fclose(fp_entry_num);
      }

      while (fread(&old, sizeof(old), 1, fp) == 1)
      {
	send_rss(&old, &rss, str);
	rec_add(buf2, &rss, sizeof(RSS));

	/* 更新看板 btime */
	BRD *brd;
	brd = brd_get(str);
	brd->btime = -1;
      }

      fclose(fp);

      /* 刪除舊的，把新的更名 */
      unlink(buf3);
      rename(buf2, buf3);

      /* 將備份檔寫回 .RSS */
      unlink(buf);
      rename(buf3, buf);
    }
  }

  closedir(dirp);

  return 0;
}
