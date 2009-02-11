/*-------------------------------------------------------*/
/* post.c       ( NTHU CS MapleBBS Ver 2.39 )		 */
/*-------------------------------------------------------*/
/* target : bulletin boards' routines		 	 */
/* create : 95/03/29				 	 */
/* update : 96/04/05				 	 */
/*-------------------------------------------------------*/


#include "bbs.h"

#define DO_POST_FILTER


extern BCACHE *bshm;
extern XZ xz[];


extern int wordsnum;		/* itoc.010408: 計算文章字數 */
extern int TagNum;
extern char xo_pool[];
extern char brd_bits[];


#ifdef HAVE_ANONYMOUS
extern char anonymousid[];	/* itoc.010717: 自定匿名 ID */
#endif

static char bbs_dog_str[80];
static char bbs_dog_title[80];


int
cmpchrono(hdr)
  HDR *hdr;
{
  return hdr->chrono == currchrono;
}


static int
IS_BIGGER_AGE(age)
  int age;
{
  time_t now;
  struct tm *ptime;

  if (age && !cuser.year)	/* 生日未填 */
    return 0;

  time(&now);
  ptime = localtime(&now);

  return ((cuser.year + 11 + age) < ptime->tm_year) ? 1 :
	((cuser.year + 11 + age) > ptime->tm_year) ? 0 :
	(cuser.month < (ptime->tm_mon + 1))        ? 1 :
	(cuser.month > (ptime->tm_mon + 1))        ? 0 :
	(cuser.day > (ptime->tm_mday))             ? 0 : 1;
}


static int
IS_BIGGER_1STLG(month)
  int month;
{
  /* 一個月以 30 天計算 */
  return (((time(0) - cuser.firstlogin) / (86400 * 30)) >= month);
#if 0
  time_t this_time;
  struct tm *ptime;
  int my_year, my_month, my_day;
  int now_year, now_month, now_day;

  time(&this_time);
  ptime = localtime(&this_time);

  now_year  = ptime->tm_year;
  now_month = ptime->tm_mon;
  now_day   = ptime->tm_mday;

  ptime = localtime(&cuser.firstlogin);

  my_year = ptime->tm_year;
  my_month = ptime->tm_mon;
  my_day = ptime->tm_mday;

  return ((my_year   + (month / 12)) < now_year   ) ? 1 :
	((my_year   + (month / 12)) > now_year   ) ? 0 :
	((my_month  + (month % 12)) < now_month  ) ? 1 :
	((my_month  + (month % 12)) > now_month  ) ? 0 :
	( my_day                    > now_day    ) ? 0 : 1 ;
#endif
}


#define	COLOR_NOT_ACP	"\033[1;31m"
#define	COLOR_ACP	"\033[1;37m"	


int
IS_WELCOME(board, fname)
  char *board;
  char *fname;
{
  FILE *fp;
  char fpath[64];
  int wi = 0;

  brd_fpath(fpath, board, fname);

  if (!(fp = fopen(fpath, "r")))
    return 1;

  while (1)
  {
    fscanf(fp, "%d", &wi);
    if (!IS_BIGGER_AGE(wi))
      break;

    fscanf(fp, "%d", &wi);
    if (wi && (wi != cuser.sex + 1))
      break;

    fscanf(fp, "%d", &wi);
    if (cuser.numlogins < wi)
      break;

    fscanf(fp, "%d", &wi);
    if (cuser.numposts < wi)
      break;

    fscanf(fp, "%d", &wi);
    if (cuser.good_article < wi)
      break;

    fscanf(fp, "%d", &wi);
    if (wi && (cuser.poor_article >= wi))
      break;

    fscanf(fp, "%d", &wi);
    if (wi && (cuser.violation >= wi))
      break;

    fscanf(fp, "%d", &wi);
    if (cuser.money < wi)
      break;

    fscanf(fp, "%d", &wi);
    if (cuser.gold < wi)
      break;

    fscanf(fp, "%d", &wi);
    if (cuser.numemails < wi)
      break;

    fscanf(fp, "%d", &wi);
    if (!IS_BIGGER_1STLG(wi))
      break;

    /* 會走到這裡就表示過關了 */
    fclose(fp);
    return 1;
  }

  fclose(fp);
  return 0;
}


/* smiler.080830 : 看門狗 */
static int
IS_BRD_DOG_FOOD(fpath, board)
  char *fpath;
  char *board;
{
  FILE *fp;
  char fpath_filter[64];
  char filter[73];

  brd_fpath(fpath_filter, board, FN_BBSDOG);

  if (!(fp = fopen(fpath_filter, "r")))
    return 0;

  while (fgets(filter, sizeof(filter), fp))
  {
    if (filter[0] == '\0' || filter[0] == '\n')
      continue;
    else
      filter[strlen(filter) - 1] = '\0';

    if (f_str_sub_space_lf(fpath, filter))
    {
      strcpy(bbs_dog_str, filter);
      fclose(fp);
      return 1;
    }
  }

  fclose(fp);
  return 0;

}


static int
IS_BBS_DOG_FOOD(fpath)
  char *fpath;
{
  FILE *fp;
  char fpath_filter[64];
  char filter[73];

  sprintf(fpath_filter, FN_ETC_BBSDOG);

  if (!(fp = fopen(fpath_filter, "r")))
    return 0;

  while (fgets(filter, sizeof(filter), fp))
  {
    if (filter[0] == '\0' || filter[0] == '\n')
      continue;
    else
      filter[strlen(filter) - 1] = '\0';

     if (f_str_sub_all_chr(fpath, filter))
     {
       strcpy(bbs_dog_str, filter);
       fclose(fp);
       return 1;
     }
  }

  fclose(fp);
  return 0;

}

/* smiler.080830 : 看門狗對文章內容過濾 */
int	/* 0: 一般正常文章  1: 被擋下來了 */
post_filter(fpath)
  char *fpath;			/* file name of access control list */
{
  char warn[70];
  char fpath_log[64];
  char content_log[256];

  /* smiler.080910: 讓使用者決定是否加入BBS看門狗計畫 */
  if ((currbattr & BRD_BBS_DOG) && (IS_BBS_DOG_FOOD(fpath)))
  {
    brd_fpath(fpath_log, currboard, FN_BBSDOG_LOG);
    sprintf(content_log, "%s BBS看門狗計畫: 文章寄回給原po\n作者: %s\n標題: %s\n\n",
      Now(), cuser.userid, bbs_dog_title);
    f_cat(fpath_log, content_log);

    sprintf(content_log, "%s BBS看門狗計畫: 文章寄回給原po\n作者: %s\n看板: %s\n標題: %s\n字串: %s\n\n",
      Now(), cuser.userid, currboard, bbs_dog_title, bbs_dog_str);
    f_cat(FN_ETC_BBSDOG_LOG, content_log);

    vmsg("您所post文章不為本站接受，請洽本站站務群");
    sprintf(warn, "[警告] 本篇文章不為本站接受，有問題請洽站務群");
    mail_self(fpath, cuser.userid, warn, 0);
    sprintf(warn, "[警告] 本篇文章不為本站接受，有問題請洽站務群\n");
    f_cat(fpath, warn);
    unlink(fpath);
    return 1;
  }

  if (IS_BRD_DOG_FOOD(fpath, currboard))
  {
    brd_fpath(fpath_log, currboard, FN_BBSDOG_LOG);
    sprintf(content_log, "%s 文章內容限制: 文章寄回給原po\n作者: %s\n標題: %s\n\n",
      Now(), cuser.userid, bbs_dog_title);
    f_cat(fpath_log, content_log);

    sprintf(content_log, "%s 文章內容限制: 文章寄回給原po\n作者: %s\n看板: %s\n標題: %s\n字串: %s\n\n",
      Now(), cuser.userid, currboard, bbs_dog_title, bbs_dog_str);
    f_cat(FN_ETC_BBSDOG_LOG, content_log);

    vmsg("您所post文章不為本看板接受，請洽本看板板主");
    sprintf(warn, "[警告] 本篇文章不為 %s 板接受，有問題請洽看板板主", currboard);
    mail_self(fpath, cuser.userid, warn, 0);
    sprintf(warn, "[警告] 本篇文章不為 %s 板接受，有問題請洽看板板主\n", currboard);
    f_cat(fpath, warn);
    unlink(fpath);
    return 1;
  }
#if 0
  char filter[256];

  while (fgets(filter, sizeof(filter), fp))
  {
    if (strstr(filter, "paperdo@gmail.com") || strstr(filter, "http://paperdo.googlepages.com/index.htm")
	|| strstr(filter, "論文代寫") || strstr(filter, "代寫論文"))
      return 1;
  }
#endif

  return 0;
}

/* ----------------------------------------------------- */
/* 改良 innbbsd 轉出信件、連線砍信之處理程序		 */
/* ----------------------------------------------------- */


void
btime_update(bno)
  int bno;
{
  if (bno >= 0)
    (bshm->bcache + bno)->btime = -1;	/* 讓 class_item() 更新用 */
}


#ifndef HAVE_NETTOOL
static			/* 給 enews.c 用 */
#endif
void
outgo_post(hdr, board)
  HDR *hdr;
  char *board;
{
  bntp_t bntp;

  memset(&bntp, 0, sizeof(bntp_t));

  if (board)		/* 新信 */
  {
    bntp.chrono = hdr->chrono;
  }
  else			/* cancel */
  {
    bntp.chrono = -1;
    board = currboard;
  }
  strcpy(bntp.board, board);
  strcpy(bntp.xname, hdr->xname);
  strcpy(bntp.owner, hdr->owner);
  strcpy(bntp.nick, hdr->nick);
  strcpy(bntp.title, hdr->title);
  rec_add("innd/out.bntp", &bntp, sizeof(bntp_t));
}


void
cancel_post(hdr)
  HDR *hdr;
{
  if ((hdr->xmode & POST_OUTGO) &&		/* 外轉信件 */
    (hdr->chrono > ap_start - 7 * 86400))	/* 7 天之內有效 */
  {
    outgo_post(hdr, NULL);
  }
}


static inline int		/* 回傳文章 size 去扣錢 */
move_post(hdr, folder, by_bm)	/* 將 hdr 從 folder 搬到別的板 */
  HDR *hdr;
  char *folder;
  int by_bm;
{
  HDR post;
  int xmode;
  char fpath[64], fnew[64], *board;
  struct stat st;

  xmode = hdr->xmode;
  hdr_fpath(fpath, folder, hdr);

  if (!(xmode & POST_BOTTOM))	/* 置底文被砍不用 move_post */
  {
#ifdef HAVE_REFUSEMARK
    board = by_bm && !(xmode & POST_RESTRICT) ? BN_DELETED : BN_JUNK;	/* 加密文章丟去 junk */
#else
    board = by_bm ? BN_DELETED : BN_JUNK;
#endif

    brd_fpath(fnew, board, fn_dir);
    hdr_stamp(fnew, HDR_LINK | 'A', &post, fpath);

    /* 直接複製 trailing data：owner(含)以下所有欄位 */

    memcpy(post.owner, hdr->owner, sizeof(HDR) -
      (sizeof(post.chrono) + sizeof(post.xmode) + sizeof(post.xid) + sizeof(post.xname)));

    if (by_bm)
      sprintf(post.title, "%-13s%.59s", cuser.userid, hdr->title);

    rec_bot(fnew, &post, sizeof(HDR));
    btime_update(brd_bno(board));
  }

  by_bm = stat(fpath, &st) ? 0 : st.st_size;

  unlink(fpath);
  btime_update(currbno);
  cancel_post(hdr);

  return by_bm;
}


#ifdef HAVE_DETECT_CROSSPOST
/* ----------------------------------------------------- */
/* 改良 cross post 停權					 */
/* ----------------------------------------------------- */


#define MAX_CHECKSUM_POST	20	/* 記錄最近 20 篇文章的 checksum */
#define MAX_CHECKSUM_LINE	6	/* 只取文章前 6 行來算 checksum */


typedef struct
{
  int sum;			/* 文章的 checksum */
  int total;			/* 此文章已發表幾篇 */
}      CHECKSUM;


static CHECKSUM checksum[MAX_CHECKSUM_POST];
static int checknum = 0;


static inline int
checksum_add(str)		/* 回傳本列文字的 checksum */
  char *str;
{
  int i, len, sum;

  len = strlen(str);

  sum = len;	/* 當字數太少時，前四分之一很可能完全相同，所以將字數也加入 sum 值 */
  for (i = len >> 2; i > 0; i--)	/* 只算前四分之一字元的 sum 值 */
    sum += *str++;

  return sum;
}


static inline int		/* 1:是cross-post 0:不是cross-post */
checksum_put(sum)
  int sum;
{
  int i;

  if (sum)
  {
    for (i = 0; i < MAX_CHECKSUM_POST; i++)
    {
      if (checksum[i].sum == sum)
      {
	checksum[i].total++;

	if (checksum[i].total > MAX_CROSS_POST)
	  return 1;
	return 0;	/* total <= MAX_CROSS_POST */
      }
    }

    if (++checknum >= MAX_CHECKSUM_POST)
      checknum = 0;
    checksum[checknum].sum = sum;
    checksum[checknum].total = 1;
  }
  return 0;
}


static int			/* 1:是cross-post 0:不是cross-post */
checksum_find(fpath)
  char *fpath;
{
  int i, sum;
  char buf[ANSILINELEN];
  FILE *fp;

  sum = 0;
  if (fp = fopen(fpath, "r"))
  {
    for (i = -(LINE_HEADER + 1);;)	/* 前幾列是檔頭 */
    {
      if (!fgets(buf, ANSILINELEN, fp))
	break;

      if (i < 0)	/* 跳過檔頭 */
      {
	i++;
	continue;
      }

      if (*buf == QUOTE_CHAR1 || *buf == '\n' || !strncmp(buf, "※", 2))	 /* 跳過引言 */
	continue;

      sum += checksum_add(buf);
      if (++i >= MAX_CHECKSUM_LINE)
	break;
    }
    fclose(fp);
  }

  return checksum_put(sum);
}


static int
check_crosspost(fpath, bno)
  char *fpath;
  int bno;			/* 要轉去的看板 */
{
  char *blist, folder[64];
  ACCT acct;
  HDR hdr;

  if (HAS_PERM(PERM_ALLADMIN))
    return 0;

  /* 板主在自己管理的看板不列入跨貼檢查 */
  blist = (bshm->bcache + bno)->BM;
  if (HAS_PERM(PERM_BM) && blist[0] > ' ' && is_bm(blist, cuser.userid))
    return 0;

  /* FN_ETC_NOCROSSPOST 存放允許 cross-post 的特例名單 */
  str_lower(folder, cuser.userid);
  if (acl_has(FN_ETC_NOCROSSPOST, "", folder) > 0)
    return 0;

  if (checksum_find(fpath))
  {
    /* 如果是 cross-post，那麼轉去 BN_SECURITY 並直接停權 */
    brd_fpath(folder, BN_SECURITY, fn_dir);
    hdr_stamp(folder, HDR_COPY | 'A', &hdr, fpath);
    strcpy(hdr.owner, cuser.userid);
    strcpy(hdr.nick, cuser.username);
    sprintf(hdr.title, "%s %s Cross-Post", cuser.userid, Now());
    rec_bot(folder, &hdr, sizeof(HDR));
    btime_update(brd_bno(BN_SECURITY));

    bbstate &= ~STAT_POST;
    cuser.userlevel &= ~PERM_POST;
    cuser.userlevel |= PERM_DENYPOST;
    if (acct_load(&acct, cuser.userid) >= 0)
    {
      acct.tvalid = time4(NULL) + CROSSPOST_DENY_DAY * 86400;
      acct_setperm(&acct, PERM_DENYPOST, PERM_POST);
    }
    board_main();
    mail_self(FN_ETC_CROSSPOST, str_sysop, "Cross-Post 停權", 0);
    vmsg("您因為過度 Cross-Post 已被停權");
    return 1;
  }
  return 0;
}
#endif	/* HAVE_DETECT_CROSSPOST */


/* ----------------------------------------------------- */
/* 發表、回應、編輯、轉錄文章				 */
/* ----------------------------------------------------- */


#ifdef HAVE_ANONYMOUS
static void
log_anonymous(fname)
  char *fname;
{
  char buf[512];

  sprintf(buf, "%s %-13s(%s)\n%-13s %s %s\n",
    Now(), cuser.userid, fromhost, currboard, fname, ve_title);
  f_cat(FN_RUN_ANONYMOUS, buf);
}
#endif


#ifdef HAVE_UNANONYMOUS_BOARD
static void
do_unanonymous(fpath)
  char *fpath;
{
  HDR hdr;
  char folder[64];

  brd_fpath(folder, BN_UNANONYMOUS, fn_dir);
  hdr_stamp(folder, HDR_LINK | 'A', &hdr, fpath);

  strcpy(hdr.owner, cuser.userid);
  strcpy(hdr.title, ve_title);

  rec_bot(folder, &hdr, sizeof(HDR));
  btime_update(brd_bno(BN_UNANONYMOUS));
}
#endif


static void
copy_post_IAS(hdr, fpath)
  HDR *hdr;
  char *fpath;
{
  char folder[64];
  HDR post;
  BRD *brdp, *bend;

  brdp = bshm->bcache;
  bend = brdp + bshm->number;

  while (brdp < bend)
  {
    if (brdp->battr & BRD_IAS)
    {
      brd_fpath(folder, brdp->brdname, fn_dir);
      hdr_stamp(folder, HDR_COPY | 'A', &post, fpath);
      memcpy(post.owner, hdr->owner, TTLEN + 140);
      rec_bot(folder, &post, sizeof(HDR));
    }
    brdp++;
  }
}

void
do_ias_post_log(hdr)
  HDR hdr;
{
  char fpath[64];
  char buf[512];
  
  if( ( (currbattr & BRD_IAS) || (strstr(currboard, "IS_")) || 
        (strstr(currboard, "IA_")) || (strstr(currboard, "IAS_")) ) && 
        (bbstate & STAT_BM) )
  {
    sprintf(buf, "時間:%s\n作者:%s 看板:%s\n標題:%s\n檔名:%s\n--\n\n", Now(), cuser.userid, currboard, hdr.title, hdr.xname);
    f_cat("run/IAS_POLOG", buf);
    sprintf(fpath, "gem/brd/IAS_Meeting/@/@IAS_POLOG");
    f_cat(fpath, buf);
  }
}

