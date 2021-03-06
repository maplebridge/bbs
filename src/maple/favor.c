/*-------------------------------------------------------*/
/* favorite.c	( NTHU CS MapleBBS Ver 3.10 )		 */
/*-------------------------------------------------------*/
/* target : 我的最愛					 */
/* create : 00/06/16					 */
/* update :   /  /  					 */
/* author : weichung.bbs@bbs.ntit.edu.tw		 */
/*-------------------------------------------------------*/


#include "bbs.h"


#ifdef MY_FAVORITE

static int mf_add();
static int mf_paste();
static int mf_load();
static void XoMF();


extern XZ xz[];
extern char xo_pool[];
extern char brd_bits[];		/* itoc.010821: 判斷是否有閱讀看板的權限 */
extern BCACHE *bshm;
extern int class_flag;

#ifndef ENHANCED_VISIT
extern time4_t brd_visit[];
#endif

#ifdef AUTO_JUMPBRD
static int mf_jumpnext = 0;     /* itoc.020615: 是否跳去下一個未讀板 1:要 0:不要 */
#endif


static MF mftmp;		/* for copy & paste */

static int mf_depth;


void
mf_fpath(fpath, userid, fname)
  char *fpath;
  char *userid;
  char *fname;
{
  char buf[64];

  sprintf(buf, "MF/%s", fname);
  usr_fpath(fpath, userid, buf);
}


static int
mf_urifolder(fpath)
  char * fpath;
{
  BRD *brd;
  int fsize, bno, unread;
  MF *data, *head, *tail;

  unread = 0;
  if (data = (MF *) f_img(fpath, &fsize))
  {
    head = data;
    tail = data + (fsize / sizeof(MF));
    do
    {
      if (head->mftype & MF_BOARD)
      {
	if ((bno = brd_bno(head->xname)) >= 0)
	{
	  if (!(brd_bits[bno] & BRD_R_BIT))	/* 只計算進得去的看板 */
	    continue;
	  brd = bshm->bcache + bno;
	  btime_refresh(brd);
#ifdef ENHANCED_VISIT
	  /* itoc.010407: 改用最後一篇已讀/未讀來判斷 */
	  brh_get(brd->bstamp, bno);
	  unread += brh_unread(brd->blast);
#else
	  unread += brd->blast > brd_visit[bno] ? 1 : 0;
#endif
	}
      }
    } while (++head < tail);
    free(data);
  }

  return unread;
}


static void
mf_item(num, mf)
  int num;
  MF *mf;
{
  char folder[64];
  int mftype, brdpost, bno, label;

  mftype = mf->mftype;
  brdpost = cuser.ufo & UFO_BRDPOST;
  label = mftype & MF_LABEL;

  if (mftype & MF_FOLDER)
  {
    mf_fpath(folder, cuser.userid, mf->xname);
    if (brdpost)
    {
      num = rec_num(folder, sizeof(MF));
    }
    if ((USR_SHOW & USR_SHOW_MF_FOLDER_UNREAD) && (bno = mf_urifolder(folder)))
      sprintf(folder, "\033[1;33m%2d\033[m", BMIN(99, bno));
    else
      sprintf(folder, "◆");

    prints("%6d%c  %-2s %s\n", num, mftype & MF_MARK ? ')' : label ? 'T' : ' ',
      folder, mf->title);
  }
  else if (mftype & MF_BOARD)
  {
    if ((bno = brd_bno(mf->xname)) >= 0)
    {
      if (!(brd_bits[bno] & BRD_L_BIT))
	prints("%6d%c  \033[36m%s\033[m\n",
	  brdpost ? 0 : num, label ? 'T' : ' ', "<本看板為隱板>");
      else
	class_item(num, bno, brdpost, 1, label);
    }
    else
      /* itoc.010821: 不見的看板，讓 user 自己清掉，如此 user 才知道哪些看板被砍了 */
      prints("       %c \033[36m<%s 已改名或被刪除，請將本捷徑刪除>\033[m\n", label ? 'T' : ' ', mf->xname);
  }
  else if (mftype & MF_GEM)
  {
    prints("%6d%c  %s %s\n",
      brdpost ? 0 : num, 
      mftype & MF_MARK ? ')' : label ? 'T' : ' ', "■", mf->title);
  }
  else  if (mftype & MF_LINE)		/* qazq.040721: 分隔線 */
  {
    prints("%6d%c  %s\n", 
      brdpost ? 0 : num, label ? 'T' : ' ', mf->title);
  }
  else /* if (mftype & MF_CLASS) */	/* LHD.051007: 分類群組 */
  {
    char cname[BNLEN + 2];

    sprintf(cname, "%s/", mf->xname);
    prints("%6d%c  %-13.13s\033[1;3%dm%-5.5s\033[m%s\n",
      num, label ? 'T' : ' ', cname, mf->class[3] & 7, mf->class, mf->title);
  }
}


