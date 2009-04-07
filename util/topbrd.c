/*-------------------------------------------------------*/
/* topbrd.c ( NTHU CS MapleBBS Ver 3.10 )                */
/*-------------------------------------------------------*/
/* target : 熱門看板排行榜                               */
/* create : 03/07/03                                     */
/* update : 03/07/03                                     */
/* author : itoc.bbs@bbs.tnfsh.tn.edu.tw                 */
/* modify : qazq.bbs@bbs.cs.nchu.edu.tw                  */
/*-------------------------------------------------------*/


#include "bbs.h"


#if 0
crontab 新增:
36 3 * * * bin/topbrd > /dev/null 2>&1
//一定要先過午夜12點才能跑唷！因為要讓 account 跑出 usies-sort
//或是直接放進 account 給 system() 做
#endif

#define BRD_TIMES	"run/brd_times.log"	/* FN_RUN_BRDUSIES ".log" */
#define TOPBRD		"gem/@/@-topbrd"

/*-------------------------------------------------------*/
/* BRD shm 部分須與 cache.c 相容                         */
/*-------------------------------------------------------*/


static BCACHE *bshm;


static void
init_bshm()
{
  /* itoc.030727: 在開啟 bbsd 之前，應該就要執行過 account，所以 bshm 應該已設定好 */

  bshm = shm_new(BRDSHM_KEY, sizeof(BCACHE));

  if (bshm->uptime <= 0)		/* bshm 未設定完成 */
    exit(0);
}


static BRD *
brd_get(brdname)
  char *brdname;
{
  BRD *bhdr, *tail;
  
  system(FN_BIN_ACCOUNT);       /* smiler.090408: 準備好 bshm */

  bhdr = bshm->bcache;
  tail = bhdr + bshm->number;
  do
  {
    if (!strcmp(brdname, bhdr->brdname))
      return bhdr;
  } while (++bhdr < tail);
  return NULL;
}


typedef struct
{
  int times;			/* 進入看板閱讀次數 */
  char brdname[IDLEN + 1];		/* 板名 */
}   BRDDATA;


static int
int_cmp(a, b)
  BRDDATA *a, *b;
{
  return (b->times - a->times);		/* 由大排到小 */
}


int
main()
{
  time_t now;
  struct tm *ptime;
  BRDDATA board[MAXBOARD];
  FILE *fp;
  int locus, i, m;
  BRD *bhdr;

  chdir(BBSHOME);

  if (!(fp = fopen(BRD_TIMES, "r")))
    return;

  locus = 0;
  i = 0;
  while (!feof(fp))
  {
    fscanf(fp, "%s%d", board[i].brdname, &(board[i].times));
    locus++;
    i++;
  }
  fclose(fp);

  qsort(board, locus, sizeof(BRDDATA), int_cmp);

  init_bshm();

  fp = fopen(TOPBRD, "w");

// 下面是顯示畫面，可以自己畫....看起來和無名小站的有點像...(逃...:P)

  time(&now);
  ptime = localtime(&now);
  fprintf(fp, "        ┤\033[1;41m        熱門看板排行榜        \033[m├"
    "        \033[33m統計日期: \033[36m%d 月 %d 日\033[m\n\n",
    ptime->tm_mon + 1, ptime->tm_mday);

  m = 1;
  for (i = 0; i < locus; i++)
  {
    if (board[i].times == 0)
      break;

    if (!(bhdr = brd_get(board[i].brdname)))	/* 此板已改名或被砍 */
      continue;

    /* 跳過不列入排行榜的看板 */
    /* (BASIC + ... + VALID) < (VALID << 1) */
    if ((bhdr->readlevel | bhdr->postlevel) >= (PERM_VALID << 1))
      continue;

    fprintf(fp, "\033[1;31m%4d. \033[33m看板: \033[32m%-13s  "
      "\033[1;3%dm%5.5s\033[m %-34.33s  \033[1;35m人次 \033[36m%-3d\033[m\n",
      m, board[i].brdname, bhdr->class[3] & 7, bhdr->class, bhdr->title, board[i].times);

    m++;
  }

  fclose(fp);
}