static int
do_post(xo, title)
  XO *xo;
  char *title;
{
  /* Thor.981105: 進入前需設好 curredit 及 quote_file */
  HDR hdr;
  char fpath[64], *folder, *nick, *rcpt;
  int mode = -1, value;
  time_t spendtime;

  if (!(bbstate & STAT_POST))
  {
#ifdef NEWUSER_LIMIT
    if (cuser.lastlogin - cuser.firstlogin < 3 * 86400)
      vmsg("新手上路，三日後始可張貼文章");
    else
#endif
      vmsg("對不起，您沒有在此發表文章的權限");
    pcurrhdr = NULL;
    return XO_FOOT;
  }

  film_out(FILM_POST, 0);

  move(19, 0);
  prints("發表文章於【 %s 】看板", currboard);

#ifdef POST_PREFIX
  /* 借用 mode、rcpt、fpath */

  if (title)
  {
    rcpt = NULL;
  }
  else		/* itoc.020113: 新文章選擇標題分類 */
  {
    if (!(currbattr & BRD_NOPREFIX))
    {
      FILE *fp;
      int len = 6, pnum, newline = 0;
      char prefix[NUM_PREFIX][16];
      char *prefix_default[NUM_PREFIX] = DEFAULT_PREFIX;

      for (mode = 0; mode < NUM_PREFIX; mode++)
	strcpy(prefix[mode], prefix_default[mode]);

      move(21, 0);
      prints("類別：");

      brd_fpath(fpath, currboard, "prefix.new");
      if (fp = fopen(fpath, "r"))
      {
	/* 載入設定檔 */
	for (mode = 0; mode < NUM_PREFIX; mode++)
	{
	  if (!fgets(fpath, 14, fp))
	    break;
	  if (strlen(fpath) == 1)	/* '\n' */
	    break;
	  fpath[strlen(fpath) - 1] = '\0';
	  strcpy(prefix[mode], fpath);
	  if ((len + 3 + strlen(fpath)) >= 70)	/* 換行列印 */
	  {
	    move(22, 0);
	    prints("      ");
	    len = 6;
	    newline = 1;
	  }
	  prints("%d.%s ", mode + 1, fpath);
	  len += (3 + strlen(fpath));
	  pnum = mode + 1;
	}
	fclose(fp);
      }
      else	/* 沒有設定檔, 使用預設 */
      {
	pnum = NUM_PREFIX;
	for (mode = 0; mode < NUM_PREFIX; mode++)
	{
	  prints("%d.%s ", mode + 1, prefix[mode]);
	  len += (3 + strlen(prefix[mode]));
	}
      }

      mode = vget(21 + newline, len, "", fpath, 3, DOECHO) - '1';
      if (mode >= 0 && mode < pnum)	/* 輸入數字選項 */
      {
	if (prefix[mode][0] == '[' ||
	    !strncmp(prefix[mode], "【", 2) ||
	    !strncmp(prefix[mode], "《", 2) ||
	    !strncmp(prefix[mode], "〈", 2))
	  sprintf(fpath, "%s ", prefix[mode]);
	else
	  sprintf(fpath, "[%s] ", prefix[mode]);

	rcpt = fpath;
      }
      else				/* 空白跳過 */
	rcpt = NULL;

      move(20, 0);
      clrtobot();
    }
    else	/* 看板設定不使用文章類別 */
      rcpt = NULL;
  }

  if (!ve_subject(21, title, rcpt))
#else
  if (!ve_subject(21, title, NULL))
#endif
  {
    pcurrhdr = NULL;
    return XO_HEAD;
  }

  /* 未具備 Internet 權限者，只能在站內發表文章 */
  /* Thor.990111: 沒轉信出去的看板, 也只能在站內發表文章 */

  if (!HAS_PERM(PERM_INTERNET) || (currbattr & BRD_NOTRAN))
    curredit &= ~EDIT_OUTGO;

  utmp_mode(M_POST);
  fpath[0] = '\0';
#ifdef HAVE_TEMPLATE
  if (mode >= 0 && mode < NUM_PREFIX)
  {
    char buf1[32], buf2[64];
    sprintf(buf1, "prefix/template_%d", mode + 1);
    brd_fpath(buf2, currboard, buf1);
    if (dashf(buf2))
    {
      sprintf(fpath, "tmp/%s.%d", cuser.userid, time(0));
      f_cp(buf2, fpath, O_TRUNC);
    }
  }
#endif
  time(&spendtime);
  if (vedit(fpath, 1) < 0)
  {
    unlink(fpath);
    vmsg(msg_cancel);
    pcurrhdr = NULL;
    return XO_HEAD;
  }

#ifdef DO_POST_FILTER
  strcpy(bbs_dog_title, ve_title);
  if (post_filter(fpath))	/* smiler.080830: 針對文章標題內容偵測有無不當之處 */
  {
    unlink(fpath);
    pcurrhdr = NULL;
    return XO_HEAD;
  }
#endif

  spendtime = time(0) - spendtime;	/* itoc.010712: 總共花的時間(秒數) */

  if (currbattr & BRD_NOCOUNT || wordsnum < 30)
  {				/* itoc.010408: 以此減少灌水現象 */
    value = 0;
  }
  else
  {
    /* itoc.010408: 依文章長度/所費時間來決定要給多少錢；幣制才會有意義 */
    value = BMIN(wordsnum, spendtime) / 10;	/* 每十字/秒 一元 */
  }

  /* build filename */

  folder = xo->dir;
  hdr_stamp(folder, HDR_LINK | 'A', &hdr, fpath);

  /* set owner to anonymous for anonymous board */

#ifdef HAVE_ANONYMOUS
  /* Thor.980727: lkchu新增之[簡單的選擇性匿名功能] */
  if (curredit & EDIT_ANONYMOUS)
  {
    rcpt = anonymousid;	/* itoc.010717: 自定匿名 ID */
    nick = STR_ANONYMOUS;
    value = cuser.userno;

    /* Thor.980727: lkchu patch: log anonymous post */
    /* Thor.980909: gc patch: log anonymous post filename */
    log_anonymous(hdr.xname);

#ifdef HAVE_UNANONYMOUS_BOARD
    do_unanonymous(fpath);
#endif
  }
  else
#endif
  {
    rcpt = cuser.userid;
    nick = cuser.username;
  }
  title = ve_title;
  mode = (curredit & EDIT_OUTGO) ? POST_OUTGO : 0;
#ifdef HAVE_REFUSEMARK
  if (curredit & EDIT_RESTRICT)
    mode |= POST_RESTRICT;
#endif

  hdr.xmode = mode;
  sprintf(hdr.value, "%d", value);
  strcpy(hdr.owner, rcpt);
  strcpy(hdr.nick, nick);
  strcpy(hdr.title, title);

  rec_bot(folder, &hdr, sizeof(HDR));
  btime_update(currbno);

  if (mode & POST_OUTGO)
    outgo_post(&hdr, currboard);

  post_history(xo, &hdr);

#if 1
  /* smiler.080820: 依站務要求改轉文至 nthu.forsale */
  if (strstr(hdr.title, "賣") || strstr(hdr.title, "售") || strstr(hdr.title, "出清"))
  {
    /* smiler.080820: 依站務要求改轉文至 nthu.forsale */
    /* smiler.080705: 依站務要求改轉文至 forsale */
    /* smiler.070916: for 轉文至 nthu.forsale */
    HDR  hdr2;
    char fpath2[64], folder2[64];	// smiler.070916
    char board_from[30];		// smiler.070916

    //if( (!strstr(board_from,"P_")) && (!strstr(board_from,"R_")) &&
    //   (!strstr(board_from,"LAB_")) && (!strstr(board_from,"G_")) &&
    //   (!strstr(board_from,"deleted")) && (!strstr(board_from,"junk")) &&
    //   (!strstr(board_from,BN_DELLOG)) && (!strstr(board_from,BN_EDITLOG)) )

    /* smiler.080820: 依站務要求僅 nctu nthu 轉買賣文至 nthu.forsale */
    if ((!strcmp(board_from, "nctu")) || (!strcmp(board_from, "nthu")))
    {
      strcpy(fpath2, fpath);			// smiler.070916
      brd_fpath(folder2, "nthu.forsale", FN_DIR);
      //brd_fpath(folder2, "forsale", FN_DIR);	// smiler.080705
      hdr_stamp(folder2, HDR_LINK | 'A', &hdr2, fpath2);	// smiler.070916
      strcpy(board_from, currboard);		// smiler.070916

      hdr2.xmode = mode;	// smiler.070916
      strcpy(hdr2.owner, rcpt);	// smiler.070916
      strcpy(hdr2.nick, nick);	// smiler.070916
      strcpy(hdr2.title, title);	// smiler.070916

      rec_bot(folder2, &hdr2, sizeof(HDR));
      btime_update(brd_bno("nthu.forsale"));
      //btime_update(brd_bno("forsale"));
    }
  }
#endif

  clear();
  outs("順利貼出文章，");
  
  /* smiler.090120: 藝文館板主po文記錄 */
  do_ias_post_log(hdr);

  if ((currbattr & BRD_NOCOUNT) || (curredit & EDIT_ANONYMOUS) || (wordsnum < 30))
  {				/* itoc.010408: 以此減少灌水現象 */
    outs("文章不列入紀錄，敬請包涵。");
  }
  else
  {
    prints("這是您的第 %d 篇文章，得 %d 銀。", ++cuser.numposts, value);
    addmoney(value);
  }

  /* 回應到原作者信箱 */

  if (curredit & EDIT_BOTH)
  {
    rcpt = quote_user;

    if (strchr(rcpt, '@'))	/* 站外 */
      mode = bsmtp(fpath, title, rcpt, 0);
    else			/* 站內使用者 */
      mode = mail_him(fpath, rcpt, title, 0);

    outs(mode >= 0 ? "\n\n成功\回應至作者信箱" : "\n\n作者無法收信");
  }

  /* smiler.080705:自動貼文至各藝文館 */
  if (!strcmp(currboard, "IAS_Announce") && HAS_PERM(PERM_ALLADMIN))
    copy_post_IAS(&hdr, fpath);

  unlink(fpath);
  pcurrhdr = NULL;

  vmsg(NULL);

  return XO_INIT;
}


int
do_reply(xo, hdr)
  XO *xo;
  HDR *hdr;
{
  curredit = 0;

  pcurrhdr = hdr;
  switch (vans("▲ 回應至 (F)看板 (M)作者信箱 (B)二者皆是 (Q)取消？[F] "))
  {
  case 'm':
    hdr_fpath(quote_file, xo->dir, hdr);
    return do_mreply(hdr, 0);

  case 'q':
    return XO_FOOT;

  case 'b':
    /* 若無寄信的權限，則只回看板 */
    if (HAS_PERM(strchr(hdr->owner, '@') ? PERM_INTERNET : PERM_LOCAL))
      curredit = EDIT_BOTH;
    break;
  }

  /* Thor.981105: 不論是轉進的, 或是要轉出的, 都是別站可看到的, 所以回信也都應該轉出 */
  if (hdr->xmode & (POST_INCOME | POST_OUTGO))
    curredit |= EDIT_OUTGO;

  hdr_fpath(quote_file, xo->dir, hdr);
  strcpy(quote_user, hdr->owner);
  strcpy(quote_nick, hdr->nick);
  return do_post(xo, hdr->title);
}


#ifdef HAVE_REFUSEMARK
int
chkrestrict(hdr)
  HDR *hdr;
{
#if 0
  return !(hdr->xmode & POST_RESTRICT) ||
    !strcmp(hdr->owner, cuser.userid) || (bbstate & STAT_BM);
#endif
  return !(hdr->xmode & POST_RESTRICT) || RefusePal_belong(currboard, hdr);
}
#endif


int
chkrescofo(hdr)	/* restrict / copy / forward */
  HDR *hdr;
{
  return
#ifdef HAVE_REFUSEMARK
    chkrestrict(hdr) &&
#endif
    !(hdr->xmode & POST_NOFORWARD) && (!(currbattr & BRD_NOFORWARD) || (bbstate & STAT_BM));
}


#ifdef SYSOP_MBOX_BRD
int sysop_reply = 0;


static int
post_sysop_reply(xo, hdr)
  XO *xo;
  HDR *hdr;
{
  char fpath[64];

  hdr_fpath(fpath, xo->dir, hdr);
  strcpy(quote_file, fpath);
  sysop_reply = 1;
  do_mreply(hdr, 1);
  sysop_reply = 0;
  rec_put(xo->dir, hdr, sizeof(HDR), xo->pos, NULL);
  btime_update(brd_bno(currboard));
  return XO_HEAD;
}


static int
post_sysop_send(xo)
  XO *xo;
{
  sysop_reply = 1;
  m_send();
  sysop_reply = 0;
  btime_update(brd_bno(currboard));
  return XO_HEAD;
}
#endif


static int
post_reply(xo)
  XO *xo;
{
  if (bbstate & STAT_POST)
  {
    HDR *hdr;

    hdr = (HDR *) xo_pool + (xo->pos - xo->top);

#ifdef SYSOP_MBOX_BRD
    if (!strcmp(currboard, BN_SYSOPMBOX))
      return post_sysop_reply(xo, hdr);
#endif

#ifdef HAVE_REFUSEMARK
    if (!chkrestrict(hdr))
      return XO_NONE;
#endif

    return do_reply(xo, hdr);
  }
  return XO_NONE;
}


static int
post_add(xo)
  XO *xo;
{
#ifdef SYSOP_MBOX_BRD
  if (!strcmp(currboard, BN_SYSOPMBOX))
    return post_sysop_send(xo);
#endif

  curredit = EDIT_OUTGO;
  *quote_file = '\0';
  return do_post(xo, NULL);
}


/* ----------------------------------------------------- */
/* 印出 hdr 標題					 */
/* ----------------------------------------------------- */


int
tag_char(chrono)
  int chrono;
{
  return TagNum && !Tagger(chrono, 0, TAG_NIN) ? '*' : ' ';
}


#ifdef HAVE_DECLARE
inline int
cal_day(date)		/* itoc.010217: 計算星期幾 */
  char *date;
{
#if 0
   蔡勒公式是一個推算哪一天是星期幾的公式.
   這公式是:
         c                y       26(m+1)
    W= [---] - 2c + y + [---] + [---------] + d - 1
         4                4         10
    W → 為所求日期的星期數. (星期日: 0  星期一: 1  ...  星期六: 6)
    c → 為已知公元年份的前兩位數字.
    y → 為已知公元年份的後兩位數字.
    m → 為月數
    d → 為日數
   [] → 表示只取該數的整數部分 (地板函數)
    ps.所求的月份如果是1月或2月,則應視為上一年的13月或14月.
       所以公式中m的取值範圍不是1到12,而是3到14
#endif

  /* 適用 2000/03/01 至 2099/12/31 */

  int y, m, d;

  y = 10 * ((int) (date[0] - '0')) + ((int) (date[1] - '0'));
  d = 10 * ((int) (date[6] - '0')) + ((int) (date[7] - '0'));
  if (date[3] == '0' && (date[4] == '1' || date[4] == '2'))
  {
    y -= 1;
    m = 12 + (int) (date[4] - '0');
  }
  else
  {
    m = 10 * ((int) (date[3] - '0')) + ((int) (date[4] - '0'));
  }
  return (-1 + y + y / 4 + 26 * (m + 1) / 10 + d) % 7;
}
#endif


void
hdr_outs(hdr, cc)		/* print HDR's subject */
  HDR *hdr;
  int cc;			/* 印出最多 cc - 1 字的標題 */
{
  /* 回覆/轉錄/原創/閱讀中的同主題回覆/閱讀中的同主題轉錄/閱讀中的同主題原創 */
  static char *type[6] = {"Re", "Fw", "◇", "\033[1;33m=>", "\033[1;33m->", "\033[1;32m◆"};
  uschar *title, *mark;
  int ch, len;
  int in_chi;		/* 1: 在中文字中 */
#ifdef HAVE_DECLARE
  int square;		/* 1: 要處理方括 */
#endif
#ifdef CHECK_ONLINE
  UTMP *online;
#endif

  /* --------------------------------------------------- */
  /* 印出日期						 */
  /* --------------------------------------------------- */

#ifdef HAVE_DECLARE
  /* itoc.010217: 改用星期幾來上色 */
  /*smiler.070724: 日期印出前,多空一格*/
  prints(" \033[1;3%dm%s\033[m ", cal_day(hdr->date) + 1, hdr->date + 3);

#else
  prints(" %s ", hdr->date + 3);
#endif

  /* --------------------------------------------------- */
  /* 印出作者						 */
  /* --------------------------------------------------- */

#ifdef CHECK_ONLINE
  if (online = utmp_seek(hdr))
    outs(COLOR7);
#endif

  mark = hdr->owner;
  len = IDLEN + 1;
  in_chi = 0;

  while (ch = *mark)
  {
    if (--len <= 0)
    {
      /* 把超過 len 長度的部分直接切掉 */
      /* itoc.060604.註解: 如果剛好切在中文字的一半就會出現亂碼，不過這情況很少發生，所以就不管了 */
      ch = '.';
    }
    else
    {
      /* 站外的作者把 '@' 換成 '.' */
      if (in_chi || IS_ZHC_HI(ch))	/* 中文字尾碼是 '@' 的不算 */
	in_chi ^= 1;
      else if (ch == '@')
	ch = '.';
    }

    outc(ch);

    if (ch == '.')
      break;

    mark++;
  }

  while (len--)
    outc(' ');

#ifdef CHECK_ONLINE
  if (online)
    outs(str_ransi);
#endif

  /* --------------------------------------------------- */
  /* 印出標題的種類					 */
  /* --------------------------------------------------- */

  /* len: 標題是 type[] 裡面的那一種 */
#ifdef HAVE_REFUSEMARK
  if (!chkrestrict(hdr))
  {
    title = "<< 文章保密 >>";
    len = 2;
  }
  else
#endif
  {
    title = str_ttl(mark = hdr->title);
    len = (title == mark) ? 2 : (*mark == 'R') ? 0 : 1;
    if (!strcmp(currtitle, title))
      len += 3;
  }
  outs(type[len]);
  outc(' ');


  /* --------------------------------------------------- */
  /* 印出標題						 */
  /* --------------------------------------------------- */

  mark = title + cc;

#ifdef HAVE_DECLARE	/* Thor.980508: Declaration, 嘗試使某些title更明顯 */
  square = in_chi = 0;
  if (len < 3)
  {
    if (*title == '[')
    {
      outs("\033[1m");
      square = 1;
    }
  }
#endif

  /* 把超過 cc 長度的部分直接切掉 */
  /* itoc.060604.註解: 如果剛好切在中文字的一半就會出現亂碼，不過這情況很少發生，所以就不管了 */
  while ((ch = *title++) && (title < mark - 3))
  {
#ifdef HAVE_DECLARE
    if (square)
    {
      if (in_chi || IS_ZHC_HI(ch))	/* 中文字的第二碼若是 ']' 不算是方括 */
      {
	in_chi ^= 1;
      }
      else if (ch == ']')
      {
	outs("]\033[m");
	square = 0;			/* 只處理一組方括，方括已經處理完了 */
	continue;
      }
    }
#endif

    outc(ch);
  }

  if (title == mark - 3)	/* 還沒印完 */
  {
    if (strlen(title) < 4)
    {
      outc(ch);
      while (ch = *title++)
	outc(ch);
    }
    else
    {
      if (in_chi)
	prints("%c...", ch);
      else
	outs(" ...");
    }
  }

#ifdef HAVE_DECLARE
  if (square || len >= 3)	/* Thor.980508: 變色還原用 */
#else
  if (len >= 3)
#endif
    outs("\033[m");

  outc('\n');
}