#ifdef HAVE_LIGHTBAR
static int
mf_item_bar(xo, mode)
  XO *xo;
  int mode;
{
  int num;
  MF *mf;
  int mftype, brdpost, invalid, label;

  num = xo->pos + 1;
  mf = (MF *) xo_pool + xo->pos - xo->top;
  mftype = mf->mftype;
  brdpost = cuser.ufo & UFO_BRDPOST;
  label = mftype & MF_LABEL;

  if (mftype & MF_FOLDER)
  {
    int unread;
    char fpath[64];
    mf_fpath(fpath, cuser.userid, mf->xname);
    if (brdpost)
    {
      struct stat st;

      stat(fpath, &st);
      num = st.st_size / sizeof(MF);
    }

    if ((USR_SHOW & USR_SHOW_MF_FOLDER_UNREAD) && (unread = mf_urifolder(fpath)))
      sprintf(fpath, "%s%2d%s", mode ? "" : "\033[1;33m", BMIN(99, unread), mode ? "" : "\033[m");
    else
      sprintf(fpath, "◆");

    prints("%s%6d%c  %-2s %-*.*s\033[m", mode ? UCBAR[UCBAR_BRD] : "",
      num, mftype & MF_MARK ? ')' : label ? 'T' : ' ',
      fpath, d_cols + 66, d_cols + 65, mf->title);
  }
  else if (mftype & MF_BOARD)
  {
    BRD *bhead, *btail;
    int chn;

    chn = 0;
    invalid = 1;
    bhead = bshm->bcache;
    btail = bhead + bshm->number;

    do
    {
      if (!strcmp(mf->xname, bhead->brdname))
      {
	if (!(brd_bits[chn] & BRD_L_BIT))
	  prints("%s%6d%c  \033[36m%-*.*s\033[m",
	    mode ? UCBAR[UCBAR_BRD] : "",
	    brdpost ? 0 : num, label ? 'T' : ' ',
	    d_cols + 69, d_cols + 68, "<本看板為隱板>");
	else
	{
	  if (mode)
	    class_item_bar(bhead, num, chn, brdpost, 1, label);
	  else
	    class_item(num, chn, brdpost, 1, label);
	}
	invalid = 0;
	break;
      }
      chn++;
    } while (++bhead < btail);

    if (invalid)        /* itoc.010821: 被砍的看板要另外印 */
    {
      prints("%s       %c \033[36m%-13s%-*.*s\033[m", mode ?
	UCBAR[UCBAR_BRD] : "", label ? 'T' : ' ', mf->xname,
	d_cols + 56, d_cols + 55, "<已改名或被刪除，請將本捷徑刪除>");
    }  /* 長度好難調 乾脆自己改了= =*/
  }
  else if (mftype & MF_GEM)
  {
    prints("%s%6d%c  %s %-*.*s\033[m",
      mode ? UCBAR[UCBAR_BRD] : "",
      brdpost ? 0 : num, mftype & MF_MARK ? ')' : label ? 'T' : ' ',
      "■", d_cols + 66, d_cols + 65, mf->title);
  }
  else  if (mftype & MF_LINE)		/* qazq.040721: 分隔線 */
  {
    prints("%s%6d%c  %-*.*s\033[m",
      mode ? UCBAR[UCBAR_BRD] : "",
      brdpost ? 0 : num, label ? 'T' : ' ',
      d_cols + 69, d_cols + 68, mf->title);
  }
  else /* if (mftype & MF_CLASS) */	/* LHD.051007: 分類群組 */
  {
    char cname[BNLEN + 2];

    sprintf(cname, "%s/", mf->xname);
    prints("%s%6d%c  %-13.13s\033[1;3%dm%-5.5s\033[m%s%-*.*s\033[m",
      mode ? UCBAR[UCBAR_BRD] : "",num, label ? 'T' : ' ', cname, mf->class[3] & 7,
      mf->class, mode ? UCBAR[UCBAR_BRD] : "", d_cols + 51, d_cols + 50, mf->title);
  }

  move(xo->pos - xo->top + 3, 0);
  return XO_NONE;
}
#endif


static int
mf_body(xo)
  XO *xo;
{
  MF *mf;
  int max, num, tail;
#ifdef AUTO_JUMPBRD
  int nextpos;
  static int originpos = -1;
#endif

  max = xo->max;
  if (max <= 0)
  {
    max = vans("我的最愛 (A)新增 (P)貼上 (Q)離開 [Q] ");
    switch (max)
    {
    case 'a':

      max = mf_add(xo);
      if (xo->max > 0)
	return max;
      break;

    case 'p':

      mf_paste(xo);
      return mf_load(xo);
    }

    return XO_QUIT;
  }

  num = xo->top;
  tail = num + XO_TALL;
  if (max > tail)
    max = tail;

#ifdef AUTO_JUMPBRD
  nextpos = 0;

  /* itoc.020615: 搜尋下一個未讀看板 */
  if (mf_jumpnext)
  {
    BRD *bcache;

    tail = xo->pos;	/* 借用 tail */
    if (originpos < 0)	/* 在去下頁搜尋未讀看板前，記錄一開始游標所在 */
      originpos = tail;
    mf = (MF *) xo_pool + tail - num;
    bcache = bshm->bcache;

    while (tail < max)	/* 只能找本頁中的未讀看板，因為下頁還沒載入 */
    {
      if (mf->mftype & MF_BOARD)
      {
	int chn;
	BRD *brd;

	chn = brd_bno(mf->xname);
	brd = bcache + chn;

#ifdef ENHANCED_VISIT
	/* itoc.010407: 改用最後一篇已讀/未讀來判斷 */
	brh_get(brd->bstamp, chn);
	if (brh_unread(brd->blast))
#else
	if (brd->blast > brd_visit[chn])
#endif
	{
	  nextpos = tail;
	  mf_jumpnext = 0;
	  originpos = -1;
	  break;
	}
      }
      tail++;
      mf++;
    }

    if (mf_jumpnext)	/* 如果在本頁沒有找到未讀看板 */
    {
      if (max < rec_num(xo->dir, sizeof(MF)))	/* 再去下頁找 */
	return num + XO_TALL + XO_MOVE;
 
      /* 已經是最後一頁了還是找不到未讀看板 */
      mf_jumpnext = 0;
      tail = originpos;
      originpos = -1;
      if (tail < num)		/* 回到原來那頁 */
	return tail + XO_MOVE;
    }
  }
#endif

  mf = (MF *) xo_pool;

  move(3, 0);
  do
  {
    mf_item(++num, mf++);
  } while (num < max);
  clrtobot();

#ifdef AUTO_JUMPBRD
  /* itoc.020615: 下一個未讀板在本頁，要把游標移過去 */
  outf(FEETER_MF);
  return nextpos ? nextpos + XO_MOVE : XO_NONE;
#else
  /* return XO_NONE; */
  return XO_FOOT;	/* itoc.010403: 把 b_lines 填上 feeter */
#endif
}


