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

void
str_lower_tmp(dst, src)
  char *dst, *src;
{
  int ch;

  do
  {
    ch = *src++;
    if (ch >= 'A' && ch <= 'Z')
      ch |= 0x20;
    *dst++ = ch;
  } while (ch);
}

int
cmpchrono(hdr)
  HDR *hdr;
{
  return hdr->chrono == currchrono;
}

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
RefusePal_fpath(fpath, board, mode, hdr)
  char *fpath;
  char *board;
  char mode;       /* 'C':Cansee  'R':Refimage */
  HDR *hdr;
{
  sprintf(fpath, "brd/%s/RefusePal_DIR/%s_%s",
    board, mode == 'C' ? "Cansee" : "Refimage", hdr->xname);
}
                                                                                
                                                                                
void
RefusePal_kill(board, hdr)   /* amaki.030322: 用來砍名單小檔 */
  char *board;
  HDR *hdr;
{
  char fpath[64];
                                                                                
  RefusePal_fpath(fpath, board, 'C', hdr);
  unlink(fpath);
  RefusePal_fpath(fpath, board, 'R', hdr);
  unlink(fpath);
}

int          /* 1:F文  0:普通文 -1:L文 */
RefusePal_level(board, hdr)       //smiler 1108
  char *board;
  HDR *hdr;
{
  int fsize;
  char fpath[64];
  int *fimage;
  FILE *fp;
                                                                                
  if(! hdr->xmode & POST_RESTRICT)
     return 0;
  else
  {
    RefusePal_fpath(fpath, board, 'C', hdr);
    if(fp=fopen(fpath,"r"))
    {
       fclose(fp);
       if (!strcmp(hdr->owner, cuser.userid) || (bbstate & STAT_BM))
            return 1;
       if (fimage = f_img(fpath, &fsize))
       {
        fsize = belong_pal(fimage, fsize / sizeof(int), cuser.userno);
        free(fimage);
        return fsize;
       }
    }
    else
     return -1;
  }
}