#ifdef HAVE_LIGHTBAR
void
hdr_outs_bar(hdr, cc)	/* print HDR's subject */
  HDR *hdr;
  int cc;		/* 印出最多 cc - 1 個字 */
{
  /* 回覆/轉錄/原創/閱讀中的同主題回覆/閱讀中的同主題轉錄/閱讀中的同主題原創 */
  static char *type[6] = {"Re", "Fw", "◇", "\033[1;33m=>", "\033[1;33m->", "\033[1;32m◆"};
  uschar *title, *mark;
  int ch, len;
  int in_chi;		/* 1: 在中文字中 */
#ifdef HAVE_DECLARE
  int square;		/* 1: 要處理方括 */
#endif
#ifdef CHECK_ONLINE
  UTMP *online;
#endif

  /* --------------------------------------------------- */
  /* 印出日期						 */
  /* --------------------------------------------------- */

#ifdef HAVE_DECLARE
  /* itoc.010217: 改用星期幾來上色 */
  /*smiler.070724: 日期印出前,多空一格*/
  prints("%s \033[1;3%dm%s\033[m%s ",
    UCBAR[UCBAR_POST], cal_day(hdr->date) + 1, hdr->date + 3, UCBAR[UCBAR_POST]);
#else
  prints(" %s ", hdr->date + 3);
#endif

  /* --------------------------------------------------- */
  /* 印出作者						 */
  /* --------------------------------------------------- */

#ifdef CHECK_ONLINE
  if (online = utmp_seek(hdr))
    outs(COLOR7);
#endif

  mark = hdr->owner;
  len = IDLEN + 1;
  in_chi = 0;

  while (ch = *mark)
  {
    if (--len <= 0)
    {
      /* 把超過 len 長度的部分直接切掉 */
      /* itoc.060604.註解: 如果剛好切在中文字的一半就會出現亂碼，不過這情況很少發生，所以就不管了 */
      ch = '.';
    }
    else
    {
      /* 站外的作者把 '@' 換成 '.' */
      if (in_chi || IS_ZHC_HI(ch))	/* 中文字尾碼是 '@' 的不算 */
	in_chi ^= 1;
      else if (ch == '@')
	ch = '.';
    }

    outc(ch);

    if (ch == '.')
      break;

    mark++;
  }

  while (len--)
    outc(' ');

#ifdef CHECK_ONLINE
  if (online)
    outs(str_ransi);
#endif

  /* --------------------------------------------------- */
  /* 印出標題的種類					 */
  /* --------------------------------------------------- */

  /* len: 標題是 type[] 裡面的那一種 */
#ifdef HAVE_REFUSEMARK
  if (!chkrestrict(hdr))
  {
    title = "<< 文章保密 >>";
    len = 2;
  }
  else
#endif
  {
    title = str_ttl(mark = hdr->title);
    len = (title == mark) ? 2 : (*mark == 'R') ? 0 : 1;
    if (!strcmp(currtitle, title))
      len += 3;
  }
  outs(UCBAR[UCBAR_POST]);
  outs(type[len]);
  outs(UCBAR[UCBAR_POST]);
  outc(' ');

  /* --------------------------------------------------- */
  /* 印出標題						 */
  /* --------------------------------------------------- */

  mark = title + cc;

#ifdef HAVE_DECLARE	/* Thor.980508: Declaration, 嘗試使某些title更明顯 */
  square = in_chi = 0;
  if (len < 3)
  {
    if (*title == '[')
    {
      outs("\033[1m");
      square = 1;
    }
  }
#endif

  outs(UCBAR[UCBAR_POST]);

  /* 把超過 cc 長度的部分直接切掉 */
  /* itoc.060604.註解: 如果剛好切在中文字的一半就會出現亂碼，不過這情況很少發生，所以就不管了 */
  while ((ch = *title++) && (title < mark - 3))
  {
#ifdef HAVE_DECLARE
    if (square)
    {
      if (in_chi || IS_ZHC_HI(ch))	/* 中文字的第二碼若是 ']' 不算是方括 */
      {
	in_chi ^= 1;
      }
      else if (ch == ']')
      {
	prints("]\033[m%s", UCBAR[UCBAR_POST]);
	square = 0;			/* 只處理一組方括，方括已經處理完了 */
	continue;
      }
    }
#endif

    outc(ch);
  }

  if (ch && (title == mark - 3))	/* 還沒印完 */
  {

    if (strlen(title) < 4)
    {
      outc(ch);
      while (ch = *title++)
	outc(ch);
    }
    else
    {
      if (in_chi)
	prints("%c...", ch);
      else
	outs(" ...");
      title += 4;
    }
  }

  title--;	/* 考慮 (title == mark - 3) 時剛好印完的情形(*title == '\0'), 要補回來 */
  while(title++ < mark)
    outc(' ');

  outs("\033[m");
}
#endif


/* ----------------------------------------------------- */
/* 看板功能表						 */
/* ----------------------------------------------------- */


static int post_body();
static int post_head();


static int
post_init(xo)
  XO *xo;
{
  xo_load(xo, sizeof(HDR));
  return post_head(xo);
}


static int
post_load(xo)
  XO *xo;
{
  xo_load(xo, sizeof(HDR));
  return post_body(xo);
}


#ifdef SYSOP_MBOX_BRD
static char*
sysop_mbox_attr(hdr, attr, unread)
  HDR *hdr;
  int attr, unread;
{
  int mode;
  char attr_tmp[15] = {0};

  mode = hdr->xmode;

  if (mode & MAIL_REPLIED)
  {
    attr = (mode & MAIL_MARKED) ? 'R' : 'r';
    strcpy(attr_tmp, "\033[1;31m");
  }
  else if (mode & MAIL_MARKED)
  {
    attr |= 'M';
    strcpy(attr_tmp, "\033[1;36m");
  }
  else if (!attr)
  {
    attr = '+';
    strcpy(attr_tmp, "");
  }

  if (unread)
  {
    if (attr == 'm')
      attr = '=';
    else if (!(mode & POST_BOTTOM) &&
      (!strcmp(hdr->owner, cuser.userid) || (bbstate & STAT_BM)))
      attr = '~';
  }

  static char color_attr[30];
  sprintf(color_attr, "%s%c\033[m", attr_tmp, attr);
  return color_attr;
}
#endif