static int
mf_head(xo)
  XO *xo;
{
  vs_head("我的最愛", str_site);
  prints(NECKER_MF, 
    class_flag & UFO_BRDPOST ? "總數" : "編號", 
    d_cols >> 1, "", d_cols - (d_cols >> 1), "");
  return mf_body(xo);
}


static int
mf_init(xo)
  XO *xo;
{
  xo_load(xo, sizeof(MF));
  return mf_head(xo);
}


static int
mf_load(xo)
  XO *xo;
{
  xo_load(xo, sizeof(MF));
  return mf_body(xo);
}


static int
mf_stamp(mf)
  MF *mf;
{
  char fpath[64];
  int fd;

  do
  {
    mf->xname[0] = 'F';
    archiv32(mf->chrono, mf->xname + 1);

    mf_fpath(fpath, cuser.userid, mf->xname);

    if ((fd = open(fpath, O_WRONLY | O_CREAT | O_EXCL, 0600)) >= 0)
      close(fd);
  } while (fd < 0 && mf->chrono++);

  return fd;
}


static int
mf_add(xo)
  XO *xo;
{
  MF mf;
  int ans;

  ans = vans("新增 (B)看板捷徑 (F)卷宗 (G)精華區捷徑 (L)分隔線 [Q] ");

  if (ans != 'b' && ans != 'f' && ans != 'g' && ans != 'l')
    return XO_FOOT;

  time4(&mf.chrono);

  if (ans == 'b' || ans == 'g')
  {
    BRD *brd;
    char bname[BNLEN + 1];

    if (!(brd = ask_board(bname, BRD_R_BIT, NULL)))
      return mf_head(xo);

    mf.mftype = (ans == 'b') ? MF_BOARD : MF_GEM;
    strcpy(mf.xname, brd->brdname);
    if (ans == 'g')
      sprintf(mf.title, "%s 板 精華區捷徑", mf.xname);
  }
  else /* if (ans == 'f' || ans == 'l') */
  {
    if (!vget(b_lines, 0, "標題：", mf.title, BTLEN + 1, DOECHO))
      return XO_FOOT;

    mf.mftype = (ans == 'f') ? MF_FOLDER : MF_LINE;
    if (ans == 'f')
    {
      if (mf_stamp(&mf) < 0)
	return XO_FOOT;
    }
  }

  ans = vans("存放位置 A)新增 I)插入 N)下一個 Q)離開 [A] ");
  switch (ans)
  {
  case 'q':  
    break;

  case 'i':
  case 'n':

    rec_ins(xo->dir, &mf, sizeof(MF), xo->pos + (ans == 'n'), 1);
    break;

  default:

    rec_add(xo->dir, &mf, sizeof(MF));
    break;
  }

  return mf_init(xo);
}


static void
mf_do_delete(folder)
  char *folder;
{
  MF mf;
  char fpath[64];
  FILE *fp;

  if (!(fp = fopen(folder, "r")))
    return;

  while (fread(&mf, sizeof(MF), 1, fp) == 1)
  {
    if (mf.mftype & MF_FOLDER)
    {
      mf_fpath(fpath, cuser.userid, mf.xname);
      mf_do_delete(fpath);
    }
  }

  fclose(fp);
  unlink(folder);
}


static int
mf_delete(xo)
  XO *xo;
{
  MF *mf;
  int mftype;
  char fpath[64];

  mf = (MF *) xo_pool + (xo->pos - xo->top);
  mftype = mf->mftype;

  if (mftype & MF_MARK)
    return XO_NONE;

  if (vans(msg_del_ny) == 'y')
  {
    if (mftype & MF_FOLDER)
    {
      mf_fpath(fpath, cuser.userid, mf->xname);
      mf_do_delete(fpath);
    }
    if (!rec_del(xo->dir, sizeof(MF), xo->pos, NULL))
      return mf_load(xo);
  }

  return XO_FOOT;
}


static void
delmf(xo, mf)
  XO *xo;
  MF *mf;
{
  if (mf->mftype & MF_FOLDER)
  {
    char fpath[64];

    mf_fpath(fpath, cuser.userid, mf->xname);
    mf_do_delete(fpath);
  }
}