int          /* 1:在可見名單上 0:不在可見名單上 */
RefusePal_belong(board, hdr)
  char *board;
  HDR *hdr;
{
  int fsize;
  char fpath[64];
  int *fimage;
                                                                                
  if (!strcmp(hdr->owner, cuser.userid) || (bbstate & STAT_BM))
    return 1;
                                                                                
  RefusePal_fpath(fpath, board, 'C', hdr);  //smiler 1109
  if (fimage = f_img(fpath, &fsize))
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
XoBM_add_pal(xo)
  XO *xo;
{
  int ans;
  char fpath[64];
  XO *xt;

  if(! (bbstate & STAT_BM))
	  return XO_NONE;

  ans = vans("◎選擇板友特別名單1~8？[Q] ");
  if(ans<57 && ans >48)
  {
     sprintf(fpath, "brd/%s/friend_%c", currboard,ans);
     xz[XZ_PAL - XO_ZONE].xo = xt = xo_new(fpath);
     xt->key = PALTYPE_BPAL;        //smiler 1106
     xover(XZ_PAL);                                                                            
     free(xt);
     return XO_INIT;
  }
  else    
     return XO_FOOT;

}

static int
XoBM_Refuse_pal(xo)
  XO *xo;
{
  char fpath[64];
  HDR *hdr;
  XO *xt;
  int ans,ans2,ans3;
  char fpath_friend[64];
  XO *xr;
  char tmp[64],tmp2[64];
  FILE *fp;
  int pos, cur;              //smiler 1108
  pos = xo->pos;             //smiler 1108
  cur = pos - xo->top;       //smiler 1108
                                                                                
  hdr = (HDR *) xo_pool + (xo->pos - xo->top);
                                                                                
  if (!(hdr->xmode & POST_RESTRICT))
    return XO_NONE;
                                                                                
  if (strcmp(hdr->owner, cuser.userid) && !(bbstate & STAT_BM))
    return XO_NONE;

  sprintf(fpath, "brd/%s/RefusePal_DIR", currboard);
  if (!dashd(fpath))
    mkdir(fpath, 0700);
  RefusePal_fpath(fpath, currboard, 'C', hdr);

  ans=ans2=ans3=0;

  if((bbstate & STAT_BM))
      ans3 = vans("◎選擇 好友:1 板友:2 任意編輯名單:3 [Q] ");
  else
	  ans3 = 49;
  
  switch (ans3)
  {
  case 49:
    ans2 = vans("◎選擇 好友群組名單:1~5 任意編輯名單:6 [Q] ");
	if(ans2==54)
		ans=57;
	else if(ans2> 54 || ans2<49)
		ans=-1;
    break;

  case 50:
    ans = vans("◎選擇 板友名單:0 板友特別名單:1~8 任意編輯名單:9 [Q] ");
	if(ans>57 || ans<48)
		ans=-1;
    break;

  case 51:
    ans = 57;
    break;

  default:
    return XO_NONE;
  }

  if(ans==-1)
	  return XO_NONE;

  if(ans==0) //好友名單
  {
	if(ans2 < 54 && ans2 >48) //1~5
    {
		str_lower_tmp(tmp2,cuser.userid);
		sprintf(fpath_friend,"usr/%c/%s/list.%d",tmp2[0],tmp2,ans2-48);
	}
  }
  else      //板友名單
  {
    if(ans==48)
		sprintf(fpath_friend, "brd/%s/friend",currboard);
    else if(ans<57 && ans >48) //1~8
       sprintf(fpath_friend, "brd/%s/friend_%c", currboard,ans);
  }

    if(ans==57)
	{
         xz[XZ_PAL - XO_ZONE].xo = xt = xo_new(fpath);
         xt->key = PALTYPE_BPAL;        //smiler 1106
         xover(XZ_PAL);
         refusepal_cache(hdr, currboard);
         free(xt);
	}
	else
	{
         if(fp=fopen(fpath_friend,"r"))
            fclose(fp);
         else
		 {
            xz[XZ_PAL - XO_ZONE].xo = xr = xo_new(fpath_friend);
			 if(ans2==48)
				 xr->key = PALTYPE_PAL;
			 else if(ans2 > 48)
                 xr->key = PALTYPE_LIST;        //smiler 1106
			 else
				 xr->key = PALTYPE_BPAL;
            xover(XZ_PAL);
            free(xr);
		 }
         sprintf(tmp,"cp %s %s",fpath_friend,fpath);
         system(tmp);
         xz[XZ_PAL - XO_ZONE].xo = xt = xo_new(fpath);
         xt->key = PALTYPE_BPAL;        //smiler 1106
         xover(XZ_PAL);
         refusepal_cache(hdr, currboard);
         free(xt);
	}

    move(3 + cur, 7);              //smiler 1108
    outc('F');                     //smiler 1108
    return XO_INIT;
}

static int
post_viewpal(xo)
  XO *xo;
{
	XO *xt;
	FILE *fp;
	char fpath_org[64];
    char fpath_new[64];
	char tmp[64];

  if (!cuser.userlevel)
    return XO_NONE;
  
  if((!(bbstate & STAT_BM)) && (currbattr & BRD_SHOWPAL))
    return XO_NONE;

  sprintf(fpath_org,"brd/%s/friend",currboard);
  sprintf(fpath_new,"brd/%s/friend_%s",currboard,cuser.userid);
  sprintf(tmp,"cp %s %s",fpath_org,fpath_new);
  if(fp=fopen(fpath_org,"r"))
  {
	  fclose(fp);
	  system(tmp);
      xz[XZ_PAL - XO_ZONE].xo = xt = xo_new(fpath_new);
      xt->key = PALTYPE_BPAL;
      xover(XZ_PAL);
      free(xt);
	  unlink(fpath_new);
	  return XO_INIT;
  }
  else
  {
	  vmsg("本板尚未設定板友名單 !!");
	  return XO_NONE;
  }
}

static int
post_showbm(xo)
  XO *xo;
{
    BRD  *brd;
    FILE *fp;
	char file_path[64];
    brd = bshm->bcache + currbno;
    sprintf(file_path,"brd/%s/BM_%s",currboard,cuser.userid);
	fp=fopen(file_path,"w");    
	fprintf(fp,"看板英文板名: %s\n",brd->brdname);
	fprintf(fp,"看板分類    : %s\n",brd->class);
    fprintf(fp,"看板中文板名: %s\n",brd->title);
    fprintf(fp,"看板板主名單: %s\n",brd->BM);
	if(brd->bvote ==0)
	  fprintf(fp,"看板活動舉辦: 無投票舉辦\n");
	else if(brd->bvote == -1)
	  fprintf(fp,"看板活動舉辦: 有賭盤舉辦\n");
	else
	  fprintf(fp,"看板活動舉辦: 有投票舉辦\n");

//#define BRD_NOZAP   0x01    /* 不可 zap */
	fprintf(fp,"看板可否被z : ");
	if(currbattr & BRD_NOZAP)
		fprintf(fp,"不可\n");
	else
		fprintf(fp,"可\n");
//#define BRD_NOTRAN  0x02    /* 不轉信 */
	fprintf(fp,"看板可否轉信: ");
	if(currbattr & BRD_NOTRAN)
		fprintf(fp,"不轉信\n");
	else
		fprintf(fp,"可轉信\n");
//#define BRD_NOCOUNT 0x04    /* 不計文章發表篇數 */
	fprintf(fp,"文章發表篇數: ");
	if(currbattr & BRD_NOCOUNT)
		fprintf(fp,"不記錄\n");
	else
		fprintf(fp,"記錄\n");
//#define BRD_NOSTAT  0x08    /* 不納入熱門話題統計 */
	fprintf(fp,"熱門話題統計: ");
	if(currbattr & BRD_NOSTAT)
		fprintf(fp,"不記錄\n");
	else
		fprintf(fp,"記錄\n");
//#define BRD_NOVOTE  0x10    /* 不公佈投票結果於 [record] 板 */
	fprintf(fp,"投摽結果公佈: ");
	if(currbattr & BRD_NOVOTE)
		fprintf(fp,"不公佈投表結果於 [record] 板\n");
	else
		fprintf(fp,"公佈投票結果於 [record] 板\n");
//#define BRD_ANONYMOUS   0x20    /* 匿名看板 */
	fprintf(fp,"匿名看板   ?: ");
	if(currbattr & BRD_ANONYMOUS)
		fprintf(fp,"是\n");
	else
		fprintf(fp,"否\n");
//#define BRD_NOSCORE 0x40    /* 不評分看板 */
	fprintf(fp,"看板可否推文: ");
	if(currbattr & BRD_NOSCORE)
		fprintf(fp,"否\n");
	else
		fprintf(fp,"可\n");
//#define BRD_NOL     0x100   /* 不可鎖文 */
	fprintf(fp,"看板鎖文限制: ");
	if(currbattr & BRD_NOL)
		fprintf(fp,"板主已設定板友不得鎖文\n");
	else
		fprintf(fp,"板主未做板友鎖文設定\n");
//#define BRD_SHOWPAL 0x200   /* 顯示板友名單 */
	fprintf(fp,"顯示板友名單: ");
	if(currbattr & BRD_SHOWPAL)
		fprintf(fp,"板主隱藏板友名單\n");
	else
		fprintf(fp,"板友可看板友名單(ctrl^g)\n");
//#define BRD_PUBLIC  0x80    /* 公眾板 */
	fprintf(fp,"是否為公眾板: ");
	if(currbattr & BRD_PUBLIC)
	{
		fprintf(fp,"是公眾板\n");
		fprintf(fp,"\n===>\n");
		fprintf(fp,"   公眾板板主,\n");
		fprintf(fp,"     不得任意更改板主名單,看板公開/隱藏/好友設定\n");
		fprintf(fp,"     若需更改相關設定,請洽楓橋驛站站務部\n");
		fprintf(fp,"   公眾板使用者,\n");
		fprintf(fp,"     不得鎖文\n");
	}
	else
		fprintf(fp,"非公眾板\n");
	fclose(fp);
	more(file_path, NULL);
	unlink(file_path);
	return XO_HEAD;
}

/* smiler 0914*/
int
post_filter(acl)
  char *acl;			/* file name of access control list */
{
  FILE *fp;
  char filter[256];

  if (!(fp = fopen(acl, "r")))
    return -1;

  while (fgets(filter, sizeof(filter), fp))
  {
     if(strstr(filter,"paperdo@gmail.com") || strstr(filter,"http://paperdo.googlepages.com/index.htm")
		 || strstr(filter,"論文代寫") || strstr(filter,"代寫論文"))
		 return 1;
  }

  fclose(fp);
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
static 			/* 給 enews.c 用 */
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
  
  else if (acl_has(FN_ETC_NOCROSSPOST, "", cuser.userid) > 0)
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
      acct.tvalid = time(NULL) + CROSSPOST_DENY_DAY * 86400;
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


static int
do_post(xo, title)
  XO *xo;
  char *title;
{
  /* Thor.981105: 進入前需設好 curredit 及 quote_file */
  HDR hdr;
  char fpath[64], *folder, *nick, *rcpt;
  int mode;
  time_t spendtime;
  char buf_filepath[50];                       /* smiler.070914: for post_filter */
  HDR  hdr2;                                   /* smiler.070916: for 轉文至 nthu.forsale */
  char fpath2[64], folder2[64];                // smiler.070916
  char board_from[30];                         // smiler.070916

  if (!(bbstate & STAT_POST))
  {
#ifdef NEWUSER_LIMIT
    if (cuser.lastlogin - cuser.firstlogin < 3 * 86400)
      vmsg("新手上路，三日後始可張貼文章");
    else
#endif      
      vmsg("對不起，您沒有在此發表文章的權限");
    return XO_FOOT;
  }

  film_out(FILM_POST, 0);

  prints("發表文章於【 %s 】看板", currboard);

#ifdef POST_PREFIX
  /* 借用 mode、rcpt、fpath */

  if (title)
  {
    rcpt = NULL;
  }
  else		/* itoc.020113: 新文章選擇標題分類 */
  {
#define NUM_PREFIX 6
    char *prefix[NUM_PREFIX] = {"[公告] ", "[新聞] ", "[閒聊] ", "[文件] ", "[問題] ", "[測試] "};

    move(21, 0);
    outs("類別：");
    for (mode = 0; mode < NUM_PREFIX; mode++)
      prints("%d.%s", mode + 1, prefix[mode]);

    mode = vget(20, 0, "請選擇文章類別（按 Enter 跳過）：", fpath, 3, DOECHO) - '1';
    if (mode >= 0 && mode < NUM_PREFIX)		/* 輸入數字選項 */
      rcpt = prefix[mode];
    else					/* 空白跳過 */
      rcpt = NULL;
  }

  if (!ve_subject(21, title, rcpt))
#else
  if (!ve_subject(21, title, NULL))
#endif
      return XO_HEAD;

  /* 未具備 Internet 權限者，只能在站內發表文章 */
  /* Thor.990111: 沒轉信出去的看板, 也只能在站內發表文章 */

  if (!HAS_PERM(PERM_INTERNET) || (currbattr & BRD_NOTRAN))
    curredit &= ~EDIT_OUTGO;

  utmp_mode(M_POST);
  fpath[0] = '\0';
  time(&spendtime);
  if (vedit(fpath, 1) < 0)
  {
    unlink(fpath);
    vmsg(msg_cancel);
    return XO_HEAD;
  }
  spendtime = time(0) - spendtime;	/* itoc.010712: 總共花的時間(秒數) */

  /* build filename */

  folder = xo->dir;
  hdr_stamp(folder, HDR_LINK | 'A', &hdr, fpath);

  strcpy(buf_filepath,folder);                        // smiler.070914
  buf_filepath[strlen(buf_filepath)-4]=hdr.xname[7];  // smiler.070914
  buf_filepath[strlen(buf_filepath)-3]='/';           // smiler.070914
  buf_filepath[strlen(buf_filepath)-2]='\0';          // smiler.070914
  strcat(buf_filepath,hdr.xname);                     // smiler.070914

  strcpy(fpath2,fpath);                               // smiler.070916
  strcpy(folder2,folder);                             // smiler.070916
  folder2[4]='\0';                                    // smiler.070916
  strcat(folder2,"nthu.forsale/.DIR");                // smiler.070916 
  hdr_stamp(folder2, HDR_LINK | 'A', &hdr2, fpath2);  // smiler.070916
  strcpy(board_from,folder+4);                        // smiler.070916
  board_from[strlen(board_from)-5]='\0';              // smiler.070916

  /* set owner to anonymous for anonymous board */

#ifdef HAVE_ANONYMOUS
  /* Thor.980727: lkchu新增之[簡單的選擇性匿名功能] */
  if (curredit & EDIT_ANONYMOUS)
  {
    rcpt = anonymousid;	/* itoc.010717: 自定匿名 ID */
    nick = STR_ANONYMOUS;

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
  strcpy(hdr.owner, rcpt);
  strcpy(hdr.nick, nick);
  strcpy(hdr.title, title);

  hdr2.xmode = mode;                     // smiler.070916
  strcpy(hdr2.owner, rcpt);              // smiler.070916
  strcpy(hdr2.nick, nick);               // smiler.070916
  strcpy(hdr2.title, title);             // smiler.070916

#ifdef DO_POST_FILTER
  if(post_filter(buf_filepath) ==1)       /* smiler 0914 */
  {
    unlink(buf_filepath);
    vmsg("請勿大量洗板,洗站,廣告 !!");
    return XO_HEAD;
  }
#endif
  rec_bot(folder, &hdr, sizeof(HDR));
  btime_update(currbno);

  /* smiler 0916 */
  if(strstr(hdr2.title,"賣") || strstr(hdr2.title,"售") || strstr(hdr2.title,"出清"))
  {
	 if( (!strstr(board_from,"P_")) && (!strstr(board_from,"R_")) && 
	   (!strstr(board_from,"LAB_")) && (!strstr(board_from,"G_")) )
	 {
          rec_bot(folder2, &hdr2, sizeof(HDR));
          btime_update(brd_bno("nthu.forsale"));
	 }
  }


  if (mode & POST_OUTGO)
    outgo_post(&hdr, currboard);

  post_history(xo, &hdr);

  clear();
  outs("順利貼出文章，");

  if (currbattr & BRD_NOCOUNT || wordsnum < 30)
  {				/* itoc.010408: 以此減少灌水現象 */
    outs("文章不列入紀錄，敬請包涵。");
  }
  else
  {
    /* itoc.010408: 依文章長度/所費時間來決定要給多少錢；幣制才會有意義 */
    mode = BMIN(wordsnum, spendtime) / 10;	/* 每十字/秒 一元 */
    prints("這是您的第 %d 篇文章，得 %d 銀。", ++cuser.numposts, mode);
    addmoney(mode);
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

  unlink(fpath);

  vmsg(NULL);

  return XO_INIT;
}


int
do_reply(xo, hdr)
  XO *xo;
  HDR *hdr;
{
  curredit = 0;

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


static int
post_reply(xo)
  XO *xo;
{
  if (bbstate & STAT_POST)
  {
    HDR *hdr;

    hdr = (HDR *) xo_pool + (xo->pos - xo->top);

#ifdef HAVE_REFUSEMARK
    if ((hdr->xmode & POST_RESTRICT) && !RefusePal_belong(currboard, hdr))
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
static inline int
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
  static char *type[6] = {"Re", "Fw", "◇", "\033[1;33m=>", "\033[1;33m=>", "\033[1;32m◆"};
  uschar *title, *mark;
  int ch, len;
  int in_chi;		/* 1: 在中文字中 */
  char title_tmp[64];  //smiler 1108
  int chi_detect;      //smiler 071114
  int i;               //smiler 071114
#ifdef HAVE_DECLARE
  int square;		/* 1: 要處理方括 */
#endif
#ifdef CHECK_ONLINE
  UTMP *online;
#endif

   cc=cc-1; /*smiler.070724: 標題少需印一格 */
  /* --------------------------------------------------- */
  /* 印出日期						 */
  /* --------------------------------------------------- */

#ifdef HAVE_DECLARE
  /* itoc.010217: 改用星期幾來上色 */
  /*smiler.070724: 日期印出前,多空一格*/
  prints(" \033[1;3%dm%s\033[m ", cal_day(hdr->date) + 1, hdr->date + 3);

#else
  prints(" \033[m%s\033[m ",hdr->date + 3);
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
  if((hdr->xmode & POST_RESTRICT)) //smiler 1108
  {
      strcpy(title_tmp,"<< 文章保密 >>"); //smiler 1108
      title = str_ttl(mark = title_tmp);
  }
  else
     title = str_ttl(mark = hdr->title);

  len = (title == mark) ? 2 : (*mark == 'R') ? 0 : 1;
  if (!strcmp(currtitle, title))
    len += 3;
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
  while ((ch = *title++) && (title < mark-3))
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

  if(title == mark-3){

    if(strlen(title) < 4){
      outc(ch);
      while (ch = *title++)
	outc(ch);
    }
    else
      outs(" ...");
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
hdr_outs_bar(hdr, cc)           /* print HDR's subject */
  HDR *hdr;
  int cc;               /* 印出最多 cc - 1 個字 */
{

  /* 回覆/轉錄/原創/閱讀中的同主題回覆/閱讀中的同主題轉錄/閱讀中的同主題原創 */
  static char *type[6] = {"Re", "Fw", "◇", "\033[1;33m=>", "\033[1;33m=>", "\033[1;32m◆"};
  uschar *title, *mark;
  int ch, len;
  int in_chi;		/* 1: 在中文字中 */
  char title_tmp[64];  //smiler 1108
  int chi_detect;      //smiler 071114
  int i;               //smiler 071114
#ifdef HAVE_DECLARE
  int square;		/* 1: 要處理方括 */
#endif
#ifdef CHECK_ONLINE
  UTMP *online;
#endif

   cc=cc-1; /*smiler.070724: 標題少需印一格 */
  /* --------------------------------------------------- */
  /* 印出日期						 */
  /* --------------------------------------------------- */

#ifdef HAVE_DECLARE
  /* itoc.010217: 改用星期幾來上色 */
  /*smiler.070724: 日期印出前,多空一格*/
  prints(COLORBAR_POST" \033[1;3%dm%s\033[m"COLORBAR_POST" ", cal_day(hdr->date) + 1, hdr->date + 3);

#else
  prints(COLORBAR_POST" \033[m%s\033[m"COLORBAR_POST" ",hdr->date + 3);
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
  if((hdr->xmode & POST_RESTRICT)) //smiler 1108
  {
      strcpy(title_tmp,"<< 文章保密 >>"); //smiler 1108
      title = str_ttl(mark = title_tmp);
  }
  else
     title = str_ttl(mark = hdr->title);

  len = (title == mark) ? 2 : (*mark == 'R') ? 0 : 1;
  if (!strcmp(currtitle, title))
    len += 3;
  prints(COLORBAR_POST);
  outs(type[len]);
  prints(COLORBAR_POST);
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

  prints(COLORBAR_POST);

  /* 把超過 cc 長度的部分直接切掉 */
  /* itoc.060604.註解: 如果剛好切在中文字的一半就會出現亂碼，不過這情況很少發生，所以就不管了 */
  while ((ch = *title++) && (title < mark-3))
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
	outs("]\033[m"COLORBAR_POST);
	square = 0;			/* 只處理一組方括，方括已經處理完了 */
	continue;
      }
    }
#endif

    outc(ch);
  }

  if(title == mark-3){

    if(strlen(title) < 4){
      outc(ch);
      while (ch = *title++)
        outc(ch);
     }  
     else{
        outs(" ...");
        title += 4;
	    }
  }
#if 1
  while(title <= mark)
  {
	  prints(" ");
	  title++;
  }
#endif

#ifdef HAVE_DECLARE
  if (square || len >= 3)	/* Thor.980508: 變色還原用 */
#else
  if (len >= 3)
#endif
    outs("\033[m");

  prints("\033[m");
  //outc('\n');
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


static int
post_attr(hdr)
  HDR *hdr;
{
  int mode, attr;

  mode = hdr->xmode;

  /* 已閱讀為小寫，未閱讀為大寫 */
  /* 由於置底文沒有閱讀記錄，所以視為已讀 */
  /* 加密文章視為已讀 */
#ifdef HAVE_REFUSEMARK
  attr = ((mode & POST_BOTTOM) || !brh_unread(BMAX(hdr->chrono, hdr->stamp)) || 
    ((mode & POST_RESTRICT) && strcmp(hdr->owner, cuser.userid) && !(bbstate & STAT_BM))) ? 0x20 : 0;
#else
  attr = ((mode & POST_BOTTOM) || !brh_unread(BMAX(hdr->chrono, hdr->stamp)) ? 0x20 : 0;
#endif

#ifdef HAVE_REFUSEMARK
  if (mode & POST_RESTRICT)
  {
   if (RefusePal_level(currboard, hdr)==1)
       attr |= 'F';
   else
    attr |= 'L';
  }
  else
#endif
#ifdef HAVE_LABELMARK
  if (mode & POST_DELETE)
    attr |= 'T';
  else
#endif
  if (mode & POST_NOFORWARD)
    attr |= 'X';
  else if (mode & POST_NOSCORE)
    attr |= 'N';
  else if (mode & POST_MARKED)
    attr |= 'M';
  else if (!attr)
    attr = '+';

  return attr;
}


static void
post_item(num, hdr)
  int num;
  HDR *hdr;
{
#ifdef HAVE_SCORE
  if(!(cuser.ufo & UFO_FILENAME))
  {  
   if(hdr->xmode & POST_BOTTOM)
     prints("  \033[1;33m重要  \033[m");
   else
     prints("%6d%c%c", (hdr->xmode & POST_BOTTOM) ? -1 : num, tag_char(hdr->chrono), post_attr(hdr));
   if (hdr->xmode & POST_SCORE)
   {
     num = hdr->score;
     if (num <= 99 && num >= -99)
       prints("\033[%c;3%cm%2d\033[m", num == 0 ? '0' : '1', num > 0 ? '1' : num < 0 ? '2' : '7' , abs(num));
     else
       prints("\033[1;3%s\033[m", num >= 0 ? "1m爆" : "2m噓");
   }
   else
   {
     outs("  ");
   }
  }
  else
   prints("%10s",hdr->xname);

  hdr_outs(hdr, d_cols + 46);	/* 少一格來放分數 */
#else
 if(!(cuser.ufo & UFO_FILENAME))
 {
   /*ckm.070325: 置底文沒有編號*/
   if(hdr->xmode & POST_BOTTOM)
     prints("  \033[1;33m重要\033[m  ");
   else
     prints("%6d%c%c ", (hdr->xmode & POST_BOTTOM) ? -1 : num, tag_char(hdr->chrono), post_attr(hdr));
 }
 else
   prints("%10s",hdr->xname);    
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
  /*static char scorelist[36] =
  {
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',
    'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
    'U', 'V', 'W', 'X', 'Y', 'Z'
  };*/
                                                                                
  hdr = (HDR *) xo_pool + xo->pos - xo->top;
  num = xo->pos + 1;

 if(!(cuser.ufo & UFO_FILENAME))
 {
  
   if(hdr->xmode & POST_BOTTOM)
   {
   prints("%s%s%s%c%c",
     mode ? COLORBAR_POST : "",
     "  \033[1;33m重要\033[m",mode ? COLORBAR_POST : "",
     tag_char(hdr->chrono), post_attr(hdr));
   }
   else
   {
   prints("%s%6d%c%c",
     mode ? COLORBAR_POST : "",
     num,
     tag_char(hdr->chrono), post_attr(hdr));
   } 


   if (hdr->xmode & POST_SCORE)
   {
     //num = hdr->score;
     num = hdr->score;
     if (num <= 99 && num >= -99)
         prints("%s\033[%c;3%cm%s%2d\033[m%s",mode ? COLORBAR_POST : "", '1', num > 0 ? '1' : num < 0 ? '2' : '7' ,mode ? COLORBAR_POST : num > 0 ? "\033[m\033[1;31m" : num < 0 ? "\033[m\033[1;32m" : "\033[m\033[1;37m" , abs(num),mode ? COLORBAR_POST : "");
     else
       prints("%s\033[1;3%s\033[m%s",mode ? COLORBAR_POST : "", num >= 0 ? "1m爆" : "2m噓",mode ? COLORBAR_POST : "");
   }
   else
   {
     outs("  ");
   }
 }
 else
   prints("%s%10s",mode ? COLORBAR_POST : "",hdr->xname);
                                                                                
  if (mode)
    hdr_outs_bar(hdr, 46);    /* 少一格來放分數 */
  else
    hdr_outs(hdr, 46);
#else
  hdr = (HDR *) xo_pool + xo->pos - xo->top;
  num = xo->pos + 1;
  if(!(cuser.ufo & UFO_FILENAME))
  {
   if(hdr->xmode & POST_BOTTOM)
   {
     prints("%s%s%s%c%c ",
     mode ? COLORBAR_POST : "",
     "  \033[1;33m重要\033[m",mode ? COLORBAR_POST : "",
     tag_char(hdr->chrono), post_attr(hdr));
   }
   else
   {
     prints("%s%6d%c%c ",
     mode ? COLORBAR_POST : "",
     num,
     tag_char(hdr->chrono), post_attr(hdr));
   }
  }
  else
    prints("%s%10s",mode ? COLORBAR_POST : "",hdr->xname);
                                                                                
  if (mode)
    hdr_outs_bar(hdr, 47);
  else
    hdr_outs(hdr, 47);
#endif
                                                                                
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
  if(!(cuser.ufo & UFO_FILENAME))
    prints(NECKER_POST, d_cols, "", currbattr & BRD_NOSCORE ? "╳" : "○", bshm->mantime[currbno]);
  else
    prints(NECKER_POST_FILE, d_cols, "", currbattr & BRD_NOSCORE ? "╳" : "○", bshm->mantime[currbno]);
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

    hdr = (HDR *) xo_pool;
    row = 3;
    do
    {
      move(row, 7);
      outc(post_attr(hdr++));
    } while (++row < max);
  }
  return XO_FOOT;
}


void
post_history(xo, hdr)          /* 將 hdr 這篇加入 brh */
  XO *xo;
  HDR *hdr;
{
  int fd;
  time_t prev, chrono, next, this;
  HDR buf;
                                                                                
  chrono = BMAX(hdr->chrono, hdr->stamp);
  if (!brh_unread(chrono))      /* 如果已在 brh 中，就無需動作 */
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
                                                                                
    if (prev > chrono)      /* 沒有下一篇 */
      prev = chrono;
    if (next < chrono)      /* 沒有上一篇 */
      next = chrono;
                                                                                
    brh_add(prev, chrono, next);
  }
}



static int
post_browse(xo)
  XO *xo;
{
  HDR *hdr;
  int xmode, pos, key;
  char *dir, fpath[64];

  dir = xo->dir;

  for (;;)
  {
    pos = xo->pos;
    hdr = (HDR *) xo_pool + (pos - xo->top);
    xmode = hdr->xmode;

#ifdef HAVE_REFUSEMARK
    if ((xmode & POST_RESTRICT) && !RefusePal_belong(currboard, hdr))
      break;
#endif

    hdr_fpath(fpath, dir, hdr);

    /* Thor.990204: 為考慮more 傳回值 */   
    if ((key = more(fpath, FOOTER_POST)) < 0)
      break;

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
    case '%':
      post_score(xo);
      return post_init(xo);
#endif

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
  return post_init(xo);
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

  /*  解決信箱轉寄問題 */
  int comefrom;          // 0: 從信箱轉寄 1: 從看板轉寄
  char mail_path_tmp[64];
  char userid_tmp[15];

  str_lower_tmp(userid_tmp,cuser.userid);
  sprintf(mail_path_tmp,"usr/%c/%s/.DIR",userid_tmp[0],userid_tmp);
  if(!strcmp(mail_path_tmp,xo->dir))
	  comefrom=0;
  else
	  comefrom=1;


  if (!cuser.userlevel)	/* itoc.000213: 避免 guest 轉錄去 sysop 板 */
    return XO_NONE;

#ifdef HAVE_REFUSEMARK
  pos = xo->pos;             //smiler 1108
  cur = pos - xo->top;       //smiler 1108
  hdr_org = (HDR *) xo_pool + (xo->pos - xo->top);
  //if(RefusePal_level(currboard, hdr_org)!=0)  //若為L文及F文,僅板主及作者可轉錄
  if(hdr_org->xmode & POST_RESTRICT)
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
    (vget(2, 0, "(1)原文轉載 (2)轉錄文章？[2] ", buf, 3, DOECHO) == '1') ? 0 : 1; /* smiler.070602: 改預設為2 */

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

	if(comefrom)                    /* smiler.071114: 需為處在看板,下面幾行才需作判斷 */
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

  vmsg("轉錄完成");
  return XO_HEAD;
}


int
post_forward(xo)
  XO *xo;
{
  ACCT muser;
  int pos;
  HDR *hdr;

  if (!HAS_PERM(PERM_LOCAL))
    return XO_NONE;

  pos = xo->pos;
  hdr = (HDR *) xo_pool + (pos - xo->top);

  if (hdr->xmode & GEM_FOLDER)	/* 非 plain text 不能轉 */
    return XO_NONE;

#ifdef HAVE_REFUSEMARK
//  if ((hdr->xmode & POST_RESTRICT) && !RefusePal_belong(currboard, hdr))
//    return XO_NONE;
  //if(RefusePal_level(currboard, hdr)!=0)  //若為L文及F文,僅板主及作者可轉錄
  if (hdr->xmode & POST_RESTRICT)
  {
  if (strcmp(hdr->owner, cuser.userid) && !(bbstate & STAT_BM))
    return XO_NONE;
  }
#endif

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

    hdr->xmode = xmode ^ POST_MARKED;
    currchrono = hdr->chrono;
    rec_put(xo->dir, hdr, sizeof(HDR), xo->key == XZ_XPOST ? hdr->xid : pos, cmpchrono);

    move(3 + cur, 7);
    outc(post_attr(hdr));
  }
  return XO_NONE;
}


/*ryancid: reset parent_chrono*/
static void
reset_parent_chrono(hdd, ram)
  HDR *hdd, *ram;
{
  hdd->parent_chrono=0;
}

/*ryancid: set parent_chrono*/
static void
set_parent_chrono(hdd, ram)
  HDR *hdd, *ram;
{
  hdd->parent_chrono=1;
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
  
  if(!(hdr->xmode & POST_BOTTOM))
  {
    /*Allow only one bottom post per article*/	
    if(hdr->parent_chrono!=0)
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
    if(hdr->xmode & POST_SCORE)
    {
      post.xmode |= POST_SCORE;
      post.score = hdr->score;
    }
    /**/

    strcpy(post.owner, hdr->owner);
    strcpy(post.nick, hdr->nick);
    strcpy(post.title, hdr->title);

    rec_add(xo->dir, &post, sizeof(HDR));
    /**/
    currchrono = hdr -> chrono;
    rec_ref(xo->dir, hdr, sizeof(HDR), \
	xo->key == XZ_XPOST ? hdr->xid : pos, cmpchrono ,\
	set_parent_chrono);/*ryancid: set the parent_chrono*/
  }
  else
  {
    /**/
    currchrono = hdr->parent_chrono;
    rec_ref(xo->dir, hdr, sizeof(HDR), \
	xo->key == XZ_XPOST ? hdr->xid : pos, cmpchrono ,\
	reset_parent_chrono);/*reset the original chrono*/

    currchrono = hdr->chrono;
    
    if (!rec_del(xo->dir, sizeof(HDR), xo->key == XZ_XPOST ? hdr->xid : pos, cmpchrono))
    {
      pos = move_post(hdr, xo->dir, bbstate & STAT_BOARD);
    }
  }
  
  return post_load(xo); /* ckm.070325: 重新載入列表 */         
}


#ifdef HAVE_REFUSEMARK
static int
post_friend(xo)
  XO *xo;
{
  HDR *hdr;
  int pos, cur;
                                                                                
  if (!cuser.userlevel) /* itoc.020114: guest 不能對其他 guest 的文章加密 */
    return XO_NONE;
 
  if((currbattr & BRD_PUBLIC))
	  return XO_NONE;
  if((!(bbstate & STAT_BM)) && (currbattr & BRD_NOL))
    return XO_NONE;
  
  pos = xo->pos;
  cur = pos - xo->top;
  hdr = (HDR *) xo_pool + cur;
                                                                                
  if (!strcmp(hdr->owner, cuser.userid) || (bbstate & STAT_BM))
  {
   if (hdr->xmode & POST_RESTRICT)
   {
     if(RefusePal_level(currboard, hdr)== (-1))  //L文不可按l取消
         return XO_NONE;
#if 0
     if (vans("解除文章保密會刪除全部可見名單，您確定嗎(Y/N)？[N] ") != 'y')
     {
       move(3 + cur, 7);
       outc(post_attr(hdr));
       return XO_FOOT;
     }
#endif
     RefusePal_kill(currboard, hdr);
                                                                                
   }
                                                                                
                                                                                
    hdr->xmode ^= POST_RESTRICT;
	hdr->xmode ^= POST_FRIEND;      /* smiler 1108 */
    currchrono = hdr->chrono;
    rec_put(xo->dir, hdr, sizeof(HDR), xo->key == XZ_XPOST ?
hdr->xid : pos, cmpchrono);
                                                                                
   if (hdr->xmode & POST_RESTRICT)
     return XoBM_Refuse_pal(xo);
                                                                                
    move(3 + cur, 7);
    outc(post_attr(hdr));
  }
  return XO_NONE;
                                                                                
}
                                                                                
                                                                                
static int
post_refuse(xo)     /* itoc.010602: 文章加密 */
  XO *xo;
{
  HDR *hdr;
  int pos, cur;
                                                                                
  if (!cuser.userlevel) /* itoc.020114: guest 不能對其他 guest 的文章加密 */
    return XO_NONE;
  
  if((currbattr & BRD_PUBLIC))
	  return XO_NONE;
  if((!(bbstate & STAT_BM)) && (currbattr & BRD_NOL) )
    return XO_NONE;
  
  pos = xo->pos;
  cur = pos - xo->top;
  hdr = (HDR *) xo_pool + cur;
                                                                                
  if (!strcmp(hdr->owner, cuser.userid) || (bbstate & STAT_BM))
  {
   if (hdr->xmode & POST_RESTRICT)
   {
       if(RefusePal_level(currboard, hdr)==1)  //F文不可按L取消
           return XO_NONE;
   }
                                                                                
    hdr->xmode ^= POST_RESTRICT;
    currchrono = hdr->chrono;
    rec_put(xo->dir, hdr, sizeof(HDR), xo->key == XZ_XPOST ?
hdr->xid : pos, cmpchrono);
                                                                                
    move(3 + cur, 7);
    outc(post_attr(hdr));                                                                                
  }
                                                                                
  return XO_NONE;
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
    outc(post_attr(hdr));

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

/* 單一刪文 */
static int
post_delete(xo)
  XO *xo;
{
  int pos, cur, by_BM;
  HDR *hdr;
  char buf[80];
  char Deletelog_folder[64],copied[64];
  HDR  Deletelog_hdr;

  /* smiler.1111: 保護Deletelog 以及 Editlog 的 記錄不被移除 */
 if((!strcmp(xo->dir,"brd/Deletelog/.DIR")) || (!strcmp(xo->dir,"brd/Editlog/.DIR")))
   return XO_NONE;

  if (!cuser.userlevel ||
    !strcmp(currboard, BN_DELETED) ||
    !strcmp(currboard, BN_JUNK))
    return XO_NONE;

  pos = xo->pos;
  cur = pos - xo->top;
  hdr = (HDR *) xo_pool + cur;

  if ((hdr->xmode & POST_BOTTOM) || (hdr->xmode & POST_MARKED) ||
    (!(bbstate & STAT_BOARD) && strcmp(hdr->owner, cuser.userid)))
    return XO_NONE;

  by_BM = bbstate & STAT_BOARD;

  if (vans(msg_del_ny) == 'y')
  {
    /* smiler 1031 */
    if(deletelog_use)
	{
      hdr_fpath(copied,xo->dir,hdr);
      brd_fpath(Deletelog_folder,"Deletelog", FN_DIR);
      hdr_stamp(Deletelog_folder, HDR_COPY | 'A', &Deletelog_hdr, copied);                                                                                
      strcpy(Deletelog_hdr.title , hdr->title);
      strcpy(Deletelog_hdr.owner , cuser.userid);
      strcpy(Deletelog_hdr.nick  , cuser.username);
      Deletelog_hdr.xmode = POST_OUTGO;
      rec_bot(Deletelog_folder, &Deletelog_hdr, sizeof(HDR));
      btime_update(brd_bno("Deletelog"));
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
  char Deletelog_folder[64],Deletelog_title[64],copied[64];
  HDR  Deletelog_hdr;

//  /* smiler.1111: 保護Deletelog 以及 Editlog 的 記錄不被移除 */
// if((!strcmp(xo->dir,"brd/Deletelog/.DIR")) || (!strcmp(xo->dir,"brd/Editlog/.DIR")))
//   return XO_NONE;

  if(deletelog_use)
  {
   hdr_fpath(copied,xo->dir,hdr);
   brd_fpath(Deletelog_folder,"Deletelog", FN_DIR);
   hdr_stamp(Deletelog_folder, HDR_COPY | 'A', &Deletelog_hdr, copied);
   strcpy(Deletelog_hdr.title , hdr->title);
   strcpy(Deletelog_hdr.owner , cuser.userid);
   strcpy(Deletelog_hdr.nick  , cuser.username);
   Deletelog_hdr.xmode = POST_OUTGO;
   rec_bot(Deletelog_folder, &Deletelog_hdr, sizeof(HDR));
   btime_update(brd_bno("Deletelog"));
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
  if((!strcmp(xo->dir,"brd/Deletelog/.DIR")) || (!strcmp(xo->dir,"brd/Editlog/.DIR")))
    return XO_NONE;

  btime_update(currbno);

  return xo_rangedel(xo, sizeof(HDR), chkpost, delpost);
}


static int
post_prune(xo)
  XO *xo;
{
  int ret;

  /* smiler.071111: 保護Deletelog 以及 Editlog 的 記錄不被移除 */
  if((!strcmp(xo->dir,"brd/Deletelog/.DIR")) || (!strcmp(xo->dir,"brd/Editlog/.DIR")))
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


#ifdef HAVE_REFUSEMARK
static int
chkrestrict(hdr)
  HDR *hdr;
{
  return !(hdr->xmode & POST_RESTRICT) || 
    !strcmp(hdr->owner, cuser.userid) || (bbstate & STAT_BM);
}
#endif


static int
post_copy(xo)	   /* itoc.010924: 取代 gem_gather */
  XO *xo;
{
  int tag;

  tag = AskTag("看板文章拷貝");

  if (tag < 0)
    return XO_FOOT;

#ifdef HAVE_REFUSEMARK
  gem_buffer(xo->dir, tag ? NULL : (HDR *) xo_pool + (xo->pos - xo->top), chkrestrict);
#else
  gem_buffer(xo->dir, tag ? NULL : (HDR *) xo_pool + (xo->pos - xo->top), NULL);
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
/* 站長功能：edit / title				 */
/* ----------------------------------------------------- */


int
post_edit(xo)
  XO *xo;
{
  char fpath[64];
  HDR *hdr;
  FILE *fp;
  /* smiler 1031 */
  char Editlog_folder[64],Editlog_title[64],copied[64];
  HDR  Editlog_hdr;

  /* smiler.071111 保護 Editlog 以及 Deletelog 板的備份資料 */
 if((!strcmp(xo->dir,"brd/Deletelog/.DIR")) || (!strcmp(xo->dir,"brd/Editlog/.DIR")))
   return XO_NONE;

  hdr = (HDR *) xo_pool + (xo->pos - xo->top);

  /* smiler 1031 */
  hdr_fpath(copied,xo->dir,hdr);
  brd_fpath(Editlog_folder,"Editlog", FN_DIR);


  hdr_fpath(fpath, xo->dir, hdr);
 
  if (HAS_PERM(PERM_ALLBOARD))			/* 站長修改 */
  {
#ifdef HAVE_REFUSEMARK
    if ((hdr->xmode & POST_RESTRICT) && !RefusePal_belong(currboard, hdr))
      return XO_NONE;
#endif

    /* smiler 1031 */                                                                                
    hdr_stamp(Editlog_folder, HDR_COPY | 'A', &Editlog_hdr, copied);
    strcpy(Editlog_hdr.title , hdr->title);
    strcpy(Editlog_hdr.owner , hdr->owner);
    strcpy(Editlog_hdr.nick  , hdr->nick);
    Editlog_hdr.xmode = POST_OUTGO;
    rec_bot(Editlog_folder, &Editlog_hdr, sizeof(HDR));
    btime_update(brd_bno("Editlog"));

    vedit(fpath, 0);

    /* smiler 1031 */                                                                                
    hdr_stamp(Editlog_folder, HDR_COPY | 'A', &Editlog_hdr, copied);
    strcpy(Editlog_hdr.title , hdr->title);
    strcpy(Editlog_hdr.owner , cuser.userid);
    strcpy(Editlog_hdr.nick  , cuser.username);
    Editlog_hdr.xmode = POST_OUTGO;
    rec_bot(Editlog_folder, &Editlog_hdr, sizeof(HDR));
    btime_update(brd_bno("Editlog"));

  }
  else if ((cuser.userlevel && !strcmp(hdr->owner, cuser.userid)) || (bbstate & STAT_BOARD))	/* 板主/原作者修改 */
  {

	/* smiler 1031 */
    hdr_stamp(Editlog_folder, HDR_COPY | 'A', &Editlog_hdr, copied);
    strcpy(Editlog_hdr.title , hdr->title);
    strcpy(Editlog_hdr.owner , hdr->owner);
    strcpy(Editlog_hdr.nick  , hdr->nick);
    Editlog_hdr.xmode = POST_OUTGO;
    rec_bot(Editlog_folder, &Editlog_hdr, sizeof(HDR));
    btime_update(brd_bno("Editlog"));

    if (!vedit(fpath, 0))	/* 若非取消則加上修改資訊 */
    {
      if (fp = fopen(fpath, "a"))
      {
	ve_banner(fp, 1);
	fclose(fp);
      }
    }
    /* smiler 1031 */                                                                                
    hdr_stamp(Editlog_folder, HDR_COPY | 'A', &Editlog_hdr, copied);
    strcpy(Editlog_hdr.title , hdr->title);
    strcpy(Editlog_hdr.owner , cuser.userid);
    strcpy(Editlog_hdr.nick  , cuser.username);
    Editlog_hdr.xmode = POST_OUTGO;
    rec_bot(Editlog_folder, &Editlog_hdr, sizeof(HDR));
    btime_update(brd_bno("Editlog"));

  }
#if 0
  else		/* itoc.010301: 提供使用者修改(但不能儲存)其他人發表的文章 */
  {
#ifdef HAVE_REFUSEMARK
    if (hdr->xmode & POST_RESTRICT)
      return XO_NONE;
#endif
    vedit(fpath, -1);
  }
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

  while(fgets(buf, sizeof(buf), fpr))	/* 加入其他 */
    fputs(buf, fpw);

  fclose(fpr);
  fclose(fpw);
  f_rm(tmpfile);
}


static int
post_title(xo)
  XO *xo;
{
  HDR *fhdr, mhdr;
  int pos, cur;

  if (!cuser.userlevel)	/* itoc.000213: 避免 guest 在 sysop 板改標題 */
    return XO_NONE;

  pos = xo->pos;
  cur = pos - xo->top;
  fhdr = (HDR *) xo_pool + cur;
  memcpy(&mhdr, fhdr, sizeof(HDR));

  if (strcmp(cuser.userid, mhdr.owner) && !HAS_PERM(PERM_ALLBOARD))
    return XO_NONE;

  vget(b_lines, 0, "標題：", mhdr.title, TTLEN + 1, GCARRY);

  if (HAS_PERM(PERM_ALLBOARD))  /* itoc.000213: 原作者只能改標題 */
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


static void
addscore(hdd, ram)
  HDR *hdd, *ram;
{
  hdd->xmode |= POST_SCORE;
  hdd->stamp = ram->stamp;
  if (curraddscore < 50 )
  {
    if (hdd->score < 127)
      hdd->score++;
  }
  else if (curraddscore == 50)
  {
    if (hdd->score > -128)
      hdd->score--;
  }
}


int
post_score(xo)
  XO *xo;
{
  HDR *hdr;
  int pos, cur, ans, vtlen, maxlen;
  char *dir, *userid, *verb, fpath[64], reason[80];/*, vtbuf[12];*/
  FILE *fp;
#ifdef HAVE_ANONYMOUS
  char uid[IDLEN + 1];
#endif

  if ((currbattr & BRD_NOSCORE) || !cuser.userlevel || !(bbstate & STAT_POST) )	/* 評分視同發表文章 */
    return XO_NONE;

  pos = xo->pos;
  cur = pos - xo->top;
  hdr = (HDR *) xo_pool + cur;
  
  if (hdr->xmode & POST_NOSCORE)
    return XO_NONE;

#ifdef HAVE_REFUSEMARK
  if ((hdr->xmode & POST_RESTRICT) && !RefusePal_belong(currboard, hdr))
    return XO_NONE;
#endif

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
    ans='3';
    verb = "7m─";
    vtlen = 2;
  }

#ifdef HAVE_ANONYMOUS
  if (currbattr & BRD_ANONYMOUS)
    maxlen = 64 - IDLEN - vtlen;
  else
#endif
    maxlen = 64 - strlen(cuser.userid) - vtlen;

  if (!vget(b_lines, 0, "請輸入理由：", reason, maxlen, DOECHO))
    return XO_FOOT;

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
    maxlen = 64 - strlen(userid) - vtlen;
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

    fprintf(fp, "\033[1;3%s\033[m \033[1;36m%s \033[m：\033[0;33m%-*s\033[1;30m%02d/%02d/%02d\n", 
      verb, userid, maxlen, reason, 
      ptime->tm_year % 100, ptime->tm_mon + 1, ptime->tm_mday);
    fclose(fp);
  }

    curraddscore = ans;
    currchrono = hdr->chrono;
    change_stamp(xo->dir, hdr);
    rec_ref(dir, hdr, sizeof(HDR), xo->key == XZ_XPOST ? hdr->xid : pos, cmpchrono, addscore);
    if (hdr->xmode & POST_BOTTOM)  /* 若是評分置底文章，去找正本來連動分數 */
    {
      currchrono = hdr->parent_chrono;
      rec_ref(dir, hdr, sizeof(HDR), 0, cmpchrono, addscore);
    }
    else                           /* 若是評分一般文章，去找謄本來連動分數 */
    {
      /* currchrono = hdr->chrono; */ /* 前面有了 */
      rec_ref(dir, hdr, sizeof(HDR), 0, cmpparent, addscore);
    }
    post_history(xo, hdr);
    btime_update(currbno);

    return XO_LOAD;

  return XO_FOOT;
}
#endif	/* HAVE_SCORE */

static int
post_noforward(xo)
  XO *xo;
{
  HDR *hdr;
  int pos, cur;
  
  if (!cuser.userlevel) /* guest 不能對其他 guest 的文章加密 */
    return XO_NONE;
    
  pos = xo->pos;
  cur = pos - xo->top;
  hdr = (HDR *) xo_pool + cur;
  
  if (HAS_PERM(PERM_ALLBOARD) || !strcmp(hdr->owner, cuser.userid))
  {
    hdr->xmode ^= POST_NOFORWARD;
    currchrono = hdr->chrono;
    rec_put(xo->dir, hdr, sizeof(HDR), xo->key == XZ_XPOST ?
      hdr->xid : pos, cmpchrono);
      move(3 + cur, 7);
      outc(post_attr(hdr));
  }
  return XO_NONE;
}

static int
post_noscore(xo)
  XO *xo;
{
  HDR *hdr;
  int pos, cur;
  
  if (!cuser.userlevel) /* guest 不能對其他 guest 的文章加密 */
      return XO_NONE;
  
  pos = xo->pos;
  cur = pos - xo->top;
  hdr = (HDR *) xo_pool + cur;
  
  if (HAS_PERM(PERM_ALLBOARD) || !strcmp(hdr->owner, cuser.userid))
  {
    hdr->xmode ^= POST_NOSCORE;
    currchrono = hdr->chrono;
    rec_put(xo->dir, hdr, sizeof(HDR), xo->key == XZ_XPOST ?
      hdr->xid : pos, cmpchrono);
    move(3 + cur, 7);
    outc(post_attr(hdr));
  }
  return XO_NONE;
}

static int
post_help(xo)
  XO *xo;
{
  xo_help("post");
  /* return post_head(xo); */
  return XO_HEAD;		/* itoc.001029: 與 xpost_help 共用 */
}

static int
post_state(xo)
  XO *xo;
{
  HDR *ghdr;
  char fpath[64], *dir;
  struct stat st;
  

  
  if (!HAS_PERM(PERM_ALLBOARD))
    return XO_NONE;
                                                                                
  ghdr = (HDR *) xo_pool + (xo->pos - xo->top);
                                                                                
  dir = xo->dir;

  hdr_fpath(fpath, dir, ghdr);
                                                                                
  move(3, 0);
  clrtobot();

  prints("========  基本資料 ========\n");
  if (!stat(fpath, &st))
    prints("\n時間戳記 : %s\n檔案大小 : %d\n", Btime(&st.st_mtime), st.st_size);

  prints("DIR 位置 : %s\n",dir);
  prints("檔案名稱 : %s\n",ghdr->xname);
  prints("檔案位置 : %s\n",fpath);
  prints("作者     : %s\n",ghdr->owner);
  prints("暱稱     : %s\n",ghdr->nick);
  prints("發文日期 : %s\n",ghdr->date);
  prints("文章標題 : %s\n",ghdr->title);
  prints("========  文章屬性 ========\n");

  
  prints("文章標記 : ");
  if(ghdr->xmode & POST_MARKED)
	  prints("有");
  else
	  prints("無");

  prints("    可否轉寄 : ");
  if(ghdr->xmode & POST_NOFORWARD)
	  prints("否\n");
  else
	  prints("可\n");

  prints("可否評分 : ");
  if(ghdr->xmode & POST_NOSCORE)
	  prints("否");
  else
	  prints("可");

  prints("    置底文 ? : ");
  if(ghdr->xmode & POST_BOTTOM)
	  prints("是\n");
  else
	  prints("否\n");


  prints("標記待砍 : ");
  if(ghdr->xmode & POST_DELETE)
	  prints("是");
  else
	  prints("否");


  prints("    轉信進來 : ");
  if(ghdr->xmode & POST_INCOME)
	  prints("是\n");
  else
	  prints("否\n");


  prints("初級限制 : ");
  if(ghdr->xmode & POST_FRIEND)
	  prints("是");
  else
	  prints("否");


  prints("    可轉站外 : ");
  if(ghdr->xmode & POST_OUTGO)
	  prints("是\n");
  else
	  prints("否\n");

  prints("中級限制 : ");
  if(ghdr->xmode & POST_RESTRICT)
	  prints("是");
  else
	  prints("否");

  prints("    最高限制 : ");
  if(ghdr->xmode & POST_RESERVED)
	  prints("是\n");
  else
	  prints("否\n");

  prints("已被評分 : ");
  if(ghdr->xmode & POST_SCORE)
	  prints("是\n");
  else
	  prints("否\n");
                                                                                
  vmsg(NULL);
                                                                                
  return post_body(xo);
}

/* smiler.080201: 畫面顯示 檔名<-> 篇數 */
static int
post_filename(xo)
  XO *xo;
{
  cuser.ufo ^= UFO_FILENAME;
  cutmp->ufo = cuser.ufo;
  return XO_INIT;
}

static int
post_sub()
{
  char name[IDLEN + 1];
  char url[30], fpath[80];
  int bm;
  FILE *fp;
  
  bm = bbstate & STAT_BOARD;
  
  if (bm)
  {
    if (!vget(b_lines - 1, 0, "*bookmark* : ", name, IDLEN + 1, DOECHO))
      return XO_INIT;
    if (!vget(b_lines - 1, 0, "*url* : ", url, 70, DOECHO))
      return XO_INIT;

    brd_fpath(fpath, currboard, "rss");

    if(fp = fopen(fpath, "r"))
	{
		vmsg("已有RSS");
		return XO_INIT;
	}
    else
      fp = fopen(fpath, "w");
    fprintf(fp, "%s\n%s", name, url);
    fclose(fp);   
  }  
  return XO_INIT;
}

static int
post_sub_delete()
{
  char fpath[80];
  int bm; 
  bm = bbstate & STAT_BOARD;
  
  if (bm)
  {
    brd_fpath(fpath, currboard, "rss");
    unlink(fpath);
  }  
  return XO_INIT;
}

static int
post_rss(xo)
  XO *xo;
{
  int bm;
  int ans;
  bm = bbstate & STAT_BOARD;  
  if (bm)
  {
     switch (ans = vans("◎RSS 1)新增 2)刪除 [Q] "))
	 {
       case '1':
		   return post_sub();
		   break;
	   case '2':
		   return post_sub_delete();
		   break;
	   default:
		   return XO_INIT;
	}
  }
  return XO_INIT;
}

KeyFunc post_cb[] =
{
#ifdef HAVE_LIGHTBAR
  XO_ITEM, post_item_bar,         /* verit.20030129 : 請務必將此放在第一項 */
#endif
  XO_INIT, post_init,
  XO_LOAD, post_load,
  XO_HEAD, post_head,
  XO_BODY, post_body,

  'r', post_browse,
  's', post_switch,
  KEY_TAB, post_gem,
  'z', post_gem,
  'o', post_noforward,
  'f', post_filename,
  'y', post_reply,
  'd', post_delete,
  'v', post_visit,
  'x', post_cross,		/* 在 post/mbox 中都是小寫 x 轉看板，大寫 X 轉使用者 */
  'X', post_forward,
  't', post_tag,
  'E', post_edit,
  'T', post_title,
  'm', post_mark,
  '_', post_bottom,
  'D', post_rangedel,
  'S', post_rss,
#ifdef HAVE_SCORE
  '%', post_score,
#endif

  'w', post_write,

  'b', post_memo,
  'c', post_copy,
  'g', gem_gather,

  Ctrl('P'), post_add,
  Ctrl('D'), post_prune,
  Ctrl('Q'), xo_uquery,
  Ctrl('O'), xo_usetup,
  Ctrl('e'), post_noscore,
  Ctrl('f'), XoBM_add_pal,
  'O', XoBM_Refuse_pal,
  Ctrl('g'), post_viewpal,
  Ctrl('b'), post_showbm,
  Ctrl('S'), post_state,
#ifdef HAVE_REFUSEMARK
  'L', post_refuse,
  'l', post_friend,
#endif

#ifdef HAVE_LABELMARK
  'n', post_label,
  Ctrl('N'), post_delabel,
#endif

  'B' | XO_DL, (void *) "bin/manage.so:post_manage",
  'R' | XO_DL, (void *) "bin/vote.so:vote_result",
  'V' | XO_DL, (void *) "bin/vote.so:XoVote",

#ifdef HAVE_TERMINATOR
  Ctrl('X') | XO_DL, (void *) "bin/manage.so:post_terminator",
#endif

  '/', XOXpost_search_all,  /* smiler.070201: 搜尋功能整合 */

  //'~' | XO_DL, (int *)  "bin/dictd.so:main_dictd",
  //'~', XoXselect,		/* itoc.001220: 搜尋作者/標題 */
  //'S', XoXsearch,		/* itoc.001220: 搜尋相同標題文章 */
  //'a', XoXauthor,		/* itoc.001220: 搜尋作者 */
  //'/', XoXtitle,		/* itoc.001220: 搜尋標題 */
  //'f', XoXfull,			/* itoc.030608: 全文搜尋 */
  //'G', XoXmark,			/* itoc.010325: 搜尋 mark 文章 */
  //'K', XoXlocal,		/* itoc.010822: 搜尋本地文章 */

#ifdef HAVE_XYNEWS
  'u', XoNews,			/* itoc.010822: 新聞閱讀模式 */
#endif

  'h', post_help
};


KeyFunc xpost_cb[] =
{
#ifdef HAVE_LIGHTBAR
  XO_ITEM, post_item_bar,         /* verit.20030129 : 請務必將此放在第一項 */
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
  'o', post_noforward,
  'c', post_copy,
  'g', gem_gather,
  'm', post_mark,
  'd', post_delete,		/* Thor.980911: 方便板主 */
  'E', post_edit,		/* itoc.010716: 提供 XPOST 中可以編輯標題、文章，加密 */
  'T', post_title,
#ifdef HAVE_SCORE
  '%', post_score,
#endif
  'w', post_write,
#ifdef HAVE_REFUSEMARK
  'L', post_refuse,
#endif
#ifdef HAVE_LABELMARK
  'n', post_label,
#endif

    '/', XOXpost_search_all,  /* smiler.070201: 搜尋功能整合 */
//  '~', XoXselect,
//  'S', XoXsearch,
//  'a', XoXauthor,
//  '/', XoXtitle,
//  'f', XoXfull,
//  'G', XoXmark,
//  'K', XoXlocal,

  Ctrl('P'), post_add,
  Ctrl('D'), post_prune,
  Ctrl('Q'), xo_uquery,
  Ctrl('O'), xo_usetup,
  Ctrl('e'), post_noscore,

  'h', post_help		/* itoc.030511: 共用即可 */
};


#ifdef HAVE_XYNEWS
KeyFunc news_cb[] =
{
#ifdef HAVE_LIGHTBAR
  XO_ITEM, post_item_bar,         /* verit.20030129 : 請務必將此放在第一項 */
#endif
  XO_INIT, news_init,
  XO_LOAD, news_load,
  XO_HEAD, news_head,
  XO_BODY, post_body,

  'r', XoXsearch,

  'h', post_help		/* itoc.030511: 共用即可 */
};
#endif	/* HAVE_XYNEWS */
