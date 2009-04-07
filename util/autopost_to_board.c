/*-------------------------------------------------------*/
/* util/autopost_to_board.c ( NTHU CS MapleBBS Ver 3.20 )*/
/*-------------------------------------------------------*/
/* target : MapleBBS 自動 post 系統			 */
/* create : 08/08/25					 */
/* author : smiler.bbs@bbs.cs.nthu.edu.tw		 */
/* update : //						 */
/*-------------------------------------------------------*/


#include "bbs.h"

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sysexits.h>


#if 0
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
  
  system(FN_BIN_ACCOUNT);       /* smiler.090408: 準備好 bshm */

  bhdr = bshm->bcache;
  tail = bhdr + bshm->number;
  do
  {
    if (!str_cmp(bname, bhdr->brdname))
      return bhdr;
  } while (++bhdr < tail);
  return NULL;
}
#endif


static void
add_post_util(brdname, fpath, title, userid, nick)	/* 發文到看板 */
  char *brdname;	/* 欲 post 的看板 */
  char *fpath;		/* 檔案路徑 */
  char *title;		/* 文章標題 */
  char *userid;
  char *nick;
{
  HDR hdr;
  char folder[64];

  brd_fpath(folder, brdname, FN_DIR);

  hdr_stamp(folder, HDR_COPY | 'A', &hdr, fpath);
  strcpy(hdr.owner, userid);
  strcpy(hdr.nick, nick);
  strcpy(hdr.title, title);
  if (userid[strlen(userid) - 1] == '.')
    hdr.xmode = POST_INCOME;
  rec_bot(folder, &hdr, sizeof(HDR));

  //btime_update(brd_bno(brdname));
}


int
main(argc, argv)
  int argc;
  char *argv[];
{
  FILE *fp;
  char filepath[64];
  char owner[76];
  char nick[40];
  char title[73];

  if (argc < 5)
  {
	//                     1        2      3    4
    printf("Usage:\t%s <brdname> <userid> <nick> <use>\n", argv[0]);
    exit(-1);
  }

//  init_bshm();

  chdir(BBSHOME);

  /* <use> 為 RSS_POST_TO_BBS 時 */
  if (!strcmp(argv[4], "RSS_POST_TO_BBS"))
  {

    /* smiler.080825: 擷取 title from ./RSS_POST_TITLE */
    brd_fpath(filepath, argv[1], ".RSS_POST_TITLE");
    if (fp = fopen(filepath, "r"))
    {
      fgets (title , 70 , fp);
      title[strlen(title) - 1]='\0';
      fclose(fp);
    }
    else
      strcpy(title,"iMaple RSS Feed");

    printf("%s\n", argv[4]);
    brd_fpath(filepath, argv[1], ".RSS_POST");
    //str_ncpy(owner, argv[2], 73-3);
    sprintf(owner, "%s.", argv[2]);
    str_ncpy(nick, argv[3], 40-3);
  }

  add_post_util(argv[1], filepath, title, owner, nick);

  return 0;
}