static int
mf_rangedel(xo)		/* amaki.030910: 提供我的最愛區段刪除 */
  XO *xo;
{
  return xo_rangedel(xo, sizeof(MF), NULL, delmf);
}


static int
mf_title(xo)
  XO *xo;
{
  MF *mf, xmf;

  mf = (MF *) xo_pool + (xo->pos - xo->top);
  xmf = *mf;

  if (!(mf->mftype & (MF_FOLDER | MF_LINE)))
    return XO_NONE;

  vget(b_lines, 0, "標題：", xmf.title, BTLEN + 1, GCARRY);

  if (memcmp(mf, &xmf, sizeof(MF)) && vans(msg_sure_ny) == 'y')
  {
    int num;

    *mf = xmf;
    num = xo->pos;
    rec_put(xo->dir, mf, sizeof(MF), xo->pos, NULL);
    num++;
    move(num - xo->top + 2, 0);
    mf_item(num, mf);
  }

  return XO_FOOT;
}


static int
mf_move(xo)
  XO *xo;
{
  MF *mf;
  char *dir, buf[40];
  int pos, newOrder;

  pos = xo->pos;
  mf = (MF *) xo_pool + (pos - xo->top);

  sprintf(buf, "請輸入第 %d 選項的新位置：", pos + 1);
  if (!vget(b_lines, 0, buf, buf, 5, DOECHO))
    return XO_FOOT;

  newOrder = atoi(buf) - 1;
  if (newOrder < 0)
    newOrder = 0;
  else if (newOrder >= xo->max)
    newOrder = xo->max - 1;

  if (newOrder != pos)
  {
    dir = xo->dir;
    if (!rec_del(dir, sizeof(MF), pos, NULL))
    {
      rec_ins(dir, mf, sizeof(MF), newOrder, 1);
      xo->pos = newOrder;
      return mf_load(xo);
    }
  }

  return XO_FOOT;
}


static int
mf_mark(xo)
  XO *xo;
{
  MF *mf;

  mf = (MF *) xo_pool + (xo->pos - xo->top);

  if (mf->mftype & MF_FOLDER)
  {
    int num;

    mf->mftype ^= MF_MARK;
    num = xo->pos;
    rec_put(xo->dir, mf, sizeof(MF), num, NULL);
    num++;
    move(num - xo->top + 2, 0);
    mf_item(num, mf);
  }

  return XO_NONE;
}


static int
mf_label(xo)
  XO *xo;
{
  MF *mf;
  int pos, cur, type;

  pos = xo->pos;
  cur = pos - xo->top;
  mf = (MF *) xo_pool + cur;
  type = mf->mftype;

  if (type & MF_MARK)	/* mark 的卷宗不不能移動 */
    return XO_NONE;

  mf->mftype = type ^ MF_LABEL;
  rec_put(xo->dir, mf, sizeof(MF), pos, NULL);

  move(3 + cur, 0);
  mf_item(pos + 1, mf);

  return (pos + 1 == xo->max ? 0 : pos + 1) + XO_MOVE;   /* 跳至下一項 */
}


static int
mf_has_label(src, depth)
  char *src;
  int depth;
{
  static int has_label;

  int fd;
  char fpath[64];
  MF fmf;

  if (!depth)
    has_label = 0;
  else
  {
    if (has_label)		/* 在某一個遞迴中找豆鋮?MF_LABEL 就停止搜尋 */
      return 1;
  }

  mf_fpath(fpath, cuser.userid, src);
  if ((fd = open(fpath, O_RDONLY)) >= 0)
  {
    while (read(fd, &fmf, sizeof(MF)) == sizeof(MF))
    {
      if (fmf.mftype & MF_LABEL)
      {
	has_label = 1;
	break;
      }
      if (fmf.mftype & MF_FOLDER)
	mf_has_label(fmf.xname, 1);
    }
    close(fd);
  }

  return has_label;
}


static int
mf_do_clear_label(src, curDir)
  char *src, *curDir;
{
  int fsize;
  char fpath[64];
  MF mf, *data, *head, *tail;

  if (data = (MF *) f_img(src, &fsize))
  {
    head = data;
    tail = data + (fsize / sizeof(MF));
    do
    {
      if (head->mftype & MF_FOLDER)
      {
        mf_fpath(fpath, cuser.userid, head->xname);
        mf_do_clear_label(fpath, curDir);
      }

      if ((!curDir || strcmp(curDir, src)) && (head->mftype & MF_LABEL))
      {
        memcpy(&mf, head, sizeof(MF));
        mf.mftype &= ~MF_LABEL;
	rec_put(src, &mf, sizeof(MF), head - data, NULL);
      }
    } while (++head < tail);

    free(data);
  }
}


static int
mf_clear_label(xo)
  XO *xo;
{
  char folder[64];

  if (!mf_has_label(FN_MF, 0))
    return XO_NONE;

  if (vans("要清除所有的標記嗎？[y/N]") != 'y')
    return XO_FOOT;

  mf_fpath(folder, cuser.userid, FN_MF);
  mf_do_clear_label(folder, NULL);

  return mf_load(xo);
}