static char*
post_attr(hdr)
  HDR *hdr;
{
  int mode, attr, read, unread;

  char attr_tmp[15];
  attr_tmp[0] = '\0';

  mode = hdr->xmode;

  /* 已閱讀為小寫，未閱讀為大寫 */
  /* 由於置底文沒有閱讀記錄，所以視為已讀 */
  /* 加密文章視為已讀 */
  read = (USR_SHOW & USR_SHOW_POST_MODIFY_UNREAD) ? !brh_unread(hdr->chrono) : !brh_unread(BMAX(hdr->chrono, hdr->stamp));
#ifdef HAVE_REFUSEMARK
  attr = ((mode & POST_BOTTOM) || read ||
    ((mode & POST_RESTRICT) && strcmp(hdr->owner, cuser.userid) && !(bbstate & STAT_BM))) ? 0x20 : 0;
#else
  attr = ((mode & POST_BOTTOM) || read ? 0x20 : 0;
#endif

  unread = ((USR_SHOW & USR_SHOW_POST_MODIFY_UNREAD) && attr && brh_unread(BMAX(hdr->chrono, hdr->stamp))) ? 1 : 0;

#ifdef SYSOP_MBOX_BRD
  if (!strcmp(currboard, BN_SYSOPMBOX))
    return sysop_mbox_attr(hdr, attr, unread);
#endif

#ifdef HAVE_REFUSEMARK
  if ((mode & POST_RESTRICT) && (RefusePal_level(currboard, hdr) == 1) && (USR_SHOW & USR_SHOW_POST_ATTR_RESTRICT_F))
  {
    attr |= 'F',
    strcpy(attr_tmp, "\033[1;33m");
  }
  else if ((mode & POST_RESTRICT) && (RefusePal_level(currboard, hdr) == -1) && (USR_SHOW & USR_SHOW_POST_ATTR_RESTRICT))
  {
    attr |= 'L';
    strcpy(attr_tmp, "\033[1;34m");
  }
  else
#endif
  if ((bbstate & STAT_BOARD) && (mode & POST_GEM) && (mode & POST_MARKED) && (USR_SHOW & USR_SHOW_POST_ATTR_GEM_MARKED))   /* 板主才看得到 G/B */
  {
    attr |= 'B';	/* 若有 mark+gem，顯示 B */
    strcpy(attr_tmp, "\033[1;31m");
  }
  else if ((bbstate & STAT_BOARD) && (mode & POST_GEM) && (!(mode & POST_MARKED)) && (USR_SHOW & USR_SHOW_POST_ATTR_GEM))
  {
    attr |= 'G';
    strcpy(attr_tmp, "\033[1;35m");
  }
  else
#ifdef HAVE_LABELMARK
  if ((mode & POST_DELETE) && (USR_SHOW & USR_SHOW_POST_ATTR_DELETE))
  {
    attr |= 'T';
    strcpy(attr_tmp, "\033[1;32m");
  }
  else
#endif
  if ((mode & POST_NOFORWARD) && (USR_SHOW & USR_SHOW_POST_ATTR_NOFORWARD))
  {
    attr |= 'X';
    strcpy(attr_tmp, "\033[1;34m");
  }
  else if ((mode & POST_NOSCORE) && (USR_SHOW & USR_SHOW_POST_ATTR_NOSCORE))
  {
    attr |= 'N';
    strcpy(attr_tmp, "\033[1;34m");
  }
  else if ((mode & POST_MARKED) && (USR_SHOW & USR_SHOW_POST_ATTR_MARKED))
  {
    attr |= 'M';
    strcpy(attr_tmp, "\033[1;36m");
    if (mode & POST_GOOD)
      strcpy(attr_tmp, "\033[1;33m");
  }
  else if (!attr)
  {
    attr = '+';
    strcpy(attr_tmp, "");
  }

  if (unread)
  {
    if (attr == 'm' || attr == 'b')
      attr = '=';
    else if (!(mode & POST_BOTTOM) &&
      (attr != 'l' || (!strcmp(hdr->owner, cuser.userid) || (bbstate & STAT_BM))) )
      attr = '~';
  }

  static char color_attr[30];
  sprintf(color_attr, "%s%c\033[m", attr_tmp, attr);
  return color_attr;
}


static void
post_item(num, hdr)
  int num;
  HDR *hdr;
{
#ifdef HAVE_SCORE
  if (hdr->xmode & POST_BOTTOM)
    prints("  \033[1;33m重要\033[m%c%s", tag_char(hdr->chrono), post_attr(hdr));
  else
    prints("%6d%c%s", (hdr->xmode & POST_BOTTOM) ? -1 : num, tag_char(hdr->chrono), post_attr(hdr));
  if ((hdr->xmode & POST_SCORE) && (USR_SHOW & USR_SHOW_POST_SCORE))
  {
    num = hdr->score;
    if (!num && (!(USR_SHOW & USR_SHOW_POST_SCORE_0)))
      outs("  ");
    else if (num <= 99 && num >= -99)
      prints("\033[1;3%cm%2d\033[m",
	num > 0 ? '1' : num < 0 ? '2' : '0', abs(num));
    else
      prints("\033[1;3%s\033[m", num >= 0 ? "1m爆" : "2m噓");
 }
 else
 {
   outs("  ");
 }

  hdr_outs(hdr, d_cols + 45);	/* 少一格來放分數 */
#else
 if (hdr->xmode & POST_BOTTOM)
   prints("  \033[1;33m重要\033[m%c%s", tag_char(hdr->chrono), post_attr(hdr));
 else
   prints("%6d%c%s ", (hdr->xmode & POST_BOTTOM) ? -1 : num, tag_char(hdr->chrono), post_attr(hdr));

 hdr_outs(hdr, d_cols + 47);
#endif
}


#ifdef HAVE_LIGHTBAR
static int
post_item_bar(xo, mode)
  XO *xo;
  int mode;
{
  HDR *hdr;
  int num;

#ifdef HAVE_SCORE
  hdr = (HDR *) xo_pool + xo->pos - xo->top;
  num = xo->pos + 1;

  if (hdr->xmode & POST_BOTTOM)
  {
    prints("%s%s%s%c%s%s",
      mode ? UCBAR[UCBAR_POST] : "",
      "  \033[1;33m重要\033[m", mode ? UCBAR[UCBAR_POST] : "",
      tag_char(hdr->chrono), post_attr(hdr), mode ? UCBAR[UCBAR_POST] : "");
  }
  else
  {
    prints("%s%6d%c%s%s",
      mode ? UCBAR[UCBAR_POST] : "",
      num, tag_char(hdr->chrono), post_attr(hdr), mode ? UCBAR[UCBAR_POST] : "");
  }

  if ((hdr->xmode & POST_SCORE) && (USR_SHOW & USR_SHOW_POST_SCORE))
  {
    num = hdr->score;
    if (!num && !(USR_SHOW & USR_SHOW_POST_SCORE_0))
      outs("  ");
    else if (num <= 99 && num >= -99)
      prints("%s\033[1;3%cm%s%2d\033[m%s",
	mode ? UCBAR[UCBAR_POST] : "", num > 0 ? '1' : num < 0 ? '2' : mode ? '7' : '0',
	mode ? UCBAR[UCBAR_POST] : "", abs(num), mode ? UCBAR[UCBAR_POST] : "");
    else
      prints("%s\033[1;3%s\033[m%s", mode ? UCBAR[UCBAR_POST] : "", num >= 0 ? "1m爆" : "2m噓",
	mode ? UCBAR[UCBAR_POST] : "");
  }
  else
  {
    outs("  ");
  }

  if (mode)
    hdr_outs_bar(hdr, d_cols + 45);	/* 少一格來放分數 */
  else
    hdr_outs(hdr, d_cols + 45);

#else
  hdr = (HDR *) xo_pool + xo->pos - xo->top;
  num = xo->pos + 1;
  if (hdr->xmode & POST_BOTTOM)
  {
    prints("%s%s%s%c%s%s ",
      mode ? UCBAR[UCBAR_POST] : "",
      "  \033[1;33m重要\033[m",mode ? UCBAR[UCBAR_POST] : "",
      tag_char(hdr->chrono), post_attr(hdr),
      mode ? UCBAR[UCBAR_POST] : "");
  }
  else
  {
    prints("%s%6d%c%s%s ",
      mode ? UCBAR[UCBAR_POST] : "",
      num, tag_char(hdr->chrono), post_attr(hdr), mode ? UCBAR[UCBAR_POST] : "");
  }

  if (mode)
    hdr_outs_bar(hdr, d_cols + 47);
  else
    hdr_outs(hdr, d_cols + 47);
#endif

  move(xo->pos - xo->top + 3, 0);
  return XO_NONE;
}
#endif


static int
post_body(xo)
  XO *xo;
{
  HDR *hdr;
  int num, max, tail;

  max = xo->max;
  if (max <= 0)
  {
    if (bbstate & STAT_POST)
    {
      if (vans("要新增資料嗎(Y/N)？[N] ") == 'y')
	return post_add(xo);
    }
    else
    {
      vmsg("本看板尚無文章");
    }
    return XO_QUIT;
  }

  hdr = (HDR *) xo_pool;
  num = xo->top;
  tail = num + XO_TALL;
  if (max > tail)
    max = tail;

  move(3, 0);
  do
  {
    post_item(++num, hdr++);
  } while (num < max);
  clrtobot();

  /* return XO_NONE; */
  return XO_FOOT;	/* itoc.010403: 把 b_lines 填上 feeter */
}


static int
post_head(xo)
  XO *xo;
{
  vs_head(currBM, xo->xyz);
  prints(NECKER_POST, d_cols, "", currbattr & BRD_NOSCORE ? "╳" : "○", bshm->mantime[currbno]);

  return post_body(xo);
}


/* ----------------------------------------------------- */
/* 資料之瀏覽：browse / history				 */
/* ----------------------------------------------------- */


static int
post_visit(xo)
  XO *xo;
{
  int ans, row, max;
  HDR *hdr;

  ans = vans("設定所有文章 (U)未讀 (V)已讀 (W)前已讀後未讀 (Q)取消？[Q] ");
  if (ans == 'v' || ans == 'u' || ans == 'w')
  {
    row = xo->top;
    max = xo->max - row + 3;
    if (max > b_lines)
      max = b_lines;

    hdr = (HDR *) xo_pool + (xo->pos - row);
    /* brh_visit(ans == 'w' ? hdr->chrono : ans == 'u'); */
    /* weiyu.041010: 在置底文上選 w 視為全部已讀 */
    brh_visit((ans == 'u') ? 1 : (ans == 'w' && !(hdr->xmode & POST_BOTTOM)) ? hdr->chrono : 0);

    return XO_BODY;
  }
  return XO_FOOT;
}


void
post_history(xo, hdr)	/* 將 hdr 這篇加入 brh */
  XO *xo;
  HDR *hdr;
{
  int fd;
  time4_t prev, chrono, maxchrono, next, this;
  HDR buf;

  maxchrono = BMAX(hdr->chrono, hdr->stamp);

  chrono = hdr->chrono;
  if (!brh_unread(chrono))	/* 若 hdr->chrono 已在 brh 中，就接者把 hdr->stamp 加入 brh 中 */
    chrono = maxchrono;

add_brh:
  if (!brh_unread(chrono))	/* 如果已在 brh 中，就無需動作 */
    return;

  if ((fd = open(xo->dir, O_RDONLY)) >= 0)
  {
    prev = chrono + 1;
    next = chrono - 1;

    while (read(fd, &buf, sizeof(HDR)) == sizeof(HDR))
    {
      this = BMAX(buf.chrono, buf.stamp);

      if (chrono - this < chrono - prev)
	prev = this;
      else if (this - chrono < next - chrono)
	next = this;
    }
    close(fd);

    if (prev > chrono)	/* 沒有下一篇 */
      prev = chrono;
    if (next < chrono)	/* 沒有上一篇 */
      next = chrono;

    brh_add(prev, chrono, next);
  }

  if (chrono == maxchrono)
    return;

  chrono = maxchrono;
  goto add_brh;		/* 再檢查一次 hdr->stamp */
}


static int
post_browse(xo)
  XO *xo;
{
  HDR *hdr;
  int xmode, pos, key;
  char *dir, fpath[64];
  extern int more_ip;

  dir = xo->dir;

  for (;;)
  {
    pos = xo->pos;
    hdr = (HDR *) xo_pool + (pos - xo->top);
    xmode = hdr->xmode;

#ifdef HAVE_REFUSEMARK
    if (!chkrestrict(hdr))
      return XO_NONE;
#endif

    hdr_fpath(fpath, dir, hdr);
    pcurrhdr = hdr;

    /* Thor.990204: 為考慮more 傳回值 */
    if ((key = more(fpath, FOOTER_POST)) < 0)
      break;

    pcurrhdr = NULL;
    post_history(xo, hdr);
    strcpy(currtitle, str_ttl(hdr->title));

re_key:
    switch (xo_getch(xo, key))
    {
    case XO_BODY:
      continue;

    case 'y':
    case 'r':
      if (bbstate & STAT_POST)
      {
#ifdef SYSOP_MBOX_BRD
	if (!strcmp(currboard, BN_SYSOPMBOX))
	  return post_sysop_reply(xo, hdr);
#endif
	if (do_reply(xo, hdr) == XO_INIT)	/* 有成功地 post 出去了 */
	  return post_init(xo);
      }
      break;

    case 'm':
      if ((bbstate & STAT_BOARD) && !(xmode & POST_MARKED))
      {
	/* hdr->xmode = xmode ^ POST_MARKED; */
	/* 在 post_browse 時看不到 m 記號，所以限制只能 mark */
	hdr->xmode = xmode | POST_MARKED;
	currchrono = hdr->chrono;
	rec_put(dir, hdr, sizeof(HDR), pos, cmpchrono);
      }
      break;

#ifdef HAVE_SCORE
    case 'e':
      post_e_score(xo);
      return post_init(xo);

    case '%':
      post_score(xo);
      return post_init(xo);
#endif

    case '!':
      more_ip ^= 1;
      continue;

    case '/':
      if (vget(b_lines, 0, "搜尋：", hunt, sizeof(hunt), DOECHO))
      {
	more(fpath, FOOTER_POST);
	goto re_key;
      }
      continue;

    case 'E':
      return post_edit(xo);

    case 'C':	/* itoc.000515: post_browse 時可存入暫存檔 */
      if (chkrescofo(hdr))
      {
	FILE *fp;
	if (fp = tbf_open())
	{
	  f_suck(fp, fpath);
	  fclose(fp);
	}
      }
      break;

    case 'h':
      xo_help("post");
      break;

    case 'o':
      if ((bbstate & STAT_BOARD) && !(xmode & POST_NOFORWARD))
      {
	/* hdr->xmode = xmode ^ POST_NOFORWARD; */
	/* 在 post_browse 時看不到 x 記號，所以限制只能 mark */
	hdr->xmode = xmode | POST_NOFORWARD;
	currchrono = hdr->chrono;
	rec_put(dir, hdr, sizeof(HDR), pos, cmpchrono);
      }
      break;

    }
    break;
  }

  return post_head(xo);
}


/* ----------------------------------------------------- */
/* 精華區						 */
/* ----------------------------------------------------- */


static int
post_gem(xo)
  XO *xo;
{
  int level;
  char fpath[64];

  strcpy(fpath, "gem/");
  strcpy(fpath + 4, xo->dir);

  level = 0;
  if (bbstate & STAT_BOARD)
    level ^= GEM_W_BIT;
  if (HAS_PERM(PERM_SYSOP))
    level ^= GEM_X_BIT;
  if (bbstate & STAT_BM)
    level ^= GEM_M_BIT;

  XoGem(fpath, "精華區", level);
  return XO_INIT;
}


/* ----------------------------------------------------- */
/* 進板畫面						 */
/* ----------------------------------------------------- */


static int
post_memo(xo)
  XO *xo;
{
  char fpath[64];

  brd_fpath(fpath, currboard, fn_note);
  /* Thor.990204: 為考慮more 傳回值 */
  if (more(fpath, NULL) < 0)
  {
    vmsg("本看板尚無「進板畫面」");
    return XO_FOOT;
  }

  return post_head(xo);
}


/* ----------------------------------------------------- */
/* 功能：tag / switch / cross / forward			 */
/* ----------------------------------------------------- */


static int
post_tag(xo)
  XO *xo;
{
  HDR *hdr;
  int tag, pos, cur;

  pos = xo->pos;
  cur = pos - xo->top;
  hdr = (HDR *) xo_pool + cur;

  if (xo->key == XZ_XPOST)
    pos = hdr->xid;

  if (tag = Tagger(hdr->chrono, pos, TAG_TOGGLE))
  {
    move(3 + cur, 6);
    outc(tag > 0 ? '*' : ' ');
  }

  /* return XO_NONE; */
  return xo->pos + 1 + XO_MOVE; /* lkchu.981201: 跳至下一項 */
}


static int
post_switch(xo)
  XO *xo;
{
  int bno;
  BRD *brd;
  char bname[BNLEN + 1];

  if (brd = ask_board(bname, BRD_R_BIT, NULL))
  {
    if ((bno = brd - bshm->bcache) >= 0 && currbno != bno)
    {
      XoPost(bno);
      return XZ_POST;
    }
  }
  else
  {
    vmsg(err_bid);
  }
  return post_head(xo);
}


int
post_cross(xo)
  XO *xo;
{
  /* 來源看板 */
  char *dir, *ptr;
  HDR *hdr, xhdr;

  /* 欲轉去的看板 */
  int xbno;
  usint xbattr;
  char xboard[BNLEN + 1], xfolder[64];
  HDR xpost;

  HDR *hdr_org;
  int pos, cur;

  int tag, rc, locus, finish;
  int method;		/* 0:原文轉載 1:從公開看板/精華區/信箱轉錄文章 2:從秘密看板轉錄文章 */
  usint tmpbattr;
  char tmpboard[BNLEN + 1];
  char fpath[64], buf[ANSILINELEN];
  FILE *fpr, *fpw;

  char fpath_log[64];
  char content_log[256];

  /*  解決信箱轉錄問題 */
  int comefrom;		// 0: 從信箱轉錄 1: 從看板轉錄

  /* smiler.080830: 判斷轉錄是否有被 BBS 看門狗吃掉 */
  int is_bite = 0;

  if (xo->dir[0] == 'u')
    comefrom = 0;
  else
    comefrom = 1;

  if (!cuser.userlevel)	/* itoc.000213: 避免 guest 轉錄去 sysop 板 */
    return XO_NONE;

  int can_showturn = 0;
  if (xo->dir[0] == 'b')
  {
    if ((currbattr & BRD_NOFORWARD) && !(bbstate & STAT_BM))
    {
      vmsg("本看板禁止轉錄");
      return XO_NONE;
    }

    if (currbattr & BRD_SHOWTURN)
      can_showturn = 1;
  }

#ifdef HAVE_REFUSEMARK
  pos = xo->pos;	//smiler 1108
  cur = pos - xo->top;	//smiler 1108
  hdr_org = (HDR *) xo_pool + (xo->pos - xo->top);
  if (hdr_org->xmode & POST_RESTRICT)	/* 若為L文及F文,僅板主及作者可轉錄 */
  {
    if (strcmp(hdr_org->owner, cuser.userid) && !(bbstate & STAT_BM))
      return XO_NONE;
  }
#endif

  tag = AskTag("轉錄");
  if (tag < 0)
    return XO_FOOT;

  dir = xo->dir;

  if (!ask_board(xboard, BRD_W_BIT, "\n\n\033[1;33m請挑選適當的看板，切勿轉錄超過三板。\033[m\n\n") ||
    (*dir == 'b' && !strcmp(xboard, currboard)))	/* 信箱、精華區中可以轉錄至currboard */
    return XO_HEAD;

  hdr = tag ? &xhdr : (HDR *) xo_pool + (xo->pos - xo->top);	/* lkchu.981201: 整批轉錄 */

  /* 原作者轉錄自己文章時，可以選擇「原文轉載」 */
  method = (HAS_PERM(PERM_ALLBOARD) || (!tag && !strcmp(hdr->owner, cuser.userid))) &&
    (vget(2, 0, "(1)原文轉載 (2)轉錄文章？[2] ", buf, 3, DOECHO) == '1') ? 0 : 1;

  if (!tag)	/* lkchu.981201: 整批轉錄就不要一一詢問 */
  {
    if (method)
      sprintf(ve_title, "[轉錄] %.65s", hdr->title); /* smiler.070602: 改為轉錄時,標題為[轉錄]開頭 */
    else
      strcpy(ve_title, hdr->title);

    if (!vget(2, 0, "標題：", ve_title, TTLEN + 1, GCARRY))
      return XO_HEAD;
  }

#ifdef HAVE_REFUSEMARK
  rc = vget(2, 0, "(S)存檔 (L)站內 (X)密封 (Q)取消？[Q] ", buf, 3, LCECHO);
  if (rc != 'l' && rc != 's' && rc != 'x')
#else
  rc = vget(2, 0, "(S)存檔 (L)站內 (Q)取消？[Q] ", buf, 3, LCECHO);
  if (rc != 'l' && rc != 's')
#endif
    return XO_HEAD;

  if (method && *dir == 'b')	/* 從看板轉出，先檢查此看板是否為秘密板 */
  {
    /* 借用 tmpbattr */
    tmpbattr = (bshm->bcache + currbno)->readlevel;
    if (tmpbattr == PERM_SYSOP || tmpbattr == PERM_BOARD)
      method = 2;
  }

  xbno = brd_bno(xboard);
  xbattr = (bshm->bcache + xbno)->battr;

  /* Thor.990111: 在可以轉出前，要檢查有沒有轉出的權力? */
  if ((rc == 's') && (!HAS_PERM(PERM_INTERNET) || (xbattr & BRD_NOTRAN)))
    rc = 'l';

  /* 備份 currboard */
  if (method)
  {
    /* itoc.030325: 一般轉錄呼叫 ve_header，會使用到 currboard、currbattr，先備份起來 */
    strcpy(tmpboard, currboard);
    strcpy(currboard, xboard);
    tmpbattr = currbattr;
    currbattr = xbattr;
  }

  locus = 0;
  do	/* lkchu.981201: 整批轉錄 */
  {
    if (tag)
    {
      EnumTag(hdr, dir, locus, sizeof(HDR));

      if (method)
	sprintf(ve_title, "Fw: %.68s", str_ttl(hdr->title));	/* 已有 Re:/Fw: 字樣就只要一個 Fw: */
      else
	strcpy(ve_title, hdr->title);
    }

    strcpy(bbs_dog_title, ve_title);

    if (comefrom)	/* smiler.071114: 需為處在看板,下面幾行才需作判斷 */
    {
      if (hdr->xmode & GEM_FOLDER)	/* 非 plain text 不能轉 */
	continue;

#ifdef HAVE_REFUSEMARK
      if (hdr->xmode & POST_RESTRICT)
	continue;
#endif
      if (hdr->xmode & POST_NOFORWARD)
	continue;
    }
    hdr_fpath(fpath, dir, hdr);

    if ((xbattr & BRD_BBS_DOG ) && IS_BBS_DOG_FOOD(fpath))
    {
      brd_fpath(fpath_log, xboard, FN_BBSDOG_LOG);
      sprintf(content_log, "%s BBS看門狗計畫: 轉錄失敗\n作者: %s\n來源: %s\n標題: %s\n\n",
	Now(), cuser.userid, comefrom ? (method ? tmpboard : currboard) : "個人信箱", bbs_dog_title);
      f_cat(fpath_log, content_log);

      sprintf(fpath_log, FN_ETC_BBSDOG_LOG);
      sprintf(content_log, "%s BBS看門狗計畫: 轉錄失敗\n作者: %s\n來源: %s\n看板: %s\n標題: %s\n字串: %s\n\n",
	Now(), cuser.userid, comefrom ? (method ? tmpboard : currboard) : "個人信箱",
	xboard, bbs_dog_title, bbs_dog_str);
      f_cat(fpath_log, content_log);

      vmsg("您所轉錄文章不為本站接受，請洽本站站務群");
      is_bite = 1;
      continue;
    }

    if (IS_BRD_DOG_FOOD(fpath, xboard))
    {
      brd_fpath(fpath_log, xboard, FN_BBSDOG_LOG);
      sprintf(content_log, "%s 文章內容限制: 轉錄失敗\n作者: %s\n來源: %s\n標題: %s\n\n", Now(), cuser.userid, comefrom ? (method ? tmpboard : currboard) : "個人信箱", bbs_dog_title);
      f_cat(fpath_log, content_log);

      sprintf(fpath_log, FN_ETC_BBSDOG_LOG);
      sprintf(content_log, "%s 文章內容限制: 轉錄失敗\n作者: %s\n來源: %s\n看板: %s\n標題: %s\n字串: %s\n\n", Now(), cuser.userid, comefrom ? (method ? tmpboard : currboard) : "個人信箱", xboard, bbs_dog_title, bbs_dog_str);
      f_cat(fpath_log, content_log);

      vmsg("您所轉錄文章不為對方看板接受，請洽看板板主");
      is_bite = 1;
      continue;
    }

#ifdef HAVE_DETECT_CROSSPOST
    if (check_crosspost(fpath, xbno))
      break;
#endif

    brd_fpath(xfolder, xboard, fn_dir);

    if (method)		/* 一般轉錄 */
    {
      /* itoc.030325: 一般轉錄要重新加上 header */
      fpw = fdopen(hdr_stamp(xfolder, 'A', &xpost, buf), "w");
      ve_header(fpw);

      /* itoc.040228: 如果是從精華區轉錄出來的話，會顯示轉錄自 [currboard] 看板，
	 然而 currboard 未必是該精華區的看板。不過不是很重要的問題，所以就不管了 :p */
      fprintf(fpw, "※ 本文轉錄自 [%s] %s\n\n",
	*dir == 'u' ? cuser.userid : method == 2 ? "秘密" : tmpboard,
	*dir == 'u' ? "信箱" : "看板");

      /* Kyo.051117: 若是從秘密看板轉出的文章，刪除文章第一行所記錄的看板名稱 */
      finish = 0;
      if ((method == 2) && (fpr = fopen(fpath, "r")))
      {
	if (fgets(buf, sizeof(buf), fpr) &&
	  ((ptr = strstr(buf, str_post1)) || (ptr = strstr(buf, str_post2))) && (ptr > buf))
	{
	  ptr[-1] = '\n';
	  *ptr = '\0';

	  do
	  {
	    fputs(buf, fpw);
	  } while (fgets(buf, sizeof(buf), fpr));
	  finish = 1;
	}
	fclose(fpr);
      }
      if (!finish)
	f_suck(fpw, fpath);

      ve_banner(fpw, 0);
      fclose(fpw);

      strcpy(xpost.owner, cuser.userid);
      strcpy(xpost.nick, cuser.username);
    }
    else		/* 原文轉錄 */
    {
      /* itoc.030325: 原文轉錄直接 copy 即可 */
      hdr_stamp(xfolder, HDR_COPY | 'A', &xpost, fpath);

      strcpy(xpost.owner, hdr->owner);
      strcpy(xpost.nick, hdr->nick);
      strcpy(xpost.date, hdr->date);	/* 原文轉載保留原日期 */
    }

    strcpy(xpost.title, ve_title);

    if (rc == 's')
      xpost.xmode = POST_OUTGO;
#ifdef HAVE_REFUSEMARK
    else if (rc == 'x')
      xpost.xmode = POST_RESTRICT;
#endif

    rec_bot(xfolder, &xpost, sizeof(HDR));

    if (rc == 's')
      outgo_post(&xpost, xboard);


    char str_tag_score[50];
    sprintf(str_tag_score, " 轉錄至 %s 看板 ", xboard);

    if (can_showturn)	/* 只有看板才有可能有 can_showturn */
      post_t_score(xo,str_tag_score,hdr);

  } while (++locus < tag);

  btime_update(xbno);

  /* Thor.981205: check 被轉的板有沒有列入紀錄? */
  if (!(xbattr & BRD_NOCOUNT))
    cuser.numposts += tag ? tag : 1;	/* lkchu.981201: 要算 tag */

  /* 復原 currboard、currbattr */
  if (method)
  {
    strcpy(currboard, tmpboard);
    currbattr = tmpbattr;
  }

  if (!is_bite)
    vmsg("轉錄完成");
  else
    vmsg("部分轉錄失效");

  return XO_HEAD;
}


int
post_forward(xo)
  XO *xo;
{
  ACCT muser;
  HDR *hdr;

  if (!HAS_PERM(PERM_LOCAL))
    return XO_NONE;

  hdr = (HDR *) xo_pool + (xo->pos - xo->top);

  if (hdr->xmode & GEM_FOLDER)	/* 非 plain text 不能轉 */
    return XO_NONE;

#ifdef HAVE_REFUSEMARK
  if (hdr->xmode & POST_RESTRICT)	/* 若為L文及F文,僅板主及作者可轉錄 */
  {
    if (strcmp(hdr->owner, cuser.userid) && !(bbstate & STAT_BM))
      return XO_NONE;
  }
#endif

  if (xo->dir[0] == 'b')
  {
    if ((currbattr & BRD_NOFORWARD) && !(bbstate & STAT_BM))
    {
      vmsg("本看板禁止轉錄");
      return XO_NONE;
    }
    if (hdr->xmode & POST_NOFORWARD)		/* 此篇文章禁轉 */
      return XO_NONE;
  }

  if (acct_get("轉達信件給：", &muser) > 0)
  {
    strcpy(quote_user, hdr->owner);
    strcpy(quote_nick, hdr->nick);
    hdr_fpath(quote_file, xo->dir, hdr);
    sprintf(ve_title, "%.64s (fwd)", hdr->title);
    move(1, 0);
    clrtobot();
    prints("轉達給: %s (%s)\n標  題: %s\n", muser.userid, muser.username, ve_title);

    mail_send(muser.userid);
    *quote_file = '\0';

    char str_tag_score[50];
    sprintf(str_tag_score, " 轉寄至 %s 的bbs信箱 ", muser.userid);
    if (xo->dir[0] == 'b')
    {
      if (currbattr & BRD_SHOWTURN)
	post_t_score(xo,str_tag_score,hdr);
    }
  }
  return XO_HEAD;
}


/* ----------------------------------------------------- */
/* 板主功能：mark / delete / label			 */
/* ----------------------------------------------------- */


static int
post_mark(xo)
  XO *xo;
{
  if (bbstate & STAT_BOARD)
  {
    HDR *hdr;
    int pos, cur, xmode;

    pos = xo->pos;
    cur = pos - xo->top;
    hdr = (HDR *) xo_pool + cur;
    xmode = hdr->xmode;

#ifdef HAVE_LABELMARK
    if (xmode & POST_DELETE)	/* 待砍的文章不能 mark */
      return XO_NONE;
#endif

    if (xmode & POST_GOOD)
    {
      vmsg("此篇為優文，請按 M 鍵取消本篇優文");
      return XO_FOOT;
    }

    hdr->xmode = xmode ^ POST_MARKED;
    currchrono = hdr->chrono;
    rec_put(xo->dir, hdr, sizeof(HDR), xo->key == XZ_XPOST ? hdr->xid : pos, cmpchrono);

    move(3 + cur, 7);
    outs(post_attr(hdr++));
  }

  return XO_NONE;
}


/* smiler.080827: 設定優文 */
static int
post_mark_good(xo)
  XO *xo;
{
  ACCT x, acct;
  char buf[512];
  char fpath[64];

  if (bbstate & STAT_BOARD)
  {
    HDR *hdr;
    int pos, cur, xmode;

    pos = xo->pos;
    cur = pos - xo->top;
    hdr = (HDR *) xo_pool + cur;
    xmode = hdr->xmode;

    if (!strncmp(currboard, "P_", 2) || !strncmp(currboard, "R_", 2))
    {
      vmsg("個人看板，寢板不開放優文設定");
      return XO_FOOT;
    }

    if (!strcmp(hdr->owner, cuser.userid))
    {
      vmsg("自己不可加自己優文");
      return XO_BODY;
    }

#ifdef HAVE_LABELMARK
    if (xmode & POST_DELETE)	/* 待砍的文章不能 mark */
      return XO_NONE;
#endif

    if ((xmode & POST_MARKED) && (!(xmode & POST_GOOD)))
    {
      vmsg("請先按 m 取消本篇文章標記，再按 M 重設本篇文章為優文");
      return XO_BODY;
    }

    if ( (!strchr(hdr->owner, '.')) && (acct_load(&acct, hdr->owner) >= 0))
    {
      memcpy(&x, &acct, sizeof(ACCT));
      x.good_article = acct.good_article;
      if (vans(msg_sure_ny) != 'y')
	return XO_BODY;
      else
      {
	usr_fpath(fpath, acct.userid, FN_GOOD_ARTICLE);
	hdr->xmode = xmode ^ POST_MARKED;
	if (hdr->xmode & POST_MARKED)
	{
	  hdr->xmode |= POST_GOOD;
	  x.good_article++;
	  sprintf(buf, "%s %-13s 優文增 %s %s %s\n", Now(), cuser.userid, currboard, hdr->xname, hdr->title);
	}
	else
	{
	  hdr->xmode &= (~POST_GOOD);
	  x.good_article--;
	  sprintf(buf, "%s %-13s 優文減 %s %s %s\n", Now(), cuser.userid, currboard, hdr->xname, hdr->title);
	}

	/* smiler.080827: 模仿 acct_setup() 內站長更改使用者資料模式 */
	utmp_admset(x.userno, STATUS_DATALOCK | STATUS_COINLOCK);
	memcpy(&acct, &x, sizeof(ACCT));
	acct_save(&acct);
	f_cat(fpath, buf);
      }
    }

    currchrono = hdr->chrono;
    rec_put(xo->dir, hdr, sizeof(HDR), xo->key == XZ_XPOST ? hdr->xid : pos, cmpchrono);

    return XO_INIT;
  }
}


static void
reset_parent_chrono(hdd, ram)	/*ryancid: reset parent_chrono*/
  HDR *hdd, *ram;
{
  hdd->parent_chrono = 0;
}


static void
set_parent_chrono(hdd, ram)	/*ryancid: set parent_chrono*/
  HDR *hdd, *ram;
{
  hdd->parent_chrono = 1;
}


static int
post_bottom(xo)
  XO *xo;
{
  if (!(bbstate & STAT_BOARD))
    return XO_NONE;

  int pos;
  HDR *hdr, post;
  char fpath[64];

  hdr = (HDR *) xo_pool + (xo->pos - xo->top);
  pos = xo->pos;

  if (!(hdr->xmode & POST_BOTTOM))
  {
    if (hdr->parent_chrono)	/* Allow only one bottom post per article */
      return post_load(xo);

    hdr_fpath(fpath, xo->dir, hdr);
    hdr_stamp(xo->dir, HDR_LINK | 'A', &post, fpath);
    post.parent_chrono = hdr->chrono;
#ifdef HAVE_REFUSEMARK
    post.xmode = POST_BOTTOM | (hdr->xmode & POST_RESTRICT);
#else
    post.xmode = POST_BOTTOM;
#endif
    /*ryancid: copy the score*/
    if (hdr->xmode & POST_SCORE)
    {
      post.xmode |= POST_SCORE;
      post.score = hdr->score;
    }

    strcpy(post.owner, hdr->owner);
    strcpy(post.nick, hdr->nick);
    strcpy(post.title, hdr->title);
    rec_add(xo->dir, &post, sizeof(HDR));

    /*ryancid: set the parent_chrono*/
    currchrono = hdr->chrono;
    rec_ref(xo->dir, hdr, sizeof(HDR),
      xo->key == XZ_XPOST ? hdr->xid : pos, cmpchrono , set_parent_chrono);
  }
  else if (hdr->xmode & POST_MARKED)
  {
    vmsg("請先取消 m 標記再刪除置底文章！");
    return XO_FOOT;
  }
  else
  {
    if (vans("確定要取消此篇置底文章(y/N)？[N] ") != 'y')
      return XO_FOOT;

    /*reset the original chrono*/
    currchrono = hdr->parent_chrono;
    rec_ref(xo->dir, hdr, sizeof(HDR),
      xo->key == XZ_XPOST ? hdr->xid : pos, cmpchrono , reset_parent_chrono);

    currchrono = hdr->chrono;
    if (!rec_del(xo->dir, sizeof(HDR), xo->key == XZ_XPOST ? hdr->xid : pos, cmpchrono))
    {
      pos = move_post(hdr, xo->dir, bbstate & STAT_BOARD);
    }
  }

  return post_load(xo); /* ckm.070325: 重新載入列表 */
}


#ifdef HAVE_REFUSEMARK
static void
RefusePal_fpath(fpath, board, mode, hdr)
  char *fpath;
  char *board;
  char mode;	/* 'C':Cansee  'R':Refimage */
  HDR *hdr;
{
  sprintf(fpath, "brd/%s/RefusePal_DIR/%s_%s",
    board, mode == 'C' ? "Cansee" : "Refimage", hdr->xname);
}


void
RefusePal_kill(board, hdr)	/* amaki.030322: 用來砍名單小檔 */
  char *board;
  HDR *hdr;
{
  char fpath[64];

  RefusePal_fpath(fpath, board, 'C', hdr);
  unlink(fpath);
  RefusePal_fpath(fpath, board, 'R', hdr);
  unlink(fpath);
}


int	/* -1:加密(L)文  1:好友(F)文  0:普通文 */
RefusePal_level(board, hdr)	//smiler 1108
  char *board;
  HDR *hdr;
{
  int fsize;
  char fpath[64];
  int *fimage;
  struct stat st;

  if (!(hdr->xmode & POST_RESTRICT))
    return 0;

  RefusePal_fpath(fpath, board, 'R', hdr);	//0709
  if (dashf(fpath))	/* 有 POST_RESTRICT 而 pal 檔不存在的就是加密(L)文 */
  {
    if ((!stat(fpath, &st) && S_ISREG(st.st_mode) && !st.st_size) ||
      !(hdr->xmode & POST_FRIEND))	/* 有 pal 檔而沒 POST_FRIEND */
    {	/* 有些名單的 size 為 0 */
      RefusePal_kill(currboard, hdr);
      return -1;
    }

    if (!strcmp(hdr->owner, cuser.userid) || (bbstate & STAT_BM))
      return 1;

    if (fimage = (int *) f_img(fpath, &fsize))
    {
      fsize = belong_pal(fimage, fsize / sizeof(int), cuser.userno);
      free(fimage);
      if (fsize)
	return fsize;
    }
  }

  return -1;
}


int	/* 1:在可見名單上 0:不在可見名單上 */
RefusePal_belong(board, hdr)
  char *board;
  HDR *hdr;
{
  int fsize;
  char fpath[64];
  int *fimage;

  if (!strcmp(hdr->owner, cuser.userid) || (bbstate & STAT_BM))
    return 1;

  RefusePal_fpath(fpath, board, 'R', hdr);	//smiler 1109
  if (fimage = (int *) f_img(fpath, &fsize))
  {
    fsize = belong_pal(fimage, fsize / sizeof(int), cuser.userno);
    free(fimage);
    return fsize;
  }
  return 0;
}


static void
refusepal_cache(hdr, board)
  HDR *hdr;
  char *board;
{
  int fd, max;
  char fpath[64];
  int pool[PAL_MAX];

  RefusePal_fpath(fpath, board, 'C', hdr);

  if (max = image_pal(fpath, pool))
  {
    RefusePal_fpath(fpath, board, 'R', hdr);
    if ((fd = open(fpath, O_WRONLY | O_CREAT | O_TRUNC, 0600)) >= 0)
    {
      write(fd, pool, max * sizeof(int));
      close(fd);
    }
  }
  else
    RefusePal_kill(board, hdr);
}


static int
XoBM_Refuse_pal(hdr)
  HDR *hdr;
{
  XO *xt;
  char fpath[64];
  char fpath_friend[64];
  int ans, ans2;

  if (strcmp(hdr->owner, cuser.userid) && !(bbstate & STAT_BM))
    return 0;

  brd_fpath(fpath, currboard, "RefusePal_DIR");
  if (!dashd(fpath))
    mkdir(fpath, 0700);
  RefusePal_fpath(fpath, currboard, 'C', hdr);

  if (dashf(fpath))	/* 修改舊的名單 */
    ans = '3';
  else if (bbstate & STAT_BM)
    ans = vans("◎選擇 1)好友 2)板友 3)編輯新名單 Q)直接加密 [Q] ");
  else
    ans = '1';

  switch (ans)
  {
  case '1':
    ans = vans("◎選擇 1~5)好友群組名單 6)編輯新名單 Q)直接加密 [Q] ");
    if (ans < '1' || ans > '6')
      ans = -1;
    else if (ans == '6')
      ans2 = '9';
    else	/* 1-5, 引入個人好友群組名單 */
      ans2 = 0;
    break;

  case '2':
    ans2 = vans("◎選擇 0)板友名單 1~8)板友特別名單 9)編輯新名單 Q)直接加密 [Q] ");
    if (ans2 < '0' || ans2 > '9')
      ans = -1;
    break;

  case '3':
    ans2 = '9';
    break;

  default:
    return 0;
  }

  if (ans == -1)
    return 0;

  if (!ans2)		/* ans:1-5, ans2 尚未選擇, 引入個人好友群組名單 */
  {
    usr_fpath(fpath_friend, cuser.userid, "list.0");
    fpath_friend[strlen(fpath_friend) - 1] = ans;
  }
  else if (ans2 != '9')	/* 引入板友/板友特別名單 */
  {
    if (ans2 == '0')	/* 引入板友名單 */
      brd_fpath(fpath_friend, currboard, "friend");
    else
      sprintf(fpath_friend, "brd/%s/friend_%c", currboard, ans2);
  }
  else			/* (ans2 = '9') 編輯新名單 */
  {
    xz[XZ_PAL - XO_ZONE].xo = xt = xo_new(fpath);
    xt->key = PALTYPE_BPAL;	//smiler 1106
    xover(XZ_PAL);
//    (xo->key == XZ_XPOST) ? xpost_init(xo) : post_init(xo);
    refusepal_cache(hdr, currboard);
    free(xt);
  }

  if (ans2 != '9')
  {
    vmsg("引入名單！");
    f_cp(fpath_friend, fpath, O_TRUNC);
    xz[XZ_PAL - XO_ZONE].xo = xt = xo_new(fpath);
    xt->key = PALTYPE_BPAL;	//smiler 1106
    xover(XZ_PAL);
//    (xo->key == XZ_XPOST) ? xpost_init(xo) : post_init(xo);
    refusepal_cache(hdr, currboard);
    free(xt);
  }

  vmsg("設定完成！");
  return 1;
}


static int
post_refuse(xo)	/* itoc.010602: 文章加密 */
  XO *xo;
{
  HDR *hdr, fhdr;
  int pos, cur, ans, xmode;

  if (!cuser.userlevel) /* itoc.020114: guest 不能對其他 guest 的文章加密 */
    return XO_NONE;

  if (currbattr & BRD_PUBLIC)
    return XO_NONE;

  if ((currbattr & BRD_NOL) && !(bbstate & STAT_BM))
    return XO_NONE;

  pos = xo->pos;
  cur = pos - xo->top;
  hdr = (HDR *) xo_pool + cur;
  xmode = hdr->xmode;

  /* XoBM_Refuse_pal() 進 xover(XZ_PAL) 時, 會重寫 xo_pool, 這裡先存起來 */
  memcpy(&fhdr, hdr, sizeof(HDR));

  if (!strcmp(hdr->owner, cuser.userid) || (bbstate & STAT_BM))
  {
    switch (RefusePal_level(currboard, hdr))
    {
    case 0:	/* 一般文章 */
      ans = vans("將此篇文章 1)加密 2)設為好友文 Q)取消？[Q] ");
      break;

    case -1:	/* 加密(L)文章 */
      ans = vans("將此篇文章 1)解密 2)設為好友文 Q)取消？[Q] ");
      break;

    case 1:	/* 好友(F)文章 */
      ans = vans("將此篇文章 1)加密/解密(將砍除可見好友名單) 2)修改可見好友名單 Q)取消？[Q] ");
      if (ans == '1')
      {
	ans = vans("將此篇文章 1)加密 2)解密 (加解密都將砍除可見好友名單) Q)取消？[Q] ");
	if ((ans == '1') || (ans == '2'))
	{
	  xmode &= ~POST_FRIEND;
	  RefusePal_kill(currboard, hdr);
	  if (ans == '1')		/* 先拿掉, 待會再加回去 */
	    xmode ^= POST_RESTRICT;
	  else
	    ans = '1';
	}
      }
      break;
    }

    switch (ans)
    {
    case '1':	/* 執行加解密動作 */
//      xmode &= ~POST_FRIEND;	/* 以防萬一 */
//      RefusePal_kill(currboard, hdr);
      xmode ^= POST_RESTRICT;
      break;

    case '2':	/* 編輯可見好友名單 */
      xmode |= (POST_RESTRICT | POST_FRIEND);
      XoBM_Refuse_pal(&fhdr);		/* 傳回 0 表示取消動作 */
      break;

    default:	/* 取消動作 */
      return XO_FOOT;
    }

    fhdr.xmode = xmode;
    currchrono = fhdr.chrono;
    rec_put(xo->dir, &fhdr, sizeof(HDR), xo->key == XZ_XPOST ? fhdr.xid : pos, cmpchrono);
    btime_update(brd_bno(currboard));	/* 解決因無法閱讀鎖文而一直亮燈的狀況 */
    return XO_LOAD;
  }

  return XO_NONE;
}


static int
XoBM_add_pal()
{
  int ans;
  char fpath[64];
  XO *xt;

  if (!(bbstate & STAT_BM))
    return 0;

  ans = vans("◎編輯板友特別名單 1~8？[Q] ");
  if (ans > '0' && ans < '9')
  {
    sprintf(fpath, "brd/%s/friend_%c", currboard, ans);
    xz[XZ_PAL - XO_ZONE].xo = xt = xo_new(fpath);
    xt->key = PALTYPE_BPAL;	//smiler 1106
    xover(XZ_PAL);
    free(xt);
    return 1;
  }
  else
    return 0;
}
#endif


#ifdef HAVE_LABELMARK
static int
post_label(xo)
  XO *xo;
{
  if (bbstate & STAT_BOARD)
  {
    HDR *hdr;
    int pos, cur, xmode;

    pos = xo->pos;
    cur = pos - xo->top;
    hdr = (HDR *) xo_pool + cur;
    xmode = hdr->xmode;

    if (xmode & (POST_MARKED | POST_RESTRICT))	/* mark 或 加密的文章不能待砍 */
      return XO_NONE;

    hdr->xmode = xmode ^ POST_DELETE;
    currchrono = hdr->chrono;
    rec_put(xo->dir, hdr, sizeof(HDR), xo->key == XZ_XPOST ? hdr->xid : pos, cmpchrono);

    move(3 + cur, 7);
    outs(post_attr(hdr));

    return pos + 1 + XO_MOVE;	/* 跳至下一項 */
  }

  return XO_NONE;
}


static int
post_delabel(xo)
  XO *xo;
{
  int fdr, fsize, xmode;
  char fnew[64], fold[64], *folder;
  HDR *hdr;
  FILE *fpw;

  if (!(bbstate & STAT_BOARD))
    return XO_NONE;

  if (vans("確定要刪除待砍文章嗎(Y/N)？[N] ") != 'y')
    return XO_FOOT;

  folder = xo->dir;
  if ((fdr = open(folder, O_RDONLY)) < 0)
    return XO_FOOT;

  if (!(fpw = f_new(folder, fnew)))
  {
    close(fdr);
    return XO_FOOT;
  }

  fsize = 0;
  mgets(-1);
  while (hdr = mread(fdr, sizeof(HDR)))
  {
    xmode = hdr->xmode;

    if (!(xmode & POST_DELETE))
    {
      if ((fwrite(hdr, sizeof(HDR), 1, fpw) != 1))
      {
	close(fdr);
	fclose(fpw);
	unlink(fnew);
	return XO_FOOT;
      }
      fsize++;
    }
    else
    {
      /* 連線砍信 */
      cancel_post(hdr);

      hdr_fpath(fold, folder, hdr);
      unlink(fold);
      if (xmode & POST_RESTRICT)
	RefusePal_kill(currboard, hdr);

    }
  }
  close(fdr);
  fclose(fpw);

  sprintf(fold, "%s.o", folder);
  rename(folder, fold);
  if (fsize)
    rename(fnew, folder);
  else
    unlink(fnew);

  btime_update(currbno);

  return post_load(xo);
}
#endif


static void
backup_post_log(fpath, brdname, hdr, mode)
  char *fpath;		/* 原文 */
  char *brdname;	/* 目標看板 */
  HDR *hdr;		/* 原始 hdr */
  int mode;		/* 0: 記錄 hdr 原作者  1: 記錄 cuser */
{
  HDR bhdr;
  char folder[64];

  brd_fpath(folder, brdname, FN_DIR);

  hdr_stamp(folder, HDR_COPY | 'A', &bhdr, fpath);
  strcpy(bhdr.title, hdr->title);
  strcpy(bhdr.owner, mode ? cuser.userid : hdr->owner);
  strcpy(bhdr.nick, mode ? cuser.username : hdr->nick);
  /* bhdr.xmode = POST_OUTGO; */
  rec_bot(folder, &bhdr, sizeof(HDR));
  btime_update(brd_bno(brdname));
}


static int
post_delete(xo)	/* 單一刪文 */
  XO *xo;
{
  ACCT x, acct;
  int pos, cur, by_BM;
  HDR *hdr;
  char buf[256];
  char copied[64];
  char fpath[64], reason[55], title[70];

  /* smiler.1111: 保護Deletelog 以及 Editlog 的 記錄不被移除 */
  if (!strcmp(currboard,BN_DELLOG) || !strcmp(currboard,BN_EDITLOG))
    return XO_NONE;

  if (!cuser.userlevel || !strcmp(currboard, BN_DELETED) || !strcmp(currboard, BN_JUNK))
    return XO_NONE;

  pos = xo->pos;
  cur = pos - xo->top;
  hdr = (HDR *) xo_pool + cur;

  if (hdr->xmode & POST_BOTTOM)
    return post_bottom(xo);

  if ((hdr->xmode & POST_MARKED) ||
    (!(bbstate & STAT_BOARD) && strcmp(hdr->owner, cuser.userid)))
    return XO_NONE;

  by_BM = bbstate & STAT_BOARD;

  if (vans(msg_del_ny) == 'y')
  {
    /* smiler.080827: 非自己砍文時，可設定是否要劣退 */
    if (strcmp(hdr->owner, cuser.userid) && (!strchr(hdr->owner, '.')) && (acct_load(&acct, hdr->owner) >= 0))
    {
      switch (vans("文章已刪除，額外動作 1)寄信通知原作者 2)劣退 N)免了,直接刪除？[N]"))
      {
      case '1':
	if (!vget(b_lines, 0, "請輸入理由：", reason, 55, DOECHO))
	{
	  vmsg("取消");
	  return XO_BODY;
	}

	hdr_fpath(fpath, xo->dir, hdr);
	sprintf(title, "[文章刪除] 理由: %s", reason);
	mail_him(fpath, acct.userid, title, 0);
	break;

      case '2':
	if (vans("要劣退本篇文章(y/N)？[N]") == 'y')
	{
	  if (!vget(b_lines, 0, "請輸入理由：", reason, 55, DOECHO))
	  {
	    vmsg("取消");
	    return XO_BODY;
	  }

	  memcpy(&x, &acct, sizeof(ACCT));
	  x.poor_article = acct.poor_article;

	  usr_fpath(fpath, acct.userid, FN_POOR_ARTICLE);
	  x.poor_article++;
	  sprintf(buf, "%s %-13s 劣退增 %s %s %s %s\n",
	    Now(), cuser.userid, currboard, hdr->xname, hdr->title, reason);
	  /* smiler.080827: 模仿 acct_setup() 內站長更改使用者資料模式
	  ----------------------------------------------------------------------------------
	     itoc.010811: 動態設定線上使用者
	     被站長改過資料的線上使用者(包括站長自己)，其 cutmp->status 會被加上 STATUS_DATALOCK
	     這個旗標，就無法 acct_save()，於是站長便可以修改線上使用者資料
	     在站長修改過才上線的 ID 因為其 cutmp->status 沒有 STATUS_DATALOCK 的旗標，
	     所以將可以繼續存取，所以線上如果同時有修改前、修改後的同一隻 ID multi-login，也是無妨。
	  ---------------------------------------------------------------------------------- */
	  utmp_admset(x.userno, STATUS_DATALOCK | STATUS_COINLOCK);
	  memcpy(&acct, &x, sizeof(ACCT));
	  acct_save(&acct);
	  f_cat(fpath, buf);

	  hdr_fpath(fpath, xo->dir, hdr);
	  sprintf(title, "[劣退] 理由: %s", reason);
	  mail_him(fpath, acct.userid, title, 0);
        }
	break;

      default:
	break;
      }
    }

    /* smiler 1031 */
    if (deletelog_use)
    {
      hdr_fpath(copied,xo->dir,hdr);
      backup_post_log(copied, BN_DELLOG, hdr, 1);
    }

    currchrono = hdr->chrono;

    if (!rec_del(xo->dir, sizeof(HDR), xo->key == XZ_XPOST ? hdr->xid : pos, cmpchrono))
    {
      pos = move_post(hdr, xo->dir, by_BM);

      if (!by_BM && !(currbattr & BRD_NOCOUNT))
      {
	/* itoc.010711: 砍文章要扣錢，算檔案大小 */
	pos = pos >> 3;	/* 相對於 post 時 wordsnum / 10 */

	/* itoc.010830.註解: 漏洞: 若 multi-login 砍不到另一隻的錢 */
	if (cuser.money > pos)
	  cuser.money -= pos;
	else
	  cuser.money = 0;

	if (cuser.numposts > 0)
	  cuser.numposts--;
	sprintf(buf, "%s，您的文章減為 %d 篇", MSG_DEL_OK, cuser.numposts);
	vmsg(buf);
      }

      if (xo->key == XZ_XPOST)
      {
	vmsg("原列表經刪除後混亂，請重進串接模式！");
	return XO_QUIT;
      }
      return XO_LOAD;
    }
  }
  return XO_FOOT;
}


static int
chkpost(hdr)
  HDR *hdr;
{
  return ((hdr->xmode & POST_MARKED) || (hdr->xmode & POST_BOTTOM));
}


static int
vfypost(hdr, pos)
  HDR *hdr;
  int pos;
{
  return (Tagger(hdr->chrono, pos, TAG_NIN) || chkpost(hdr));
}


static void
delpost(xo, hdr)
  XO *xo;
  HDR *hdr;
{
  char fpath[64];
  char copied[64];

  if (deletelog_use)
  {
    hdr_fpath(copied, xo->dir, hdr);
    backup_post_log(copied, BN_DELLOG, hdr, 1);
  }

  cancel_post(hdr);
  hdr_fpath(fpath, xo->dir, hdr);
  unlink(fpath);
  if (hdr->xmode & POST_RESTRICT)
    RefusePal_kill(currboard, hdr);
}


static int
post_rangedel(xo)
  XO *xo;
{
  if (!(bbstate & STAT_BOARD))
    return XO_NONE;

  /* smiler.071111: 保護Deletelog 以及 Editlog 的 記錄不被移除 */
  if (!strcmp(currboard,BN_DELLOG) || !strcmp(currboard, BN_EDITLOG))
    return XO_NONE;

  btime_update(currbno);

  return xo_rangedel(xo, sizeof(HDR), chkpost, delpost);
}


static int
post_prune(xo)
  XO *xo;
{
  int ret;

  /* smiler.071111: 保護 BN_DELLOG 以及 BN_EDITLOG 的記錄不被移除 */
  if (!strcmp(currboard, BN_DELLOG) || !strcmp(currboard, BN_EDITLOG))
    return XO_NONE;

  if (!(bbstate & STAT_BOARD))
    return XO_NONE;

  ret = xo_prune(xo, sizeof(HDR), vfypost, delpost);

  btime_update(currbno);

  if (xo->key == XZ_XPOST && ret == XO_LOAD)
  {
    vmsg("原列表經批次刪除後混亂，請重進串接模式！");
    return XO_QUIT;
  }

  return ret;
}


static int
post_copy(xo)	   /* itoc.010924: 取代 gem_gather */
  XO *xo;
{
  int tag;

  tag = AskTag("看板文章拷貝");

  if (tag < 0)
    return XO_FOOT;

  if (!(bbstate & STAT_BM))
    return XO_NONE;

#ifdef HAVE_REFUSEMARK
  gem_buffer(xo->dir, tag ? NULL : (HDR *) xo_pool + (xo->pos - xo->top), chkrescofo, 1);
#else
  gem_buffer(xo->dir, tag ? NULL : (HDR *) xo_pool + (xo->pos - xo->top), chkrescofo, 1);
#endif

  if (bbstate & STAT_BOARD)
  {
#ifdef XZ_XPOST
    if (xo->key == XZ_XPOST)
    {
      zmsg("檔案標記完成。[注意] 您必須先離開串接模式才能進入精華區。");
      return XO_FOOT;
    }
    else
#endif
    {
      zmsg("拷貝完成。[注意] 貼上後才能刪除原文！");
      return post_gem(xo);	/* 拷貝完直接進精華區 */
    }
  }

  zmsg("檔案標記完成。[注意] 您只能在擔任(小)板主所在或個人精華區貼上。");
  return XO_FOOT;
}


/* ----------------------------------------------------- */
/* 站長功能：edit / title					 */
/* ----------------------------------------------------- */


#ifdef DO_POST_FILTER
static int
find_xname_by_chrono(chrono, xpath, mode)	/* 配合看門狗，修改文章後找副本一併修改 */
  time4_t chrono;
  char *xpath;
  int mode;	/* 0: 從原文找置底  1: 從置底找原文 */
{
  char fpath[64];
  int fsize, find = 0;
  HDR *data, *head, *tail;

  brd_fpath(fpath, currboard, FN_DIR);
  if (data = (HDR *) f_img(fpath, &fsize))
  {
    head = data;
    tail = data + (fsize / sizeof(HDR));
    do
    {
      if (mode)
      {
	if (head->chrono == chrono)
	{
	  find = 1;
	  hdr_fpath(xpath, fpath, head);
	  break;
	}
      }
      else
      {
	if (head->parent_chrono == chrono)
	{
	  find = 1;
	  hdr_fpath(xpath, fpath, head);
	  break;
	}
      }
    } while (++head < tail);
    free(data);
  }

  return find;
}
#endif


int
post_edit(xo)
  XO *xo;
{
  char fpath[64], tmpfile[64];
  HDR *hdr;
  FILE *fp;
  /* smiler 1031 */
  char copied[64];

  /* smiler.071111 保護 Editlog 以及 Deletelog 板的備份資料 */
  if (!strcmp(currboard, BN_DELLOG) || !strcmp(currboard, BN_EDITLOG))
    return XO_NONE;

  hdr = (HDR *) xo_pool + (xo->pos - xo->top);

  /* smiler 1031 */
  hdr_fpath(copied, xo->dir, hdr);
  hdr_fpath(fpath, xo->dir, hdr);

  curredit = 0;

  if (HAS_PERM(PERM_ALLBOARD))			/* 站長修改 */
  {
#ifdef HAVE_REFUSEMARK
    if (!chkrestrict(hdr))
      return (cutmp->mode == M_READA) ? XO_HEAD : XO_NONE;
#endif

    backup_post_log(copied, BN_EDITLOG, hdr, 0);	/* smiler 1031 */

#ifdef DO_POST_FILTER
    strcpy(tmpfile, "tmp/");
    strcat(tmpfile, hdr->xname);
    f_cp(fpath, tmpfile, O_TRUNC);

    vedit(tmpfile, 0);
#else
    vedit(fpath, 0);
#endif

#ifdef DO_POST_FILTER
    strcpy(bbs_dog_title, hdr->title);
    if (post_filter(tmpfile))	/* smiler.080830: 針對文章標題內容偵測有無不當之處 */
      unlink(tmpfile);
    else
    {
      char xpath[64];

      unlink(fpath);
      f_ln(tmpfile, fpath);
      unlink(tmpfile);

      if (hdr->xmode & POST_BOTTOM)	/* 修改置底文，去找原文一起改 */
      {
	if (find_xname_by_chrono(hdr->parent_chrono, xpath, 1))
	{
	  unlink(xpath);
	  f_ln(fpath, xpath);
	}
      }
      else if (hdr->parent_chrono)	/* 此篇文章有置底副本，一併更改之 */
      {
	if (find_xname_by_chrono(hdr->chrono, xpath, 0))
	{
	  unlink(xpath);
	  f_ln(fpath, xpath);
	}
      }
    }
#endif

    backup_post_log(copied, BN_EDITLOG, hdr, 0);	/* smiler 1031 */
  }
  else if ((cuser.userlevel && !strcmp(hdr->owner, cuser.userid)) || (bbstate & STAT_BM))	/* 板主/原作者修改 */
  {
    if (currbattr & BRD_NOEDIT)
    {
      vedit(fpath, -1);
      return XO_HEAD;
    }

    backup_post_log(copied, BN_EDITLOG, hdr, 0);	/* smiler 1031 */

#ifdef DO_POST_FILTER
    strcpy(tmpfile, "tmp/");
    strcat(tmpfile, hdr->xname);
    f_cp(fpath, tmpfile, O_TRUNC);

    if (!vedit(tmpfile, 0))	/* 若非取消則加上修改資訊 */
    {
      if (fp = fopen(tmpfile, "a"))
      {
#else
    if (!vedit(fpath, 0))	/* 若非取消則加上修改資訊 */
    {
      if (fp = fopen(fpath, "a"))
      {
#endif
	ve_banner(fp, 1);
	fclose(fp);
      }
    }

#ifdef DO_POST_FILTER
    strcpy(bbs_dog_title, hdr->title);
    if (post_filter(tmpfile))	/* smiler.080830: 針對文章標題內容偵測有無不當之處 */
      unlink(tmpfile);
    else
    {
      char xpath[64];

      unlink(fpath);
      f_ln(tmpfile, fpath);
      unlink(tmpfile);

      if (hdr->xmode & POST_BOTTOM)	/* 修改置底文，去找原文一起改 */
      {
	if (find_xname_by_chrono(hdr->parent_chrono, xpath, 1))
	{
	  unlink(xpath);
	  f_ln(fpath, xpath);
	}
      }
      else if (hdr->parent_chrono)	/* 此篇文章有置底副本，一併更改之 */
      {
	if (find_xname_by_chrono(hdr->chrono, xpath, 0))
	{
	  unlink(xpath);
	  f_ln(fpath, xpath);
	}
      }
    }
#endif

    backup_post_log(copied, BN_EDITLOG, hdr, 0);	/* smiler 1031 */
  }
  else		/* itoc.010301: 提供使用者修改(但不能儲存)其他人發表的文章 */
#if 1
  {
    if (!chkrescofo(hdr))
      return (cutmp->mode == M_READA) ? XO_HEAD : XO_NONE;

    vedit(fpath, -1);
  }
#else
    return (cutmp->mode == M_READA) ? XO_HEAD : XO_NONE;
#endif

  /* return post_head(xo); */
  return XO_HEAD;	/* itoc.021226: XZ_POST 和 XZ_XPOST 共用 post_edit() */
}


void
header_replace(xo, hdr)		/* itoc.010709: 修改文章標題順便修改內文的標題 */
  XO *xo;
  HDR *hdr;
{
  FILE *fpr, *fpw;
  char srcfile[64], tmpfile[64], buf[ANSILINELEN];

  hdr_fpath(srcfile, xo->dir, hdr);
  strcpy(tmpfile, "tmp/");
  strcat(tmpfile, hdr->xname);
  f_cp(srcfile, tmpfile, O_TRUNC);

  if (!(fpr = fopen(tmpfile, "r")))
    return;

  if (!(fpw = fopen(srcfile, "w")))
  {
    fclose(fpr);
    return;
  }

  fgets(buf, sizeof(buf), fpr);		/* 加入作者 */
  fputs(buf, fpw);

  fgets(buf, sizeof(buf), fpr);		/* 加入標題 */
  if (!str_ncmp(buf, "標", 2))		/* 如果有 header 才改 */
  {
    strcpy(buf, buf[2] == ' ' ? "標  題: " : "標題: ");
    strcat(buf, hdr->title);
    strcat(buf, "\n");
  }
  fputs(buf, fpw);

  while (fgets(buf, sizeof(buf), fpr))	/* 加入其他 */
    fputs(buf, fpw);

  fclose(fpr);
  fclose(fpw);
  f_rm(tmpfile);
}


static int
post_title(xo)
  XO *xo;
{
  FILE *fp;
  char tmpfile[64];
  char tmptitle[TTLEN + 1];
  HDR *fhdr, mhdr;
  int pos, cur;

  if (!cuser.userlevel)	/* itoc.000213: 避免 guest 在 sysop 板改標題 */
    return XO_NONE;

  pos = xo->pos;
  cur = pos - xo->top;
  fhdr = (HDR *) xo_pool + cur;
  memcpy(&mhdr, fhdr, sizeof(HDR));

  if ((strcmp(cuser.userid, mhdr.owner) && (!(bbstate & STAT_BM))) && !HAS_PERM(PERM_ALLBOARD))
    return XO_NONE;

  strcpy(tmptitle, mhdr.title);

  vget(b_lines, 0, "標題：", mhdr.title, TTLEN + 1, GCARRY);

  /* smiler.080913: 偵測所改標題是否符合看門狗規定 */
  sprintf(tmpfile, "tmp/%s_%s_title", cuser.userid, mhdr.xname);
  fp = fopen(tmpfile, "w");
  fprintf(fp, "%s", mhdr.title);
  fclose(fp);

#ifdef DO_POST_FILTER
  strcpy(bbs_dog_title, tmptitle);
  if (post_filter(tmpfile))	/* smiler.080830: 針對文章標題內容偵測有無不當之處 */
  {
    unlink(tmpfile);
    return XO_HEAD;
  }
  else
    unlink(tmpfile);
#endif

  if (HAS_PERM(PERM_ALLBOARD))	/* itoc.000213: 原作者只能改標題 */
  {
    vget(b_lines, 0, "作者：", mhdr.owner, 73 /* sizeof(mhdr.owner) */, GCARRY);
    /* Thor.980727: sizeof(mhdr.owner) = 80 會超過一行 */
    vget(b_lines, 0, "暱稱：", mhdr.nick, sizeof(mhdr.nick), GCARRY);
    vget(b_lines, 0, "日期：", mhdr.date, sizeof(mhdr.date), GCARRY);
  }

  if (memcmp(fhdr, &mhdr, sizeof(HDR)) && vans(msg_sure_ny) == 'y')
  {
    memcpy(fhdr, &mhdr, sizeof(HDR));
    currchrono = fhdr->chrono;
    rec_put(xo->dir, fhdr, sizeof(HDR), xo->key == XZ_XPOST ? fhdr->xid : pos, cmpchrono);

    move(3 + cur, 0);
    post_item(++pos, fhdr);

    /* itoc.010709: 修改文章標題順便修改內文的標題 */
    header_replace(xo, fhdr);
  }
  return XO_FOOT;
}


/* ----------------------------------------------------- */
/* 額外功能：write / score				 */
/* ----------------------------------------------------- */


int
post_write(xo)			/* itoc.010328: 丟線上作者水球 */
  XO *xo;
{
  if (HAS_PERM(PERM_PAGE))
  {
    HDR *hdr;
    UTMP *up;

    hdr = (HDR *) xo_pool + (xo->pos - xo->top);

    if (!(hdr->xmode & POST_INCOME) && (up = utmp_seek(hdr)))
      do_write(up);
  }
  return XO_NONE;
}


#ifdef HAVE_SCORE

static int curraddscore;


static int
cmpparent(hdr)
  HDR *hdr;
{
  /* 可以不用檢查 POST_BOTTOM，因為一般文章的 parent_chrono = 0 */
  return /* (hdr->xmode & POST_BOTTOM) && */
    (hdr->parent_chrono == currchrono);
}


static void
change_stamp(folder, hdr)
  char *folder;
  HDR *hdr;
{
  HDR buf;

  /* 為了確定新造出來的 stamp 也是 unique (不和既有的 chrono 重覆)，
     就產生一個新的檔案，該檔案隨便 link 即可。
     這個多產生出來的垃圾會在 expire 被 sync 掉 (因為不在 .DIR 中) */
  hdr_stamp(folder, HDR_LINK | 'A', &buf, "etc/stamp");
  hdr->stamp = buf.chrono;
}


static void
addscore(hdd, ram)
  HDR *hdd, *ram;
{
  hdd->xmode |= POST_SCORE;
  hdd->stamp = ram->stamp;
  if (curraddscore > 0)
  {
    if (hdd->score < 127)
      hdd->score++;
  }
  else if (curraddscore < 0)
  {
    if (hdd->score > -128)
      hdd->score--;
  }
}


int
post_t_score(xo, log, hdr)	/* 轉錄文章記錄 */
  XO *xo;
  char *log;
  HDR *hdr;
{
  int pos, cur, ans, vtlen, maxlen;
  char *dir, *userid, *verb, fpath[64], reason[80];
  FILE *fp;

  pos = xo->pos;
  cur = pos - xo->top;

  if (hdr->xmode & POST_NOSCORE)
    return XO_NONE;

  ans = '3';
  verb = "0m==";
  vtlen = 2;

  maxlen = 63 - strlen(cuser.userid) - vtlen - 6;

  userid = cuser.userid;
  str_ncpy(reason, log, maxlen);

  dir = xo->dir;
  hdr_fpath(fpath, dir, hdr);

  if (fp = fopen(fpath, "a"))
  {
    time_t now;
    struct tm *ptime;

    time(&now);
    ptime = localtime(&now);

    fprintf(fp, "\033[1;3%s\033[m \033[1;30m%s\033[m：\033[1;30m%-*s\033[1;30m%02d/%02d/%02d %02d:%02d:%02d\033[m\n",
      verb, userid, maxlen, reason,
      ptime->tm_year % 100, ptime->tm_mon + 1, ptime->tm_mday, ptime->tm_hour, ptime->tm_min, ptime->tm_sec);
    fclose(fp);
  }

    /* 問題待解: 若為 tag 轉錄, 則有可能 hdr 不在 pos 上, 每次都讓 rec_ref 從頭找起有損效率*/
    curraddscore = 0;

    currchrono = hdr->chrono;
    rec_ref(dir, hdr, sizeof(HDR), xo->key == XZ_XPOST ? hdr->xid : pos, cmpchrono, addscore);
    if (hdr->xmode & POST_BOTTOM)	/* 若是評分置底文章，去找正本來連動分數 */
    {
      currchrono = hdr->parent_chrono;
      rec_ref(dir, hdr, sizeof(HDR), 0, cmpchrono, addscore);
    }
    else				/* 若是評分一般文章，去找謄本來連動分數 */
    {
      rec_ref(dir, hdr, sizeof(HDR), 0, cmpparent, addscore);
    }

    return XO_LOAD;
}


#ifdef HAVE_ANONYMOUS
static void
log_anonyscore(fname, say)
  char *fname, *say;
{
  char buf[512];

  /* Thor.990113: 加上 fromhost 比較詳盡 */
  sprintf(buf, "%s %-13s(%s)\n%-13s %s 理由:%s\n",
	Now(), cuser.userid, fromhost, currboard, fname, say);
  f_cat(FN_RUN_ANONYMOUS, buf);
}
#endif

#define	MAX_REASON_RECORD	20

static int
post_append_score(xo, choose)
  XO *xo;
  int choose;	/* 0: 尚未選擇  >0: 已選擇的選項 */
{
  HDR *hdr;
  int pos, cur, ans, ans2, vtlen, maxlen;
  char *dir, *userid, *verb, fpath[64], reason[80];/*, vtbuf[12];*/
  char *prompt[3] = {"說的真好：", "聽你鬼扯：", "留一句話："};
  int  num_reason_record = 0;
  int  ip_len = 0;
  char my_ip[128] = {0};
  FILE *fp;
#ifdef HAVE_ANONYMOUS
  char uid[IDLEN + 1];
#endif
  time_t start;
  static time_t last = 0;

  if ((currbattr & BRD_NOSCORE) || !cuser.userlevel || !(bbstate & STAT_POST) )	/* 評分視同發表文章 */
    return XO_NONE;

  pos = xo->pos;
  cur = pos - xo->top;
  hdr = (HDR *) xo_pool + cur;

  if (hdr->xmode & POST_NOSCORE)
  {
    vmsg("這篇文章被設定為不能推文哦！");
    return XO_FOOT;
  }

#ifdef HAVE_REFUSEMARK
  if (!chkrestrict(hdr))
    return XO_NONE;
#endif

  time(&start);
  if ((start - last) < 5)
  {
    srand(start);
    vtlen = rand() % 10000;
    sprintf(fpath, "◎ 離上次推文時間過短，請輸入隨機數字 %4d: ",vtlen);
    vget(b_lines, 0, fpath, reason, 5, DOECHO);
    if (atoi(reason) != vtlen)
    {
      vmsg("輸入錯誤");
      return XO_FOOT;
    }
  }

  if (!(ans = choose))
  {
    switch (ans = vans("◎ 1)說的真好 2)聽你鬼扯 3)其他意見 [3] "))
    {
    case '1':
      verb = "1m△";
      vtlen = 2;
      break;

    case '2':
      verb = "2m▽";
      vtlen = 2;
      break;

    case '3':
      verb = "7m─";
      vtlen = 2;
      break;
      /* songsongboy.070124:lexel version*/
      /*if (!vget(b_lines, 0, "請輸入動詞：", fpath, 5, DOECHO))
	return XO_FOOT;
      vtlen = strlen(fpath);
      sprintf(verb = vtbuf, "%cm%s", ans - 2, fpath);
     break;*/

    default:
      ans = '3';
      verb = "7m─";
      vtlen = 2;
    }
  }
  else
  {
    ans = '3';
    verb = "7m─";
    vtlen = 2;
  }

  ip_len = (currbattr & BRD_POST_IP) ? 16 : 5;

  if (currbattr & BRD_POST_IP)
  {
#ifdef HAVE_ANONYMOUS
    if (currbattr & BRD_ANONYMOUS)
      strcpy(my_ip, " 才不告訴你呢");
    else
#endif
      sprintf(my_ip, " %s", get_my_ip());
  }
  else
  {
#ifdef HAVE_ANONYMOUS
    if (currbattr & BRD_ANONYMOUS)
      strcpy(my_ip, "\033[30m\033*/----");
    else
#endif
      sprintf(my_ip, "\033[30m\033*/%s", get_my_ansi_ip());
  }

#ifdef HAVE_ANONYMOUS
  if (currbattr & BRD_ANONYMOUS)
    maxlen = 63 - IDLEN - vtlen - ip_len;
  else
#endif
    maxlen = 63 - strlen(cuser.userid) - vtlen - ip_len;

  move(b_lines, 0);
  outs("請注意：推文將紀錄您的IP\n");
  if (!vget(b_lines - 1, 0, prompt[ans - '1'], reason, maxlen, DOECHO))
    return XO_HEAD;

  ans2 = vans("◎ Y)確定 N)取消 E)繼續 [Y] ");

  if (ans2 == 'n')
    return XO_HEAD;

  move(b_lines, 46);
  prints("(行數: %d/%d)\n", num_reason_record + 1, MAX_REASON_RECORD - num_reason_record - 1);

#ifdef HAVE_ANONYMOUS
  if (currbattr & BRD_ANONYMOUS)
  {
    userid = uid;
    if (!vget(b_lines, 0, "請輸入您想用的ID，也可直接按[Enter]，或是按[r]用真名：", userid, IDLEN, DOECHO))
      userid = STR_ANONYMOUS;
    else if (userid[0] == 'r' && userid[1] == '\0')
      userid = cuser.userid;
    else
      strcat(userid, ".");		/* 自定的話，最後加 '.' */
    maxlen = 63 - strlen(userid) - vtlen - ip_len;
  }
  else
#endif
    userid = cuser.userid;

  dir = xo->dir;
  hdr_fpath(fpath, dir, hdr);

  if (fp = fopen(fpath, "a"))
  {
    time_t now;
    struct tm *ptime;

    time(&now);
    ptime = localtime(&now);

    fprintf(fp, "\033[1;3%s \033[36m%s\033[m：\033[33m%-*s\033[30m\033*|\033[1m%02d/%02d %02d:%02d\033[m%s\n",
      verb, userid, maxlen - 1, reason,
      ptime->tm_mon + 1, ptime->tm_mday, ptime->tm_hour, ptime->tm_min, my_ip);

#ifdef HAVE_ANONYMOUS           /* 匿名推文記錄 */
  if (currbattr & BRD_ANONYMOUS && strcmp(userid, cuser.userid))
    log_anonyscore(hdr->xname, reason);
#endif

    num_reason_record ++;

    while ((ans2 == 'e') && (num_reason_record < MAX_REASON_RECORD))
    {
      time(&now);
      ptime = localtime(&now);

      if (!vget(b_lines-1, 0, prompt[ans - '1'], reason, maxlen, DOECHO))
	break;

      ans2 = vans("◎ Y)完成推文 N)此行重新輸入 E)繼續推文 [E] ");

      move(b_lines, 46);
      prints("(行數: %d/%d)\n", (ans2 == 'e') ? num_reason_record + 1 : num_reason_record,
				(ans2 == 'e') ? MAX_REASON_RECORD - num_reason_record - 1 : MAX_REASON_RECORD - num_reason_record);

      if (ans2 == 'n')
      {
	ans2 = 'e';
	continue;
      }

      fprintf(fp, "%-*s\033[33m%-*s\033[30m\033*|\033[1m%02d/%02d %02d:%02d\033[m%s\n",
	strlen(userid) + 5, "", maxlen - 1, reason,
	ptime->tm_mon + 1, ptime->tm_mday, ptime->tm_hour, ptime->tm_min, my_ip);

#ifdef HAVE_ANONYMOUS           /* 匿名推文記錄 */
      if (currbattr & BRD_ANONYMOUS && strcmp(userid, cuser.userid))
	log_anonyscore(hdr->xname, reason);
#endif
      num_reason_record ++;

      if (ans2 != 'y')
	ans2 = 'e';
    }

    fclose(fp);
  }

  curraddscore = ans == '1' ? 1 : ans == '2' ? -1 : 0;
  currchrono = hdr->chrono;
  change_stamp(dir, hdr);
  rec_ref(dir, hdr, sizeof(HDR), xo->key == XZ_XPOST ? hdr->xid : pos, cmpchrono, addscore);
  if (hdr->xmode & POST_BOTTOM)	/* 若是評分置底文章，去找正本來連動分數 */
  {
    currchrono = hdr->parent_chrono;
    rec_ref(dir, hdr, sizeof(HDR), 0, cmpchrono, addscore);
  }
  else				/* 若是評分一般文章，去找謄本來連動分數 */
  {
    rec_ref(dir, hdr, sizeof(HDR), 0, cmpparent, addscore);
  }
  post_history(xo, hdr);
  btime_update(currbno);
  last = start;

  return XO_INIT;
}


int
post_e_score(xo)
  XO *xo;
{
  return post_append_score(xo, 3);
}


int
post_score(xo)
  XO *xo;
{
  return post_append_score(xo, 0);
}


static int
post_noscore(xo)
  XO *xo;
{
  HDR *hdr;
  int pos, cur;

  if (!cuser.userlevel)	/* guest 不能對其他 guest 的文章設定 */
    return XO_NONE;

  pos = xo->pos;
  cur = pos - xo->top;
  hdr = (HDR *) xo_pool + cur;

  if ((bbstate & STAT_BOARD) || !strcmp(hdr->owner, cuser.userid))
  {
    hdr->xmode ^= POST_NOSCORE;
    currchrono = hdr->chrono;
    rec_put(xo->dir, hdr, sizeof(HDR), xo->key == XZ_XPOST ? hdr->xid : pos, cmpchrono);
    move(3 + cur, 7);
    outs(post_attr(hdr));
  }
  return XO_NONE;
}
#endif	/* HAVE_SCORE */


static int
post_noforward(xo)
  XO *xo;
{
  HDR *hdr;
  int pos, cur;

  if (!cuser.userlevel)	/* guest 不能對其他 guest 的文章設定 */
    return XO_NONE;

  pos = xo->pos;
  cur = pos - xo->top;
  hdr = (HDR *) xo_pool + cur;

  if (HAS_PERM(PERM_ALLBOARD) || !strcmp(hdr->owner, cuser.userid))
  {
    hdr->xmode ^= POST_NOFORWARD;
    currchrono = hdr->chrono;
    rec_put(xo->dir, hdr, sizeof(HDR), xo->key == XZ_XPOST ? hdr->xid : pos, cmpchrono);
    move(3 + cur, 7);
    outs(post_attr(hdr));
  }
  return XO_NONE;
}


static int
post_addMF(xo)
  XO *xo;
{
  MF mf;
  char fpath[64];

  if (!cuser.userlevel)
    return XO_NONE;

  if (!in_favor(currboard))
  {
    memset(&mf, 0, sizeof(MF));
    time4(&mf.chrono);
    mf.mftype = MF_BOARD;
    strcpy(mf.xname, currboard);

    mf_fpath(fpath, cuser.userid, FN_MF);
    rec_add(fpath, &mf, sizeof(MF));
    vmsg("已將此看板加入我的最愛");
  }
  else
  {
    vmsg("此看板已在最愛中。若要重覆加入，請進我的最愛裡新增");
  }

  return XO_FOOT;
}


static int
post_viewpal()
{
  XO *xt;
  char fpath[64];
  struct stat st;

  if (!cuser.userlevel)
    return 0;

  if ((currbattr & BRD_HIDEPAL) && !(bbstate & STAT_BM))
    return 0;

  brd_fpath(fpath, currboard, FN_PAL);
  if (!stat(fpath, &st) && S_ISREG(st.st_mode) && !st.st_size)	/* 有些名單的 size 為 0 */
    unlink(fpath);

  if (dashf(fpath))
  {
    xz[XZ_FAKE_PAL - XO_ZONE].xo = xt = xo_new(fpath);
    xt->key = PALTYPE_BPAL;
    xover(XZ_FAKE_PAL);
    free(xt);
    return 1;
  }

  vmsg("本板尚未設定板友名單");
  return 0;
}


static int
post_show_dog(fname)
  char *fname;
{
  FILE *fp;
  char fpath[64];

  int wi = 0;

  brd_fpath(fpath, currboard, fname);

  if (!(fp = fopen(fpath, "r")))
    return vmsg("目前無此設定！");

  clear();
  move(1, 0);
  prints("\033[1;33m %s \033[m\n\n", (!strcmp(fname, FN_NO_WRITE)) ? "發文推文條件限制" :
				!strcmp(fname, FN_NO_READ ) ? "進入看板條件限制" :
				!strcmp(fname, FN_NO_LIST ) ? "看板列表顯示本板" : "");

  fscanf(fp, "%d", &wi);
  prints("%s年齡限制 >= [%2d歲]\033[m\n", IS_BIGGER_AGE(wi) ? COLOR_ACP : COLOR_NOT_ACP , wi);

  fscanf(fp, "%d", &wi);
  prints("%s性別限制 : [%s] \033[m\n",
    (wi == 0) ? COLOR_ACP : (wi == cuser.sex + 1) ? COLOR_ACP : COLOR_NOT_ACP,
    (wi == 0) ? "不限" : (wi == 1) ? "中性" : (wi == 2) ? "男性" : "女性");

  fscanf(fp, "%d", &wi);
  prints("%s上線次數 >= [%d次] \033[m\n", (cuser.numlogins >= wi) ? COLOR_ACP : COLOR_NOT_ACP, wi);

  fscanf(fp, "%d", &wi);
  prints("%s文章篇數 >= [%d篇] \033[m\n", (cuser.numposts >= wi) ? COLOR_ACP : COLOR_NOT_ACP, wi);

  fscanf(fp, "%d", &wi);
  prints("%s優文篇數 >= [%d篇] \033[m\n", (cuser.good_article >= wi) ? COLOR_ACP : COLOR_NOT_ACP, wi);

  fscanf(fp, "%d", &wi);
  prints("%s劣文篇數 <  [%d篇] (0：不限) \033[m\n",
    (wi == 0) ? COLOR_ACP :(cuser.poor_article < wi) ? COLOR_ACP : COLOR_NOT_ACP, wi);

  fscanf(fp, "%d", &wi);
  prints("%s違規次數 <  [%d次] (0：不限) \033[m\n",
    (wi == 0) ? COLOR_ACP : (cuser.violation < wi) ? COLOR_ACP : COLOR_NOT_ACP, wi);

  fscanf(fp, "%d", &wi);
  prints("%s銀幣     >= [%d枚] \033[m\n", (cuser.money >= wi) ? COLOR_ACP : COLOR_NOT_ACP, wi);

  fscanf(fp, "%d", &wi);
  prints("%s金幣     >= [%d枚] \033[m\n", (cuser.gold >= wi) ? COLOR_ACP : COLOR_NOT_ACP, wi);

  fscanf(fp, "%d", &wi);
  prints("%s發信次數 >= [%d次] \033[m\n", (cuser.numemails >= wi) ? COLOR_ACP : COLOR_NOT_ACP, wi);

  fscanf(fp, "%d", &wi);
  prints("%s註冊時間 >= [%3d月] \033[m\n", IS_BIGGER_1STLG(wi) ? COLOR_ACP : COLOR_NOT_ACP, wi);

  fclose(fp);

  return vmsg(NULL);
}


#ifdef HAVE_RSS
static int post_rss();
#endif


static int
post_ishowbm(xo)
  XO *xo;
{
  BRD  *brd;
  brd = bshm->bcache + currbno;
  int isbm;
  int ch, reload = 0;
  char *mark;

  isbm = (bbstate & STAT_BM);
  if (isbm)
    mark = "\033[1;33m";
  else
    mark = "\033[0;30m";

  while (1)
  {
    currbattr = brd->battr;	/* post_manage()更改後尚未更新, 重新載入確保是最新的 */
    clear();
    move(0, 0);
    prints("\033[1;30m============================== "
      "\033[;37;44m[看板屬性設定]\033[;1;30m ===============================\033[m\n"
      "\033[32m按鍵 \033[1;30m修改類別  屬性                         "
      "\033[;32m按鍵 \033[1;30m修改類別  屬性\033[m\n");
    prints("   - 英文板名: %-13s                   - 看板分類: %s\n",
      brd->brdname, brd->class);
    prints(" %s%s - 中文板名: %s\n", mark, "b\033[m", brd->title);
    prints(" %s%s - 板主名單: %s\n", mark,
      (currbattr & BRD_PUBLIC) ? " \033[m" : "m\033[m", brd->BM);
    prints(" %s%s - 看板屬性: %s", mark,
      (currbattr & BRD_PUBLIC) ? " \033[m" : "i\033[m",
      (brd->readlevel == PERM_SYSOP) ? "秘密" :
      (brd->readlevel == PERM_BOARD) ? "好友" : "公開");
    prints("                          %s%s - 板友名單: %s\n", mark,
      (currbattr & BRD_PUBLIC) ? " \033[m" : "v\033[m",
      (currbattr & BRD_HIDEPAL) ? "板主隱藏板友名單" : "板友可看板友名單");

    prints(" %s%s - 鎖文限制: %s", mark,
      (currbattr & BRD_PUBLIC) ? " \033[m" : "x\033[m",
      (currbattr & BRD_NOL) ? "板主已設定板友不得鎖文" : "板主未做板友鎖文設定  ");
    prints("        %s%s - 可否推文: %s\n", mark, "e\033[m",
      (currbattr & BRD_NOSCORE) ? "否" : "可");

    prints(" %s%s - 轉錄設定: %s轉錄", mark, "f\033[m",
      (currbattr & BRD_NOFORWARD) ? "禁止" : "允許\");
    prints("                      %s%s - 轉錄文章%s顯示記錄\n", mark, "a\033[m",
      (currbattr & BRD_SHOWTURN) ? "" : "不");

    char buf[64];
    sprintf(buf, "%s%s%s",
      (currbattr & BRD_PUBLIC) ? "公眾看板" : "非公眾板",
      (currbattr & BRD_IAS) ? " | 藝文館看板" : "",
      (currbattr & BRD_ATOM) ? " | ATOM看板" : "");
    prints(" %s - 看板種類: %-30s",
      (currbattr & BRD_PUBLIC) ? "\033[1;33mu\033[m" : " ", buf);

    prints("%s%s - 看板推文顯示: %s\n", mark, "!\033[m",
      (currbattr & BRD_POST_IP) ? "ip" : "ip代碼");

    prints("   - 匿名看板: %s匿名%s",
      (currbattr & BRD_ANONYMOUS) ? "可" : "不可",
      (currbattr & BRD_ANONYMOUS) ? "  " : "");
    prints("                        - 活動舉辦: %s舉辦\n",
      (brd->bvote == 0) ? "無投票/賭盤" : (brd->bvote == -1) ? "有賭盤" : "有投票");

    prints("   - 是否轉信: %s轉信", (currbattr & BRD_NOTRAN) ? "不" : "有");
    prints("                          - 可否被 Z: %s\n",
      (currbattr & BRD_NOZAP) ? "不可" : "可");

    prints("   - 文章篇數: %s記錄", (currbattr & BRD_NOCOUNT) ? "不" : "有");
    prints("                          - 熱門話題: %s統計\n\n",
      (currbattr & BRD_NOSTAT) ? "不" : "參與");

    if (isbm)
    {
      prints("\033[1;30m============================== "
	"\033[;37;44m[編輯畫面名單]\033[;1;30m ===============================\033[m\n\n");
      prints(
	" %sw%s - 進板畫面     %so%s - 板友名單     %s%sk%s - 板友特別名單"
#ifdef HAVE_TEMPLATE
	"%s\n"
#else
	"\n"
#endif
#ifdef POST_PREFIX
	" %sp%s - 文章類別    "
#endif
	" %ss%s - 擋信列表     %sg%s - BBS 看門狗"
#ifdef HAVE_RSS
	"     %sr%s - RSS 設定"
#endif
	"\n",
	mark, "\033[m", mark, "\033[m",
#ifndef POST_PREFIX
	"  ",
#else
	"",
#endif
	mark, "\033[m",
#ifdef HAVE_TEMPLATE
	(currbattr & BRD_ATOM) ? "   \033[1;33mt\033[m - 文章範本" : "",
#endif
#ifdef POST_PREFIX
	mark, "\033[m",
#endif
	mark, "\033[m", mark, "\033[m",
#ifdef HAVE_RSS
	mark, "\033[m"
#endif
	);
    }

    move(b_lines - 5, 0);
    prints("\033[1;30m============================== "
      "\033[;37;44m[觀看看板設定]\033[;1;30m ===============================\033[m\n\n");
    prints(" %s1%s - 發文限制  %s2%s - 讀取限制  %s3%s - 顯示限制  ",
      "\033[1;36m", "\033[m", "\033[1;36m", "\033[m", "\033[1;36m", "\033[m");
#ifdef HAVE_RSS
    if (!isbm)
      prints("%s4%s - RSS 設定  %s", "\033[1;36m", "\033[m",
	!(currbattr & BRD_HIDEPAL) ? "\033[1;36m5\033[m - 板友名單" : "");
    else
#endif
      prints("%s4%s - 板友名單 ", "\033[1;36m", "\033[m");

    move(b_lines, 0);
    prints(COLOR1 " ◆ %-*s " COLOR2 " [或按其餘任意鍵離開] \033[m", 51 + d_cols, "請輸入上方各提示操作按鍵");
    ch = vkey();
    if (!isbm && (ch < '1' || ch > '5') && ch != 'u')
      return reload ? XO_INIT : XO_HEAD;

    if (isbm && (ch == '5'))	/* 板主不由觀看介面進 RSS 設定 */
      return reload ? XO_INIT : XO_HEAD;

    if (!(currbattr & BRD_PUBLIC) && ch == 'u')	/* 公眾板才看說明 */
      return reload ? XO_INIT : XO_HEAD;

    if (!(currbattr & BRD_ATOM) && ch == 't')	/* ATOM看板才能使用文章範本 */
      return reload ? XO_INIT : XO_HEAD;

    if ((currbattr & BRD_PUBLIC) &&
      (ch == 'm' || ch == 'i' || ch == 'x' || ch == 'v'))
      return reload ? XO_INIT : XO_HEAD;

    if (isbm && ch == '4')	/* 板主的 4 就是看板友名單 */
       ch = '5';

    move(b_lines, 0);
    clrtoeol();
    switch (ch |= 0x20)
    {
    case '1':
      post_show_dog(FN_NO_WRITE);
      break;
    case '2':
      post_show_dog(FN_NO_READ);
      break;
    case '3':
      post_show_dog(FN_NO_LIST);
      break;
#ifdef HAVE_RSS
    case '4':
    case 'r':
      reload = post_rss(xo);
      break;
#endif
    case '5':
      reload = post_viewpal() ? 1 : reload;
      break;
    case 'u':
      move(b_lines - 6, 0);
      clrtobot();
      prints("\033[1;30m%s\033[m\n   公眾板板主,\n"
	"       不得任意更改板主名單,看板公開/隱藏/好友設定\n"
	"       若需更改相關設定,請洽楓橋驛站站務部\n"
	"   公眾板使用者,\n"
	"       不得鎖文", MSG_SEPERATOR);
      vmsg(NULL);
      break;

    /* following are BM functions. */
    case 'b':
      DL_func("bin/manage.so:post_brdtitle");
      break;
    case 'm':
      DL_func("bin/manage.so:post_changeBM");
      break;
    case 'i':
      DL_func("bin/manage.so:post_brdlevel");
      break;
    case 'e':
      DL_func("bin/manage.so:post_battr_noscore");
      break;
    case 'x':
      DL_func("bin/manage.so:post_rlock");
      break;
    case 'v':
      reload = DL_func("bin/manage.so:post_vpal") ? 1 : reload;
      break;
    case 'f':
      DL_func("bin/manage.so:post_noforward");
      break;
    case 'a':
      DL_func("bin/manage.so:post_showturn");
      break;
    case '!':
      DL_func("bin/manage.so:post_brd_ip_char");
      break;
    case 'w':
      DL_func("bin/manage.so:post_memo_edit");
      break;
    case 'o':
      reload = DL_func("bin/manage.so:XoBM") ? 1 : reload;
      break;
    case 'k':
      reload = XoBM_add_pal() ? 1 : reload;
      break;
#ifdef HAVE_TEMPLATE
    case 't':
      DL_func("bin/manage.so:post_template_edit");
      break;
#endif
#ifdef POST_PREFIX
    case 'p':
      DL_func("bin/manage.so:post_brd_prefix");
      break;
#endif
    case 's':
      DL_func("bin/manage.so:post_spam_edit");
      break;
    case 'g':
      DL_func("bin/manage.so:post_guard_dog");
      break;
    default:
      return reload ? XO_INIT : XO_HEAD;
    }
  }

  return XO_NONE;	/* 以防萬一 */
}


static int
post_jxname(xo)	/* jump to xname */
  XO *xo;
{
  HDR *head;
  char xname[32], *px, *pb, *ptr;
  char folder[64], *fimage;
  int i, fsize, max;
  int bno = -1;
  static int j_status = 0;

  if (j_status)
    return XO_NONE;

  if (!vget(b_lines, 0, "檔案名稱：", px = xname, 12 + BNLEN + 1, DOECHO))
    return XO_FOOT;

  if (xname[0] == '#')
    px++;

  if (pb = strchr(px + 8, '@'))
  {
    *pb++ = '\0';
    while (*pb == ' ')
      pb++;
    if (!*pb)	/* 防止只輸入 @ */
      pb = NULL;
    else if (ptr = strchr(pb, ' '))
      *ptr = '\0';
  }

  if (pb && strcmp(pb, currboard))
  {
    if (((bno = brd_bno(pb)) >= 0) && (brd_bits[bno] & BRD_R_BIT))
      brd_fpath(folder, pb, FN_DIR);
    else	/* 沒有權限進入 或 錯誤的看板名稱 */
      return XO_FOOT;
  }
  else	/* 直接找現在所處的看板 */
    brd_fpath(folder, currboard, FN_DIR);

  if (ptr = strchr(px, ' '))
    *ptr = '\0';

  fimage = f_map(folder, &fsize);
  max = fsize / sizeof(HDR);
  if (fimage == (char *) -1)
  {
    vmsg("目前無法開啟索引檔");
    return XO_BODY;
  }

  head = (HDR *) fimage;
  for (i = 0; i < max; i++, head++)
  {
    if (!strcmp(head->xname, px))
    {
      munmap(fimage, fsize);
      if (bno >= 0)
      {
	int tmpbno = currbno;
	int tmppos = xo->pos;
	XoPost(bno);
	xz[XZ_POST - XO_ZONE].xo->pos = i;
	j_status++;
	xover(XZ_POST);
	j_status--;
	XoPost(tmpbno);
	xo = xz[XZ_POST - XO_ZONE].xo;
	xo->pos = tmppos;
	return XO_INIT;
      }
      else
      {
	xo->pos = i;
	return XO_LOAD;
      }
    }
  }
  munmap(fimage, fsize);

  vmsg("無此檔案");
  return XO_FOOT;
}


static int
post_info(xo)
  XO *xo;
{
  HDR *hdr;
  ACCT acct;
  int value;
  static int upid = -1;

#if 1
  if (cuser.ufo & BFLAG(6))
    cuser.ufo &= ~(BFLAG(6));
#endif

  hdr = (HDR *) xo_pool + (xo->pos - xo->top);
  value = atoi(hdr->value);

  move(0, 0);
  prints("檔案名稱：#%s @ %-*s", hdr->xname, BNLEN + 2, currboard);
  prints("作者：%-19.18s\n", hdr->owner);
#ifdef HAVE_REFUSEMARK
  if (!chkrestrict(hdr))
    prints("          %s\n", "<< 文章保密 >>");
  else
#endif
  {
    prints("文章標題：%s\n", hdr->title);

    if (hdr->xmode & POST_INCOME)
      prints("本篇為站外信，無文章價值");
    else if (acct_load(&acct, hdr->owner) >= 0)
      prints("文章價值：%d 銀", value);
    else if (value)	/* 找不到使用者但又有文章價值的，就表示為匿名文的 userno */
    {
      if (upid <= 0)
      {
	UTMP *ui;
	if (ui = (UTMP *) utmp_find(cuser.userno))
	  upid = ui->pid;
      }
      if (HAS_PERM(PERM_ALLBOARD))
	prints("匿名管理編號：%d", (upid > 0) ? value + upid : 0);
      else
	prints("本篇為匿名文章，無文章價值");
    }
    else
      prints("本篇為舊格式文章");

    if (hdr->xmode & POST_GOOD)
      prints(" | \033[1;33m優文標記\033[m");
    else if (hdr->xmode & POST_MARKED)
      prints(" | \033[1;36mmark標記\033[m");
    if ((bbstate & STAT_BOARD) && (hdr->xmode & POST_GEM))
      prints(" | \033[1;35m已收錄\033[m");
    if (hdr->xmode & POST_DELETE)
      prints(" | \033[1;32m待砍\033[m");
    if (hdr->parent_chrono == 1)
      prints(" | \033\[30;46m置底\033[m");
    if (hdr->xmode & POST_NOFORWARD)
      prints(" | \033[30;41m禁止轉錄\033[m");
    if (hdr->xmode & POST_NOSCORE)
      prints(" | \033[30;44m禁止推文\033[m");
  }

  if (HAS_PERM(PERM_ALLBOARD))
  {
    char fpath[64];
    struct stat st;

    hdr_fpath(fpath, xo->dir, hdr);

    if (!stat(fpath, &st))
    {
      time4_t temp = st.st_mtime;
      prints("\n時間戳記：%s  檔案大小：%d\n", Btime(&temp), st.st_size);
    }

    prints("發文日期：%s", hdr->date);
    prints("%-17s暱稱    ：%s\n", "", hdr->nick);
    prints("檔案位置：%s\n", fpath);

    prints("好友限定：%s", (hdr->xmode & POST_FRIEND) ? "是" : "否");

    prints("%-23s作者限定：%s\n", "", (hdr->xmode & POST_RESTRICT) ? "是" : "否");

    prints("可轉站外：%s", (hdr->xmode & POST_OUTGO) ? "是" : "否");
    prints("%-23s已被評分：%s",  "", (hdr->xmode & POST_SCORE) ? "是" : "否");
  }

  clrtoeol();
  move(xo->pos - xo->top + 3, 0);
  vkey();
  return post_head(xo);
}


#ifdef HAVE_RSS
static int
post_rss()
{
  more("gem/@/@rss.info", NULL);
  DL_func("bin/rss.so:rss_main");
  return 1;
}
#endif


static int
post_whereami(xo)
  XO *xo;
{
  FILE *fp;
  static char *index = "gem/@/@Class.index";
  char cmd[32], buf[256], *ptr;
  int end = 0, line = 6;

  if (!dashf(index))
  {
    vmsg("分類索引尚未建立，請至 sysop 板反應此問題 !!");
    return XO_NONE;
  }

  if ((currboard[0] == 'P') && (currboard[1] == '_'))
    vmsg("個人看板位於 (C)lass -> People 內，以下將搜尋其餘可能結果 !!");

  sprintf(cmd , "*%s ", currboard);

  move(1, 0);
  clrtobot();
  outs("我在哪裡？\n\n");
  if (fp = fopen(index, "r"))
  {
    while (fgets(buf, sizeof(buf), fp))
    {
      if (!strncmp(buf, cmd, strlen(cmd)))
      {
	if (line >= 20 && end == 0)
	{
	  line = 6;
	  move(b_lines - 1, 0);
	  outs("\033[1;44m 尚未顯示完畢，按任意鍵顯示下頁....\033[m");
	  vmsg(NULL);
	  move(3, 0);
	  clrtobot();
	}

	if (ptr = strstr(buf, "C:"))
	{
	  prints("%-*s", end * 2, "");
	  if (!strncmp(ptr + 2, currboard, strlen(currboard)) && *(ptr + 2 + strlen(currboard)) == ' ')
	  {
	    end = -1;
	    prints("%s\n", ptr + 2);
	  }
	  else
	    prints(ptr + 2);
	  end++;
	  line++;
	}
      }
    }
    fclose(fp);
    vmsg(NULL);
  }

  return XO_HEAD;
}


static int
post_ip_to_char()
{
  int i, ip1, ip2, ip3, ip4;
  char buf[4];

  move(0, 0);
  clrtobot();

  move(3, 0);

  prints("ip 以 ip1.ip2.ip3.ip4 表示 : \n");

  i = 5;

  if(!vget(i, 0, "請輸入ip1: ", buf, 4, DOECHO))
    return XO_INIT;

  ip1 = atoi(buf);
  if(ip1 > 255  || ip1 < 1)
  {
    vmsg("輸入不正確:  0 < ip1 < 256 ");
    return XO_INIT;
  }

  i=i+2;

  if(!vget(i, 0, "請輸入ip2: ", buf, 4, DOECHO))
    return XO_INIT;

  ip2 = atoi(buf);
  if(ip2 > 255  || ip2 < 1)
  {
    vmsg("輸入不正確:  0 < ip2 < 256 ");
    return XO_INIT;
  }

  i=i+2;

  if(!vget(i, 0, "請輸入ip3: ", buf, 4, DOECHO))
    return XO_INIT;

  ip3 = atoi(buf);
  if(ip3 > 255  || ip3 < 1)
  {
    vmsg("輸入不正確:  0 < ip3 < 256 ");
    return XO_INIT;
  }

  i=i+2;

  if(!vget(i, 0, "請輸入ip4: ", buf, 4, DOECHO))
    return XO_INIT;

  ip4 = atoi(buf);
  if(ip4 > 255  || ip4 < 1)
  {
    vmsg("輸入不正確:  0 < ip4 < 256 ");
    return XO_INIT;
  }

  prints("\n\n ip代碼=> %s \n", get_my_ansi_ip_char(ip1, ip2, ip3, ip4) );

  vmsg(NULL);

  return XO_HEAD;
}


static int
post_char_to_ip(xo)
  XO *xo;
{
  int ip1=0;
  int ip2=0;
  int ip3=0;
  int ip4=0;

  move(8, 0);
  clrtoeol();
  move(9, 0);
  clrtoeol();

  move(4, 0);
  clrtoeol();  
  prints("\033[1;33m 請依序輸入ip代碼以解出 \033[5m? \033[m");

  move(5, 0);
  clrtoeol();

  prints("\033[1;33;5m?\033[m.x.x.x");

  ip1 = get_my_ansi_char_ip(6);
  if(!ip1 || ip1>255 || ip1<1)
  {
    vmsg("輸入有誤 !!");
    return XO_INIT;
  }

  move(5, 0);
  clrtoeol();
  prints("\033[1;33m%d.\033[1;33;5m?\033[m.x.x", ip1);

  ip2 = get_my_ansi_char_ip(6);
  if(!ip2 || ip2>255 || ip2<1)
  {
    vmsg("輸入有誤 !!");
    return XO_INIT;
  }

  move(5, 0);
  clrtoeol();
  prints("\033[1;33m%d.%d.\033[1;33;5m?\033[m.x", ip1, ip2);

  ip3 = get_my_ansi_char_ip(6);
  if(!ip3 || ip3>255 || ip3<1)
  {
    vmsg("輸入有誤 !!");
    return XO_INIT;
  }

  move(5, 0);
  clrtoeol();
  prints("\033[1;33m%d.%d.%d.\033[1;33;5m?\033[m", ip1, ip2, ip3);

  ip4 = get_my_ansi_char_ip(6);
  if(!ip4 || ip4>255 || ip4<1)
  {
    vmsg("輸入有誤 !!");
    return XO_INIT;
  }

  move(5, 0);
  clrtoeol();
  prints("\033[1;33m%d.%d.%d.%d <===== 查詢結果\033[m", ip1, ip2, ip3, ip4);
  vmsg(NULL);

  return XO_HEAD;
}


int
post_trans_ip(xo)
  XO *xo;
{
  switch(vans("(1)以ip碼查詢ip  (2)以ip查詢ip碼 [Q]"))
  {
  case '1':
    return post_char_to_ip(xo);
  case '2':
    return post_ip_to_char(xo);
  default:
    return XO_HEAD;
  }
}


static int
post_help(xo)
  XO *xo;
{
  xo_help("post");
  /* return post_head(xo); */
  return XO_HEAD;		/* itoc.001029: 與 xpost_help 共用 */
}


KeyFunc post_cb[] =
{
#ifdef HAVE_LIGHTBAR
  XO_ITEM, post_item_bar,	/* verit.20030129 : 請務必將此放在第一項 */
#endif
  XO_INIT, post_init,
  XO_LOAD, post_load,
  XO_HEAD, post_head,
  XO_BODY, post_body,

  'W', post_whereami,
  'r', post_browse,
  's', post_switch,
  KEY_TAB, post_gem,
  'z', post_gem,
  'y', post_reply,
  'd', post_delete,
  'v', post_visit,
  'x', post_cross,		/* 在 post/mbox 中都是小寫 x 轉看板，大寫 X 轉使用者 */
  'X', post_forward,
  't', post_tag,
  'E', post_edit,
  'T', post_title,
  'm', post_mark,
  'M', post_mark_good,
  '_', post_bottom,
  'D', post_rangedel,
  'o', post_noforward,
#ifdef HAVE_SCORE
  '%', post_score,
  'e', post_e_score,
  Ctrl('E'), post_noscore,
#endif

  'w', post_write,

  'b', post_memo,
  'c', post_copy,
  'g', gem_gather,

#ifdef HAVE_REFUSEMARK
  'l', post_refuse,
#endif

#ifdef HAVE_LABELMARK
  'n', post_label,
  Ctrl('N'), post_delabel,
#endif

  '/', XOXpost_search_all,  /* smiler.070201: 搜尋功能整合 */
  '!', XoRXsearch,
  '~', XoXselect,		/* itoc.001220: 搜尋作者/標題 */
  'a', XoXauthor,		/* itoc.001220: 搜尋作者 */
  'S', XoXsearch,		/* itoc.001220: 搜尋相同標題文章 */
  'G', XoXmark,			/* itoc.010325: 搜尋 mark 文章 */

#if 0
  '~' | XO_DL, (int *)  "bin/dictd.so:main_dictd",
  '/', XoXtitle,		/* itoc.001220: 搜尋標題 */
  'f', XoXfull,			/* itoc.030608: 全文搜尋 */
  'K', XoXlocal,		/* itoc.010822: 搜尋本地文章 */
#endif

  Ctrl('P'), post_add,
  Ctrl('D'), post_prune,
  Ctrl('Q'), xo_uquery,
  Ctrl('O'), xo_usetup,
  'f', post_info,
  '#', post_jxname,

  Ctrl('F'), post_addMF,
  'i', post_ishowbm,
  'B', post_ishowbm,
  'R' | XO_DL, (void *) "bin/vote.so:vote_result",
  'V' | XO_DL, (void *) "bin/vote.so:XoVote",
  Ctrl('G') | XO_DL, (void *) "bin/xyz.so:post_sibala",

#ifdef HAVE_TERMINATOR
  Ctrl('X') | XO_DL, (void *) "bin/manage.so:post_terminator",
#endif

#ifdef HAVE_XYNEWS
  'u', XoNews,			/* itoc.010822: 新聞閱讀模式 */
#endif

  'h', post_help
};


KeyFunc xpost_cb[] =
{
#ifdef HAVE_LIGHTBAR
  XO_ITEM, post_item_bar,	/* verit.20030129 : 請務必將此放在第一項 */
#endif
  XO_INIT, xpost_init,
  XO_LOAD, xpost_load,
  XO_HEAD, xpost_head,
  XO_BODY, post_body,		/* Thor.980911: 共用即可 */

  'r', xpost_browse,
  'y', post_reply,
  't', post_tag,
  'x', post_cross,
  'X', post_forward,
  KEY_TAB, post_gem,
  'z', post_gem,
  'o', post_noforward,
  'c', post_copy,
  'g', gem_gather,
  'm', post_mark,
  'M', post_mark_good,
  'd', post_delete,		/* Thor.980911: 方便板主 */
  'E', post_edit,		/* itoc.010716: 提供 XPOST 中可以編輯標題、文章，加密 */
  'T', post_title,
#ifdef HAVE_SCORE
  '%', post_score,
  'e', post_e_score,
  Ctrl('E'), post_noscore,
#endif

  'w', post_write,

#ifdef HAVE_REFUSEMARK
  'l', post_refuse,
#endif

#ifdef HAVE_LABELMARK
  'n', post_label,
#endif

  '/', XOXpost_search_all,  /* smiler.070201: 搜尋功能整合 */
  '!', XoRXsearch,
  '~', XoXselect,
  'a', XoXauthor,
  'S', XoXsearch,
  'G', XoXmark,

#if 0
  '/', XoXtitle,
  'f', XoXfull,
  'K', XoXlocal,
#endif

  Ctrl('P'), post_add,
  Ctrl('D'), post_prune,
  Ctrl('Q'), xo_uquery,
  Ctrl('O'), xo_usetup,
  'f', post_info,

  'h', post_help		/* itoc.030511: 共用即可 */
};


#ifdef HAVE_XYNEWS
KeyFunc news_cb[] =
{
#ifdef HAVE_LIGHTBAR
  XO_ITEM, post_item_bar,      /* verit.20030129 : 請務必將此放在第一項 */
#endif
  XO_INIT, news_init,
  XO_LOAD, news_load,
  XO_HEAD, news_head,
  XO_BODY, post_body,

  'r', XoXsearch,

  'h', post_help		/* itoc.030511: 共用即可 */
};
#endif	/* HAVE_XYNEWS */