static int
mf_browse(xo)
  XO *xo;
{
  int type, bno;
  char *xname, fpath[64];
  BRD *brd;
  MF *mf;

  mf = (MF *) xo_pool + (xo->pos - xo->top);
  type = mf->mftype;
  xname = mf->xname;

  if (type & MF_BOARD)		/* 看板捷徑 */
  {
    /* itoc.010726: 若是看板已經被砍或權限沒有了，則要移除捷徑 */
    if ((bno = brd_bno(xname)) < 0)
    {
      rec_del(xo->dir, sizeof(MF), xo->pos, NULL);
      vmsg("本看板已被刪除，系統將自動移除捷徑");
      return mf_load(xo);
    }
#if 0	/* 已於下面的 XoPost() 一併處理 */
    else if (!(brd_bits[bno] & BRD_R_BIT))
    {
      vmsg("您沒有權限閱\讀本看板");
      return XO_FOOT;
    }
#endif

    brd = bshm->bcache + bno;
    if (XoPost(bno))	/* 無法閱讀該板 */
      return XO_FOOT;
    xover(XZ_POST);
#ifndef ENHANCED_VISIT
    time4(&brd_visit[bno]);
#endif

#ifdef AUTO_JUMPBRD
    if (cuser.ufo & UFO_JUMPBRD)
      mf_jumpnext = 1;	/* itoc.010910: 只有在離開看板回到看板列表時才需要跳去下一個未讀看板 */
#endif

    if ((brd_bits[bno] & BRD_Z_BIT) &&
      (vans("本看板被 zap 掉了，將不會儲存閱\讀紀錄，是否要取消 zap？[y/N]") == 'y'))
    {
      brd_bits[bno] ^= BRD_Z_BIT;
    }

    return mf_init(xo);
  }
  else if (type & MF_GEM)	/* 精華區捷徑 */
  {
    /* itoc.010726: 若是看板已經被砍或權限沒有了，則要移除捷徑 */
    if ((type = gem_link(xname)) < 0)
    {
#if 0
      rec_del(xo->dir, sizeof(MF), xo->pos, NULL);
      vmsg("本看板已被刪除或您沒有權限閱\讀本看板，系統將自動移除捷徑");
#endif
      vmsg("本看板已被刪除或您沒有權限閱\讀本看板");
      return XO_FOOT;
    }

    gem_fpath(fpath, xname, fn_dir);
    XoGem(fpath, "精華區", type);
    return mf_init(xo);
  }
  else if (type & MF_FOLDER)	/* 卷宗 */
  {
    mf_depth++;
    mf_fpath(fpath, cuser.userid, xname);
    XoMF(fpath);
    mf_depth--;
    return mf_load(xo);
  }
  else if (type & MF_CLASS)	/* 分類群組 */
  {
    if (!MFclass_browse(xname))
    {
      rec_del(xo->dir, sizeof(MF), xo->pos, NULL);
      vmsg("本分類群組已被刪除，系統將自動移除捷徑");
      return mf_load(xo);
    }
    return mf_init(xo);
  }

  return XO_NONE;
}


static int
mf_copy(xo)
  XO *xo;
{
  MF *mf;

  mf = (MF *) xo_pool + (xo->pos - xo->top);

  memcpy(&mftmp, mf, sizeof(MF));
  zmsg("拷貝完成，但是卷宗內的資料不會被拷貝");

  return XO_FOOT;
}


static void
mf_do_paste(dstDir, mf, pos)
  char *dstDir;		/* destination folder */
  MF *mf;		/* source hdr */
  int pos;		/* -1: 附加在最後  >=0: 貼上的位置 */
{
  int xmode, fsize;
  char folder[64], fpath[64];
  MF fmf, *data, *head, *tail;

  xmode = mf->mftype;
  memcpy(&fmf, mf, sizeof(MF));

  if (xmode & MF_FOLDER)	/* 卷宗/分類 */
  {
    /* 在複製/貼上後一律變成卷宗，因為分類是站長專用特殊用途的 */
    time4(&fmf.chrono);			/* 造一個新的 chrono */
    if ((fsize = mf_stamp(&fmf)) < 0)
      return;
    close(fsize);
  }
  fmf.mftype &= ~MF_LABEL;	/* 標記不複製 */

  if (pos < 0)
    rec_add(dstDir, &fmf, sizeof(MF));
  else
    rec_ins(dstDir, &fmf, sizeof(MF), pos, 1);

  if (xmode & MF_FOLDER)	/* 卷宗/分類 */
  {
    /* 建立完自己這個卷宗以後，再 recursive 地一層一層目錄進去一篇一篇另存新檔 */
    mf_fpath(folder, cuser.userid, mf->xname);
    mf_fpath(fpath, cuser.userid, fmf.xname);
    if (data = (MF *) f_img(folder, &fsize))
    {
      head = data;
      tail = data + (fsize / sizeof(MF));
      do
      {
	mf_do_paste(fpath, head, -1);
      } while (++head < tail);

      free(data);
    }
  }
}


static int
mf_invalid_loop(dst, src, depth)
  char *dst, *src;
  int depth;
{
  static int valid;

  int fd;
  char fpath[64];
  MF fmf;

  if (!depth)
  {
    if (!strcmp(dst, src))	/* 把自己拷到自己裡面 */
      return 1;
    valid = 0;
  }
  else
  {
    if (valid)		/* 在某一個遞迴中找到非法證據就停止搜證工作 */
      return 1;
  }

  mf_fpath(fpath, cuser.userid, src);
  if ((fd = open(fpath, O_RDONLY)) >= 0)
  {
    while (read(fd, &fmf, sizeof(MF)) == sizeof(MF))
    {  
      if (fmf.mftype & MF_FOLDER)	/* MF_FOLDER 以外不會造成無窮迴圈 */
      {
	if (!strcmp(dst, fmf.xname))
	{
	  valid = 1;
	  return 1;
	}

	/* recursive 地一層一層卷宗進去檢查是否會造成無窮迴圈 */
	mf_invalid_loop(dst, fmf.xname, 1);
      }
    }
    close(fd);
  }

  return valid;
}


static int
mf_paste(xo)
  XO *xo;
{
  MF mf;
  int ans;

  if (!mftmp.chrono)
  {
    zmsg("請先執行 copy 命令後再 paste");
    return XO_FOOT;
  }

  memcpy(&mf, &mftmp, sizeof(MF));
  time4(&mf.chrono);			/* 造一個新的 chrono */

  /* itoc.010726.註解: 若是 MF_FOLDER，則換個檔名再貼上，一個卷宗一個檔案 */
  /* itoc.010726.註解: 卷宗複製貼上，裡面的東西並沒有貼上，懶得寫 recursive 的程式 :p */
  if (mf.mftype & MF_FOLDER)
  {
    if (mf_invalid_loop(xo->dir + 10 + strlen(cuser.userid), mftmp.xname, 0))	/* strlen("usr/u/userid/MF/") */
    {
      vmsg("造成迴圈的卷宗將無法收錄");
      return XO_FOOT;
    }
  }

  ans = vans("存放位置 A)新增 I)插入 N)下一個 Q)離開 [A] ");
  switch (ans)
  {
  case 'q':
    return XO_FOOT;

  case 'i':
  case 'n':

    if (mf.mftype & MF_FOLDER)
      mf_do_paste(xo->dir, &mftmp, xo->pos + (ans == 'n'));
    else
      rec_ins(xo->dir, &mf, sizeof(MF), xo->pos + (ans == 'n'), 1);
    break;

  default:

    if (mf.mftype & MF_FOLDER)
      mf_do_paste(xo->dir, &mftmp, -1);
    else
      rec_add(xo->dir, &mf, sizeof(MF));
    break;
  }

  return mf_load(xo);
}


static void
mf_do_append(dst, src)
  char *dst, *src;
{
  int fsize, offset;
  char fpath[64];
  MF mf, *data, *head, *tail;

  if (data = (MF *) f_img(src, &fsize))
  {
    offset = 0;
    head = data;
    tail = data + (fsize / sizeof(MF));
    do
    {
      if (head->mftype & MF_FOLDER)
      {
	mf_fpath(fpath, cuser.userid, head->xname);
	mf_do_append(dst, fpath);

	if (mf_invalid_loop(dst + 10 + strlen(cuser.userid), head->xname, 0))	/* strlen("usr/u/userid/MF/") */
	  continue;
      }

      if (strcmp(dst, src) && (head->mftype & MF_LABEL))	/* 不允許同一個卷宗下append, 請改用 move */
      {
	memcpy(&mf, head, sizeof(MF));
	mf.mftype &= ~MF_LABEL;
	rec_add(dst, &mf, sizeof(MF));
	rec_del(src, sizeof(MF), head - data - offset, NULL);
	++offset;
      }
    } while (++head < tail);

    free(data);
  }

}


static int
mf_append(xo)	/* 把有 MF_LABEL 標記的移到當前目錄 */
  XO *xo;
{
  char folder[64];

  if (!mf_has_label(FN_MF, 0))
    return XO_NONE;

  if (vans("要將所有標記移到現在這個卷宗嗎？[y/N]") != 'y')
    return XO_FOOT;

  mf_fpath(folder, cuser.userid, FN_MF);

  mf_do_append(xo->dir, folder);
  mf_do_clear_label(folder, xo->dir);

  return mf_load(xo);
}


static void
mf_do_del_label(src)
  char *src;
{
  int fsize, offset;
  char fpath[64];
  MF *data, *head, *tail;

  if (data = (MF *) f_img(src, &fsize))
  {
    offset = 0;
    head = data;
    tail = data + (fsize / sizeof(MF));
    do
    {
      if (head->mftype & MF_LABEL)
      {
	if (head->mftype & MF_FOLDER)
	{
	  mf_fpath(fpath, cuser.userid, head->xname);
	  mf_do_delete(fpath);
	}
	rec_del(src, sizeof(MF), head - data - offset, NULL);

	++offset;
	continue;
      }

      if (head->mftype & MF_FOLDER)
      {
        mf_fpath(fpath, cuser.userid, head->xname);
        mf_do_del_label(fpath);
      }
    } while (++head < tail);

    free(data);
  }
}


static int
mf_del_label(xo)
  XO *xo;
{
  char folder[64];

  if (!mf_has_label(FN_MF, 0))
    return XO_NONE;

  if (vans("要刪除所有標記嗎？[y/N]") != 'y')
    return XO_FOOT;

  mf_fpath(folder, cuser.userid, FN_MF);
  mf_do_del_label(folder);

  return mf_load(xo);
}


static int
mf_namemode(xo)
  XO *xo;
{
  cuser.ufo ^= UFO_BRDPOST;
  cutmp->ufo = cuser.ufo;
  class_flag ^= UFO_BRDPOST;	/* 和 class 同步 */
  return mf_head(xo);
}


static int
mf_edit(xo)		/* itoc.010110: 我的最愛中看板修改 */
  XO *xo;
{
  MF *mf;

  mf = (MF *) xo_pool + (xo->pos - xo->top);

  if ((mf->mftype & MF_BOARD) && (HAS_PERM(PERM_ALLBOARD | PERM_BM)))
  {
    int bno;

    bno = brd_bno(mf->xname);
    if (bno >= 0)
    {
      if (!HAS_PERM(PERM_ALLBOARD))
      {
	if (!brd_title(bno))
	  return XO_NONE;
      }
      else
	brd_edit(bno);  
      return mf_init(xo);
    }
  }
  return XO_NONE;
}


static int
mf_switch(xo)
  XO *xo;
{
  Select();
  return mf_init(xo);
}


static int
mf_visit(xo)		/* itoc.010402: 看板列表設定看板已讀 */
  XO *xo;
{
  int bno;
  MF *mf;

  mf = (MF *) xo_pool + (xo->pos - xo->top);
  bno = brd_bno(mf->xname);

  if (bno >= 0)		/* itoc.010110: 應該不會 < 0 ? */
  {
    BRD *brd;
    brd = bshm->bcache + bno;
    brh_get(brd->bstamp, bno);
    brh_visit(0);
#ifndef ENHANCED_VISIT
    time4(&brd_visit[bno]);
#endif
  }
  return mf_body(xo);
}


static int
mf_unvisit(xo)		/* itoc.010402: 看板列表設定看板未讀 */
  XO *xo;
{
  int bno;
  MF *mf;

  mf = (MF *) xo_pool + (xo->pos - xo->top);
  bno = brd_bno(mf->xname);

  if (bno >= 0)		/* itoc.010110: 應該不會 < 0 ? */
  {
    BRD *brd;
    brd = bshm->bcache + bno;
    brh_get(brd->bstamp, bno);
    brh_visit(1);
#ifndef ENHANCED_VISIT
    brd_visit[bno] = 0;	/* itoc.010402: 最近瀏覽時間歸零，使看板列表中顯示未讀 */
#endif
  }
  return mf_body(xo);
}


static int
mf_nextunread(xo)
  XO *xo;
{
  int max, pos, bno;
  MF *mf, origmf;
  char folder[64];
  int reload = 0;
  int originpos;

  max = xo->max;
  pos = xo->pos;
  mf = (MF *) xo_pool + (xo->pos - xo->top);

  originpos = pos;
  memcpy(&origmf, mf, sizeof(MF));

  while (++pos < max)
  {
    bno = brd_bno((++mf)->xname);
pagewrap:
    if (bno >= 0 && !(mf->mftype & MF_CLASS) &&
      !(brd_bits[bno] & BRD_Z_BIT) && (brd_bits[bno] & BRD_R_BIT))
    {	/* 跳過分類及 zap 掉、進不去的看板 */
      BRD *brd;
      brd = bshm->bcache + bno;

#ifdef ENHANCED_VISIT
      /* itoc.010407: 改用最後一篇已讀/未讀來判斷 */
      brh_get(brd->bstamp, bno);

      if (brh_unread(brd->blast))
#else
      if (brd->blast > brd_visit[bno])
#endif
      {
	xo->pos = pos;
	if (!reload)
	{
	  move(3 + originpos % XO_TALL, 0);
	  mf_item(originpos + 1, &origmf);
	  return pos + XO_MOVE;
	}
	return mf_body(xo);
      }
    }
    else if (mf->mftype & MF_FOLDER)
    {
      mf_fpath(folder, cuser.userid, mf->xname);
      if (mf_urifolder(folder))
      {
	xo->pos = pos;
	if (!reload)
	{
	  move(3 + originpos % XO_TALL, 0);
	  mf_item(originpos + 1, &origmf);
	  return pos + XO_MOVE;
	}
	return mf_body(xo);
      }
    }
    if ((pos == xo->top + XO_TALL) && (pos < max))	/* 處理翻頁 */
    {
      reload = 1;
      xo->pos = pos;
      xo->pos++;
      xo_load(xo, sizeof(MF));
      mf = (MF *) xo_pool;
      bno = brd_bno(mf->xname);
      goto pagewrap;
    }
  }

  if (reload)	/* 翻頁後找不到要還原 */
  {
    xo->pos = originpos;
    xo_load(xo, sizeof(MF));
  }
  return XO_NONE;
}


static int
mf_info(xo)
  XO *xo;
{
  MF *mf;
  BRD *brd;
  int bno;

  mf = (MF *) xo_pool + (xo->pos - xo->top);
  if (mf->mftype & MF_BOARD)
  {
    if ((bno = brd_bno(mf->xname)) >= 0)
    {
      if (brd_bits[bno] & BRD_L_BIT)
      {
	brd = bshm->bcache + bno;

	move(0, 0);
	clrtoeol();
	prints("   - 英文板名: %-13s                   - 看板分類: %s\n",
	  brd->brdname, brd->class);
	prints("   - 中文板名: %s\n", brd->title);
	if (*brd->BM > ' ')
	  prints("   - 板主名單: %s", brd->BM);
	else
	  prints("   - 本板應徵板主中");

	clrtoeol();
	move(xo->pos - xo->top + 3, 0);
	vkey();
	return mf_head(xo);
      }
    }
  }
  return XO_NONE;
}


static int
mf_help(xo)
  XO *xo;
{
  xo_help("mf");
  return mf_head(xo);
}


#ifndef NEW_KeyFunc
static KeyFunc mf_cb[] =
{
#ifdef  HAVE_LIGHTBAR
  XO_ITEM, mf_item_bar,
#endif
  XO_INIT, mf_init,
  XO_LOAD, mf_load,
  XO_HEAD, mf_head,
  XO_BODY, mf_body,

  'r', mf_browse,
  'd', mf_delete,
  'D', mf_rangedel,
  'o', mf_mark,
  't', mf_label,
  'm', mf_move,
  'T', mf_title,
  'E', mf_edit,
  's', mf_switch,
  'c', mf_namemode,
  'v', mf_visit,
  'V', mf_unvisit,
  '`', mf_nextunread,
  'i', mf_info,

  Ctrl('P'), mf_add,
  'C', mf_copy,
  'g', mf_copy,
  'p', mf_paste,
  Ctrl('V'), mf_paste,
  Ctrl('A'), mf_append,
  Ctrl('T'), mf_clear_label,
  Ctrl('D'), mf_del_label,

  'h', mf_help
};
#else
NewKeyFunc mf_cb[] =
{
#ifdef  HAVE_LIGHTBAR
  XO_ITEM, mf_item_bar, XO_ITEM,        'n',    "XO_ITEM",      "",
#endif
  XO_INIT, mf_init,     XO_INIT,        'n',    "XO_INIT",      "",
  XO_LOAD, mf_load,     XO_LOAD,        'n',    "XO_LOAD",      "",
  XO_HEAD, mf_head,     XO_HEAD,        'n',    "XO_HEAD",      "",
  XO_BODY, mf_body,     XO_BODY,        'n',    "XO_BODY",      "",

  'r', mf_browse,       'r',    'z',    "進入瀏覽",     "",
  'd', mf_delete,       'd',    'p',    "刪除",         "",
  'D', mf_rangedel,     'D',    'p',    "區段刪除",     "",
  'o', mf_mark,         'o',    'p',    "標記不刪",     "",
  't', mf_label,        't',    'p',    "切換標記T",    "",
  'm', mf_move,         'm',    'p',    "移動位置",     "",
  'T', mf_title,        'T',    'p',    "修改標題",     "",
  'E', mf_edit,         'E',    'p',    "修改看板中文敘述",     "",
  's', mf_switch,       's',    'p',    "選取看板",     "",
  'c', mf_namemode,     'c',    'p',    "切換編號/篇數",        "",
  'v', mf_visit,        'v',    'p',    "設定已讀",     "",
  'V', mf_unvisit,      'V',    'p',    "設定未讀",     "",
  '`', mf_nextunread,   '`',    'p',    "跳至下一未讀", "",
  'i', mf_info,         'i',    'p',    "顯示看板資訊", "",

  Ctrl('P'), mf_add,    Ctrl('P'),      'p',    "新增", "",
  'C', mf_copy,         'C',    'p',    "複製",         "",
  'g', mf_copy,         'g',    'p',    "複製",         "",
  'p', mf_paste,        'p',    'p',    "貼上",         "",
  Ctrl('V'), mf_paste,  Ctrl('V'),      'p',    "新增", "",
  Ctrl('A'), mf_append, Ctrl('A'),      'p',	"將全部標記T移至所在目錄",      "",
  Ctrl('T'), mf_clear_label,	Ctrl('T'),	'p',	"清除全部看板之標記T",          "",
  Ctrl('D'), mf_del_label,	Ctrl('D'),	'p',	"刪除全部有標記T之看板",        "",

  'h', mf_help,         'h',    'z',    "功\能說明",     NULL
};
#endif


static void
XoMF(folder)
  char *folder;
{
  XO *xo, *last;

  /* itoc.060706: 當 mf_depth 為 0 時，表示要進 FN_MF，在 mf_main() 已處理過，
     不需要重新 xo_new()，如此可保留首頁的 xo->pos */

  if (mf_depth)
  {
    last = xz[XZ_MF - XO_ZONE].xo;	/* record */

    xz[XZ_MF - XO_ZONE].xo = xo = xo_new(folder);
    xo->pos = 0;
  }

#ifdef AUTO_JUMPBRD
  if (cuser.ufo & UFO_JUMPBRD)
    mf_jumpnext = 1;	/* itoc.020615: 主動跳去下一個未讀看板 */
#endif
  xover(XZ_MF);

  if (mf_depth)
  {
    free(xo);
    xz[XZ_MF - XO_ZONE].xo = last;	/* restore */
  }
}


int
MyFavorite()
{
  /* 從主選單進入我的最愛，mf_depth 一定是 0 */
  XoMF(NULL);

  return 0;
}


void
mf_main()
{
  char fpath[64];
  XO *xo;

  /* itoc.060706.註解: 進站時就要初始化，因為使用者可能一上站就 every_Z 跳去我的最愛 */

  mf_fpath(fpath, cuser.userid, FN_MF);
  xz[XZ_MF - XO_ZONE].xo = xo = xo_new(fpath);
  xz[XZ_MF - XO_ZONE].cb = mf_cb;

  xo->pos = 0;

#ifdef AUTO_JUMPBRD
  if (cuser.ufo & UFO_JUMPBRD)
    mf_jumpnext = 1;	/* itoc.020615: 主動跳去下一個未讀看板 */
#endif
}
#endif				/* MY_FAVORITE */
