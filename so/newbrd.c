/*-------------------------------------------------------*/
/* newbrd.c   ( YZU_CSE WindTop BBS )                    */
/*-------------------------------------------------------*/
/* target : 連署功能    			 	 */
/* create : 00/01/02				 	 */
/* update : 02/04/29				 	 */
/*-------------------------------------------------------*/
/* run/newbrd/_/.DIR - newbrd control header		 */
/* run/newbrd/_/@/@_ - newbrd description file		 */
/* run/newbrd/_/@/G_ - newbrd voted id loG file		 */
/*-------------------------------------------------------*/


#include "bbs.h"


#ifdef HAVE_COSIGN

extern XZ xz[];
extern char xo_pool[];
extern BCACHE *bshm;		/* itoc.010805: 開新板用 */

static int nbrd_add();
static int nbrd_body();
static int nbrd_head();

static char *split_line = "\033[33m──────────────────────────────\033[m\n";

static int check_adm;

typedef struct
{
  char userid[IDLEN + 1];
  char email[60];
}      LOG;


static int
cmpbtime(nbrd)
  NBRD *nbrd;
{
  return nbrd->btime == currchrono;
}


static char
nbrd_attr(nbrd)
  NBRD *nbrd;
{
  int xmode = nbrd->mode;

  /* 筆劃越少的，越傾向結案 */
  if (xmode & NBRD_FINISH)
    return ' ';
  if (xmode & NBRD_END)
    return '-';
#ifdef SYSOP_START_COSIGN
  if (xmode & NBRD_START)
    return '+';
  else
    return 'x';
#else
  return '+';
#endif
}


static int
nbrd_stamp(folder, nbrd, fpath)
  char *folder;
  NBRD *nbrd;
  char *fpath;
{
  char *fname;
  char *family = NULL;
  int rc;
  int token;

  fname = fpath;
  while (rc = *folder++)
  {
    *fname++ = rc;
    if (rc == '/')
      family = fname;
  }

  fname = family;
  *family++ = '@';

  token = time(0);

  archiv32(token, family);

  rc = open(fpath, O_WRONLY | O_CREAT | O_EXCL, 0600);
  nbrd->btime = token;
  str_stamp(nbrd->date, &nbrd->btime);
  strcpy(nbrd->xname, fname);

  return rc;
}


static void
nbrd_fpath(fpath, folder, nbrd)
  char *fpath;
  char *folder;
  NBRD *nbrd;
{
  char *str;
  int cc;

  while (cc = *folder++)
  {
    *fpath++ = cc;
    if (cc == '/')
      str = fpath;
  }
  strcpy(str, nbrd->xname);
}


static int
nbrd_init(xo)
  XO *xo;
{
  xo_load(xo, sizeof(NBRD));
  return nbrd_head(xo);
}


static int
nbrd_load(xo)
  XO *xo;
{
  xo_load(xo, sizeof(NBRD));
  return nbrd_body(xo);
}


static void
nbrd_item(num, nbrd)
  int num;
  NBRD *nbrd;
{
  prints("%6d %c %-5s %-13s [%-13s] %.*s\n", 
    num, nbrd_attr(nbrd), nbrd->date + 3, nbrd->owner, 
    (nbrd->mode & NBRD_NEWBOARD) ? nbrd->brdname : "\033[1;33m本站連署\033[m", d_cols + 20, nbrd->title);
}


#ifdef HAVE_LIGHTBAR
static int
nbrd_item_bar(xo, mode)
  XO *xo;
  int mode;     /* 1:上光棒  0:去光棒 */
{
  NBRD *nbrd;

  nbrd = (NBRD *) xo_pool + xo->pos - xo->top;

  prints("%s%6d %c %-5s %-13s [%-13s%s] %-*.*s%s%s",
    mode ? UCBAR[UCBAR_NBRD] : "",         //這裡是光棒的顏色，可以自己改
    xo->pos + 1, nbrd_attr(nbrd), nbrd->date + 3, nbrd->owner,
    (nbrd->mode & NBRD_NEWBOARD) ? nbrd->brdname : "\033[1;33m本站連署\033[m",
    mode ? UCBAR[UCBAR_NBRD] : "", d_cols + 22, d_cols + 22, nbrd->title,
    (nbrd->mode & NBRD_NEWBOARD) ? "           " :"                ",
    mode ? "\033[m" : "");

  move(xo->pos - xo->top + 3, 0);
  return XO_NONE;
}
#endif


static int
nbrd_body(xo)
  XO *xo;
{
  NBRD *nbrd;
  int num, max, tail;

  max = xo->max;
  if (max <= 0)
  {
    if (vans("要新增連署項目嗎(Y/N)？[N] ") == 'y')
      return nbrd_add(xo);
    return XO_QUIT;
  }

  nbrd = (NBRD *) xo_pool;
  num = xo->top;
  tail = num + XO_TALL;

  if (max > tail)
    max = tail;

  move(3, 0);  
  do
  {
    nbrd_item(++num, nbrd++);
  } while (num < max);
  clrtobot();

  return XO_FOOT;
}


static int
nbrd_head(xo)
  XO *xo;
{
  vs_head("申請看板", str_site);
  prints(NECKER_COSIGN, d_cols, "");
  return nbrd_body(xo);
}


static int
nbrd_find(fpath, brdname)
  char *fpath, *brdname;
{
  NBRD old;
  int fd;
  int rc = 0;

  if ((fd = open(fpath, O_RDONLY)) >= 0)
  {
    while (read(fd, &old, sizeof(NBRD)) == sizeof(NBRD))
    {
      if (!str_cmp(old.brdname, brdname) && !(old.mode & NBRD_FINISH))
      {
	rc = 1;
	break;
      }
    }
    close(fd);
  }
  return rc;
}


static int
nbrd_add(xo)
  XO *xo;
{
  int i, fd, ans, days, numbers, ntype, readlevel, isinn;
  char *dir, fpath[64], path[64], buf[32], group[64];
  char class[BCLEN + 1], sub[BNLEN + 1];
  char *brdname, *title, *prefix, *plevel;
  FILE *fp;
  NBRD nbrd;

  if (HAS_PERM(PERM_ALLADMIN))
  {
    ans = vans("連署模式 1)開新板 2)記名 3)無記名：[Q] ");
    if (ans < '1' || ans > '3')
      return xo->max ? XO_FOOT : nbrd_body(xo);	/* itoc.020122: 如果沒有任何連署，要回到 nbrd_body() */
    /* itoc.030613: 其實下面的 return XO_FOOT; 也應該這樣改 */
  }
  else if (HAS_PERM(PERM_POST))
  {
    /* 一段使用者只能開新板連署 */
    ans = '1';
  }
  else
  {
    vmsg("對不起，本看板是唯讀的");
    return XO_FOOT;
  }

  memset(&nbrd, 0, sizeof(NBRD));

  brdname = nbrd.brdname;
  title = nbrd.title;

  clear();
  vs_bar("申請看板");
  if (ans == '1')	/* 新板連署 */
  {
    move (i = 2, 0);
    outs("          \033[1;33m1)\033[m個人\033[1;36m/\033[m情侶     "
	"\033[1;33m2)\033[m寢板     \033[1;33m3)\033[m實驗室\n"
	"          \033[1;33m4)\033[m社團\033[1;36m/\033[m校友會   "
	"\033[1;33m5)\033[m各系(級)所相關|活動板\033[1;36m/\033[m官方課程板\n"
	"          \033[1;33m6)\033[m校隊\033[1;36m/\033[m校務看板 "
	"\033[1;33m7)\033[m團體板\033[1;36m/\033[m課程小組討論板 "
	"\033[1;33m8)\033[m綜合\033[1;36m/\033[m其他\n");
    i += 3;
    if (!vget(i, 0, "開板種類？[Q] ", buf, 3, LCECHO))
      return XO_HEAD;

    switch (ntype = (*buf - '0'))
    {
    case 1:
      plevel = "個人";
      prefix = "P_";
      break;
    case 2:
      plevel = "寢板";
      prefix = "R_";
      break;
    case 3:
      plevel = "累伯";
      prefix = "LAB_";
      break;
    case 4:
      plevel = "社團";
      prefix = "";
      move(++i, 0);
      outs("\n                  1. [社團主要看板] 英文板名應為：xxxxxxxxxxxx\n");
      outs("                  2. 若您想申請社團的附屬看板，\n");
      outs("                         板名應為：[英文縮寫].xxxxxxx，\n");
      outs("                         [社團分類簡稱]、[英文縮寫] 請填寫一致的分類名稱\n");
      i += 4;
      break;
    case 5:
      plevel = prefix = "";	/* 讓站長自己手動改 */
      move(++i, 0);
      outs("\n                系(級)板：[系名].[系級]        例：cs.12 清大資工 12 級級板\n");
      outs("                  活動板：[系名].xxxxxx        例：cs.camp09 資工2009營隊板\n");
      outs("                  課程板：以課務組之課號為板名 例：isa5571  (限教師或助教開板)\n");
      i += 3;
      break;
    case 6:
      plevel = prefix = "";
      move(++i, 0);
      outs("\n                  校隊看板： nthu_xxxxxxx\n");
      outs("                  校務看板： nthu.xxxxxxx\n");
      i += 2;
      break;
    case 7:
      plevel = "團體";
      prefix = "G_";
      move(++i, 0);
      outs("\n                若已您的團體已有其他看板，[團體分類簡稱] 請填寫一致的分類名稱\n");
      i++;
      break;
    case 8:
      plevel = prefix = "";
      break;
    default:
      return XO_HEAD;
    }

    if (ntype == 4)
    {
      if (vget(++i, 0, "社團分類簡稱：", class, BCLEN + 1, DOECHO) && strlen(class) == 4)
	plevel = class;

      if (vget(++i, 0, "是否要申請 [社團主要看板](y/n)？[N] ", buf, 3, LCECHO) != 'y')
      {
	if (!vget(++i, 0, "社團英文縮寫：", sub, sizeof(nbrd.brdname) - 3, DOECHO))
	  return XO_HEAD;
	strcat(sub, ".");
	prefix = sub;
      }
    }
    else if (ntype == 7)
    {
      if (vget(++i, 0, "團體分類簡稱：", class, BCLEN + 1, DOECHO) && strlen(class) == 4)
	plevel = class;
    }

    sprintf(buf, "英文板名：%s", prefix);
    if (!vget(++i, 0, buf, brdname, sizeof(nbrd.brdname) - strlen(prefix), DOECHO))
      return XO_HEAD;

    sprintf(buf, "%s%s", prefix, brdname);
    if (brd_bno(buf) >= 0 || !valid_brdname(buf))
    {
      vmsg("已有此板或板名不合法");
      return XO_HEAD;
    }
    if (nbrd_find(xo->dir, buf))
    {
      vmsg("已有此看板的申請案");
      return XO_HEAD;
    }

    strcpy(brdname, buf);
    strcpy(nbrd.class, plevel);

    if (ntype >= 4 && ntype <= 7)
    {
      switch (ntype)
      {
      case 4:
	vget(++i, 0, "看板所屬社團/社團種類：", group, 50, DOECHO);
	break;
      case 5:
	vget(++i, 0, "看板所屬系(級)所/活動/課程：", group, 50, DOECHO);
	break;
      case 6:
	vget(++i, 0, "看板所屬校隊/學校處室：", group, 50, DOECHO);
	break;
      }
      if (!*group)
	return XO_HEAD;
    }

    if (!vget(++i, 0, "中文板名：", title, sizeof(nbrd.title), DOECHO))
      return XO_HEAD;

    readlevel = vget(++i, 0, "看板屬性：1)公開 2)秘密 3)好友 [Q] ", buf, 3, DOECHO) - '0';
    switch (readlevel)
    {
    case 1:
      plevel = "公開";
      break;
    case 2:
      plevel = "\033[1;31m秘密\033[m";
      break;
    case 3:
      plevel = "\033[1;33m好友\033[m";
      break;
    default:
      return XO_HEAD;
    }

    days = NBRD_DAY_BRD;
    numbers = NBRD_NUM_BRD;

#ifdef SYSOP_START_COSIGN
    nbrd.mode = NBRD_NEWBOARD;
#else
    nbrd.mode = NBRD_NEWBOARD | NBRD_START;
#endif
  }
  else			/* 其他連署 */
  {
    char tmp[8];

    if (!vget(b_lines, 0, "連署主題：", title, sizeof(nbrd.title), DOECHO))
      return XO_FOOT;

    /* 連署日期最多 30 天，連署人數最多 500 人 */
    if (!vget(b_lines, 0, "連署天數：", tmp, 5, DOECHO))
      return XO_FOOT;
    days = atoi(tmp);
    if (days > 30 || days < 1)
      return XO_FOOT;
    if (!vget(b_lines, 0, "連署人數：", tmp, 6, DOECHO))
      return XO_FOOT;
    numbers = atoi(tmp);
    if (numbers > 500 || numbers < 1)
      return XO_FOOT;

    nbrd.mode = (ans == '2') ? (NBRD_OTHER | NBRD_START) : (NBRD_OTHER | NBRD_START | NBRD_ANONYMOUS);
  }

  if (vget(++i, 0, "是否申請轉信(Y/N)？[N] ", buf, 3, LCECHO) == 'y')
  {
    isinn = 1;
    strcpy(nbrd.innsrv, "group.nctu.edu.tw");
    strcpy(nbrd.inngrp, "group.nthucs.");
    if (!vget(++i, 0, "轉信站台名稱：", nbrd.innsrv, sizeof(nbrd.innsrv), GCARRY) ||
      !vget(++i, 0, "轉信群組名稱：", nbrd.inngrp, sizeof(nbrd.inngrp), GCARRY) ||
      !str_cmp(nbrd.inngrp, "group.nthucs."))
    {
      vmsg("取消設定轉信");
      isinn = 0;
      nbrd.innsrv[0] = nbrd.inngrp[0] = '\0';
    }
  }
  else
    isinn = 0;

  if (isinn)
    nbrd.mode |= NBRD_INN;

  if ((ntype >= 4 && ntype <= 6) || ntype == 8)
    nbrd.mode |= NBRD_PUBLIC;

  vmsg("開始編輯 [看板說明與板主抱負]，不需再將申請看板的資料貼上一次！");
  sprintf(path, "tmp/%s.nbrd", cuser.userid);	/* 連署原因的暫存檔案 */
  if (fd = vedit(path, 0))
  {
    unlink(path);
    vmsg(msg_cancel);
    return nbrd_head(xo);
  }

  dir = xo->dir;
  if ((fd = nbrd_stamp(dir, &nbrd, fpath)) < 0)
    return nbrd_head(xo);
  close(fd);

  if (ans != '1' || ntype >= 7)
  {
    nbrd.etime = nbrd.btime + days * 86400;
    nbrd.mode = NBRD_NEWBOARD;
    nbrd.total = (ntype == 8) ? 20 : numbers;
  }
  else
  {
    nbrd.mode |= NBRD_END;	/* 不用連署，申請就開的看板 */
  }

  nbrd.readlevel = readlevel;
  strcpy(nbrd.owner, cuser.userid);

  fp = fopen(fpath, "a");
  fprintf(fp, "作者: %s (%s) 站內: newboard\n", cuser.userid, cuser.username);
  if (ans == '1')
    fprintf(fp, "標題: [提案] 申請 %s 看板\n", brdname);
  else
    fprintf(fp, "標題: %s\n", title);
  fprintf(fp, "時間: %s\n\n", Now());

  if (ans == '1')
  {
    fprintf(fp, "英文板名：%s\n", brdname);
    fprintf(fp, "中文板名：%s\n", title);
    fprintf(fp, "板主名單：%s\n", cuser.userid);
    fprintf(fp, "電子信箱：%s\n", cuser.email);
    fprintf(fp, "看板分類：%s\n", nbrd.class);
    if (ntype >= 4 && ntype <= 7)
    {
      switch (ntype)
      {
      case 4:
	fprintf(fp, "    所屬社團/社團種類：%s\n", group);
	break;
      case 5:
	fprintf(fp, "    所屬系(級)所/活動/課程：%s\n", group);
	break;
      case 6:
	fprintf(fp, "    所屬校隊/學校處室：%s\n", group);
	break;
      }
    }
    fprintf(fp, "看板屬性：%s\n", plevel);
    if (isinn)
    {
      fprintf(fp, "申請轉信：\n    站台：%s\n", nbrd.innsrv);
      fprintf(fp, "    群組：%s\n", nbrd.inngrp);
    }
  }
  else
  {
    fprintf(fp, "連署主題：%s\n", title);
  }

  if (ans != '1' || ntype >= 7)
  {
    fprintf(fp, "舉辦日期：%s\n", nbrd.date);
    fprintf(fp, "到期天數：%d\n", days);
    fprintf(fp, "需連署人：%d\n", numbers);
    fprintf(fp, split_line);
    fprintf(fp, "連署說明：\n");
  }

  f_suck(fp, path);
  unlink(path);
  fprintf(fp, "\n--\n※ 本文章由 %s 從 %s 申請\n", cuser.userid, fromhost);
  if (ans != '1' || ntype >= 7)
    fprintf(fp, split_line);
  fclose(fp);

  rec_add(dir, &nbrd, sizeof(NBRD));

#ifdef SYSOP_START_COSIGN
  vmsg(ans == '1' ? "送交申請了，請等候核准吧" : "連署開始了！");
#else
  if (ans != '1' || ntype >= 7)
    vmsg("連署開始了！");
  else
    vmsg("請等待站長開板！");
#endif
  return nbrd_init(xo);
}


static int
nbrd_seek(fpath)
  char *fpath;
{
  LOG old;
  int fd;
  int rc = 0;

  if ((fd = open(fpath, O_RDONLY)) >= 0)
  {
    while (read(fd, &old, sizeof(LOG)) == sizeof(LOG))
    {
      if (!strcmp(old.userid, cuser.userid) || !str_cmp(old.email, cuser.email))
      {
	rc = 1;
	break;
      }
    }
    close(fd);
  }
  return rc;
}


static void
addreply(hdd, ram)
  NBRD *hdd, *ram;
{
  if (--hdd->total <= 0)
  {
    if (hdd->mode & NBRD_NEWBOARD)	/* 新板連署掛 END */
      hdd->mode |= NBRD_END;
    else				/* 其他連署掛 FINISH */
      hdd->mode |= NBRD_FINISH;
  }
}


static int
nbrd_reply(xo)
  XO *xo;
{
  NBRD *nbrd;
  char *fname, fpath[64], reason[80];
  LOG mail;

  nbrd = (NBRD *) xo_pool + (xo->pos - xo->top);
  fname = NULL;

  if (nbrd->mode & (NBRD_FINISH | NBRD_END))
    return XO_NONE;

#ifdef SYSOP_START_COSIGN
  if (!(nbrd->mode & NBRD_START))
  {
    vmsg("尚未開始連署");
    return XO_FOOT;
  }
#endif

  if (time(0) >= nbrd->etime)
  {
    currchrono = nbrd->btime;
    if (nbrd->mode & NBRD_NEWBOARD)	/* 新板連署掛 END */
    {
      if (!(nbrd->mode & NBRD_END))
      {
	nbrd->mode ^= NBRD_END;
	currchrono = nbrd->btime;
	rec_put(xo->dir, nbrd, sizeof(NBRD), xo->pos, cmpbtime);
      }
    }
    else				/* 其他連署掛 FINISH */
    {
      if (!(nbrd->mode & NBRD_FINISH))
      {
	nbrd->mode ^= NBRD_FINISH;
	currchrono = nbrd->btime;
	rec_put(xo->dir, nbrd, sizeof(NBRD), xo->pos, cmpbtime);
      }
    }
    vmsg("連署已經截止了");
    return XO_FOOT;
  }


  /* --------------------------------------------------- */
  /* 檢查是否已經連署過					 */
  /* --------------------------------------------------- */

  nbrd_fpath(fpath, xo->dir, nbrd);
  fname = strrchr(fpath, '@');
  *fname = 'G';

  if (nbrd_seek(fpath))
  {
    vmsg("您已經連署過了！");
    return XO_FOOT;
  }

  /* --------------------------------------------------- */
  /* 開始連署						 */
  /* --------------------------------------------------- */

  *fname = '@';

  if (vans("要加入連署嗎(Y/N)？[N] ") == 'y' && 
    vget(b_lines, 0, "我有話要說：", reason, 65, DOECHO))
  {
    FILE *fp;

    if (fp = fopen(fpath, "a"))
    {
      if (nbrd->mode & NBRD_ANONYMOUS)
	fprintf(fp, "%3d -> " STR_ANONYMOUS "\n    %s\n", nbrd->total, reason);
      else
	fprintf(fp, "%3d -> %s (%s)\n    %s\n", nbrd->total, cuser.userid, cuser.email, reason);
      fclose(fp);
    }

    currchrono = nbrd->btime;
    rec_ref(xo->dir, nbrd, sizeof(NBRD), xo->pos, cmpbtime, addreply);

    memset(&mail, 0, sizeof(LOG));
    strcpy(mail.userid, cuser.userid);
    strcpy(mail.email, cuser.email);
    *fname = 'G';
    rec_add(fpath, &mail, sizeof(LOG));

    vmsg("加入連署完成");
    return nbrd_init(xo);
  }

  return XO_FOOT;
}


#ifdef SYSOP_START_COSIGN
static int
nbrd_start(xo)
  XO *xo;
{
  NBRD *nbrd;
  char fpath[64], buf[80], tmp[10];
  time_t etime;

  if (!HAS_PERM(PERM_ALLBOARD))
    return XO_NONE;

  nbrd = (NBRD *) xo_pool + (xo->pos - xo->top);

  if (nbrd->mode & (NBRD_FINISH | NBRD_END | NBRD_START))
    return XO_NONE;

  if (vans("請確定開始連署(Y/N)？[N] ") != 'y')
    return XO_FOOT;

  nbrd_fpath(fpath, xo->dir, nbrd);
  etime = time(0) + NBRD_DAY_BRD * 86400;

  str_stamp(tmp, &etime);
  sprintf(buf, "開始連署：      到期日期：%s\n", tmp);
  f_cat(fpath, buf);
  f_cat(fpath, split_line);

  nbrd->etime = etime;
  nbrd->mode ^= NBRD_START;
  currchrono = nbrd->btime;
  rec_put(xo->dir, nbrd, sizeof(NBRD), xo->pos, cmpbtime);

  return nbrd_head(xo);
}
#endif


static int
nbrd_finish(xo)
  XO *xo;
{
  NBRD *nbrd;
  char fpath[64], path[64];
  int fd;
  FILE *fp;

  if (!HAS_PERM(PERM_ALLBOARD))
    return XO_NONE;

  nbrd = (NBRD *) xo_pool + (xo->pos - xo->top);

  if (nbrd->mode & NBRD_FINISH)
    return XO_NONE;

  if (vans("請確定結束連署(Y/N)？[N] ") != 'y')
    return XO_FOOT;

  vmsg("請編輯結束連署原因");
  sprintf(path, "tmp/%s", cuser.userid);	/* 連署原因的暫存檔案 */
  if (fd = vedit(path, 0))
  {
    unlink(path);
    vmsg(msg_cancel);
    return nbrd_head(xo);
  }

  nbrd_fpath(fpath, xo->dir, nbrd);

  f_cat(fpath, "結束連署原因：\n\n");
  fp = fopen(fpath, "a");
  f_suck(fp, path);
  fclose(fp);
  f_cat(fpath, split_line);
  unlink(path);

  nbrd->mode ^= NBRD_FINISH;
  currchrono = nbrd->btime;
  rec_put(xo->dir, nbrd, sizeof(NBRD), xo->pos, cmpbtime);

  return nbrd_head(xo);
}


static void
add_class(brd, class_name)
  BRD *brd;
  char *class_name;
{
  HDR hdr;
  char fpath[64];

  sprintf(fpath, "gem/@/@%s", class_name);

  /* 加入適當的分類 */
  brd2gem(brd, &hdr);
  rec_add(fpath, &hdr, sizeof(HDR));
}


static int			/* 1:開板成功 */
nbrd_newbrd(xo, nbrd)		/* 開新板 */
  XO *xo;
  NBRD *nbrd;
{
  BRD newboard;
  ACCT acct;

  /* itoc.030519: 避免重覆開板 */
  if (brd_bno(nbrd->brdname) >= 0)
  {
    vmsg("已有此板");
    return 1;
  }

  memset(&newboard, 0, sizeof(BRD));

  /* itoc.010805: 新看板預設 battr = 不轉信; postlevel = PERM_POST; 看板板主為提起連署者 */
  /* itoc.010805: 新看板預設 battr = 不轉信; 看板板主為提起連署者 */
  switch (nbrd->readlevel)
  {
  case 2:	/* 秘密 */
    newboard.readlevel = PERM_SYSOP;
    newboard.battr = BRD_NOTRAN | BRD_NOSTAT | BRD_NOVOTE;
    break;
  case 3:	/* 好友 */
    newboard.readlevel = PERM_BOARD;
    newboard.battr = BRD_NOTRAN | BRD_NOSTAT | BRD_NOVOTE;
    break;
  default:	/* 公開 */
    newboard.postlevel = PERM_POST;
    newboard.battr = BRD_NOTRAN;
    break;
  }
  if (nbrd->mode & NBRD_PUBLIC)
    newboard.battr = BRD_PUBLIC;
  newboard.postlevel = PERM_POST;
  strcpy(newboard.brdname, nbrd->brdname);
  strcpy(newboard.class, nbrd->class);
  strcpy(newboard.title, nbrd->title);
  strcpy(newboard.BM, nbrd->owner);

  if (acct_load(&acct, nbrd->owner) >= 0)
    acct_setperm(&acct, PERM_BM, 0);

  if (brd_new(&newboard) < 0)
    return 0;

  /* 加入分類群組 */
  add_class(&newboard, "NewBoard");

  vmsg("新板成立，現在加入分類群組");

  XoGem("gem/@/@Class", "看板精華", (GEM_W_BIT | GEM_X_BIT | GEM_M_BIT));
  nbrd_init(xo);

  return 1;
}


static int	/* 1:成功 0:失敗 */
nbrd_nf_add(nbrd)
  NBRD *nbrd;
{
  nodelist_t nl;
  newsfeeds_t nf;
  int fd, high;
  char fpath[32], ans[12];
  BRD *brd;

  strcpy(fpath, "innd/newsfeeds.bbs");
  memset(&nf, 0, sizeof(newsfeeds_t));

  /* 找出轉信站台在 nodelist.bbs 中的資訊 */
  if ((fd = open("innd/nodelist.bbs", O_RDONLY)) >= 0)
  {
    while (read(fd, &nl, sizeof(nodelist_t)) == sizeof(nodelist_t))
    {
      if (!strcmp(nl.host, nbrd->innsrv))
      {
	strcpy(nf.path, nl.name);
	break;
      }
    }
    close(fd);
  }

  nf.high = INT_MAX;		/* 第一次取信強迫 reload */
  strcpy(nf.board, nbrd->brdname);
  strcpy(nf.newsgroup, nbrd->inngrp);

  if (!*nf.path)
  {
    vmsg("轉信設定站台尚未被設定在 newsfeeds.bbs 中，請手動設定轉信！");
    return 0;
  }

  if (vget(b_lines, 0, "英文站名：", nf.path, sizeof(nf.path), GCARRY) &&
    vget(b_lines, 0, "群組：", nf.newsgroup, /* sizeof(nf.newsgroup) */ 70, GCARRY))
  {
    if (!vget(b_lines, 0, "字集 [" MYCHARSET "]：", nf.charset, sizeof(nf.charset), GCARRY))
      str_ncpy(nf.charset, MYCHARSET, sizeof(nf.charset));
    nf.xmode = (vans("是否轉進(Y/N)？[Y] ") == 'n') ? INN_NOINCOME : 0;

    if (vans("是否更改轉信的 high-number 設定，這設定對被餵信的群組無效(Y/N)？[N] ") == 'y')
    {
      sprintf(ans, "%d", nf.high);
      vget(b_lines, 0, "目前篇數：", ans, 11, GCARRY);
      if ((high = atoi(ans)) >= 0)
	nf.high = high;
    }

    rec_add(fpath, &nf, sizeof(newsfeeds_t));

    high = brd_bno(nf.board);
    brd = bshm->bcache + high;
    if ((brd->battr & BRD_NOTRAN) && vans("本板屬性目前為不轉出，是否改為轉出(Y/N)？[Y] ") != 'n')
    {
      brd->battr &= ~BRD_NOTRAN;
      rec_put(FN_BRD, brd, sizeof(BRD), high, NULL);
    }

    return 1;
  }
  return 0;
}


static void
personal_bm_list(userid)			/* 顯示 userid 是哪些個人板的板主 */
  char *userid;
{
  int len;
  char *list;
  BRD *bhead, *btail;

  len = strlen(userid);
  outs("  \033[32m擔任個人板主：\033[37m");	/* itoc.010922: 換 user info 版面 */

  bhead = bshm->bcache;
  btail = bhead + bshm->number;

  do
  {
    if (str_ncmp(bhead->brdname, "P_", 2))
      continue;

    list = bhead->BM;
    if (str_has(list, userid, len))
    {
      outs(bhead->brdname);
      outc(' ');
    }
  } while (++bhead < btail);

  outc('\n');
}


static int
nbrd_open(xo)		/* itoc.010805: 開新板連署，連署完畢開新看板 */
  XO *xo;
{
  NBRD *nbrd;
  char fpath[64], title[TTLEN + 1], buf[256];
  time_t now;
  struct tm *ptime;

  if (!HAS_PERM(PERM_ALLBOARD))
    return XO_NONE;

  nbrd = (NBRD *) xo_pool + (xo->pos - xo->top);

  if (nbrd->mode & NBRD_FINISH || !(nbrd->mode & NBRD_NEWBOARD))
    return XO_NONE;

  if (!check_adm)
  {
    if (!adm_check())
      return XO_FOOT;
    check_adm = 1;
  }

  if (vans("請確定開啟看板(Y/N)？[N] ") == 'y')
  {
    if (!str_ncmp(nbrd->brdname, "P_", 2))	/* 讓站務檢查申請人已擔任哪些看板板主 */
    {
      clear();
      move(1, 0);
      personal_bm_list(nbrd->owner);
      if (vans("是否繼續(Y/N)？[N] ") != 'y')
	return nbrd_init(xo);
    }

    if (nbrd_newbrd(xo, nbrd))
    {
      nbrd->mode ^= NBRD_FINISH;
      currchrono = nbrd->btime;
      rec_put(xo->dir, nbrd, sizeof(NBRD), xo->pos, cmpbtime);

      if ((nbrd->mode & NBRD_INN) && vans("是否一併設定看板轉信(Y/N)？[Y] ") != 'n')
	nbrd_nf_add(nbrd);

      time(&now);
      ptime = localtime(&now);
      sprintf(buf, "\033[1;30m== %s\033[m：\033[1;33m%-*s\033[1;30m%02d/%02d %02d:%02d:%02d\033[m\n",
         cuser.userid, 56 - strlen(cuser.userid), "申請案通過",
      ptime->tm_mon + 1, ptime->tm_mday, ptime->tm_hour, ptime->tm_min, ptime->tm_sec);
      nbrd_fpath(fpath, xo->dir, nbrd);
      f_cat(fpath, buf);

      sprintf(title, "[開板] %s", nbrd->brdname);
      add_post("newboard", fpath, title, nbrd->owner, "", POST_MARKED | POST_SCORE, NULL);
    }
    return nbrd_init(xo);
  }

  return XO_FOOT;
}


static int
nbrd_browse(xo)
  XO *xo;
{
  int key;
  NBRD *nbrd;
  char fpath[80];

  /* itoc.010304: 為了讓閱讀到一半也可以加入連署，考慮 more 傳回值 */
  for (;;)
  {
    nbrd = (NBRD *) xo_pool + (xo->pos - xo->top);
    nbrd_fpath(fpath, xo->dir, nbrd);

    if ((key = more(fpath, FOOTER_COSIGN)) < 0)
      break;

    if (!key)
      key = vkey();

    switch (key)
    {
    case KEY_UP:
    case KEY_PGUP:
    case '[':
    case 'k':
      key = xo->pos - 1;

      if (key < 0)
        break;

      xo->pos = key;

      if (key <= xo->top)
      {
	xo->top = (key / XO_TALL) * XO_TALL;
	nbrd_load(xo);
      }
      continue;

    case KEY_DOWN:
    case KEY_PGDN:
    case ']':
    case 'j':
    case ' ':
      key = xo->pos + 1;

      if (key >= xo->max)
        break;

      xo->pos = key;

      if (key >= xo->top + XO_TALL)
      {
	xo->top = (key / XO_TALL) * XO_TALL;
	nbrd_load(xo);
      }
      continue;

    case 'y':
    case 'r':
      nbrd_reply(xo);
      break;

    case 'h':
      xo_help("cosign");
      break;
    }
    break;
  }

  return nbrd_head(xo);
}


static int
nbrd_delete(xo)
  XO *xo;
{
  NBRD *nbrd;
  char *fname, fpath[80];
  char *list = "@G";		/* itoc.註解: 清 newbrd file */

  nbrd = (NBRD *) xo_pool + (xo->pos - xo->top);
  if (strcmp(cuser.userid, nbrd->owner) && !HAS_PERM(PERM_ALLBOARD))
    return XO_NONE;

  if (nbrd->mode & NBRD_FINISH)
    return XO_NONE;

  if (vans(msg_del_ny) != 'y')
    return XO_FOOT;

  nbrd_fpath(fpath, xo->dir, nbrd);
  fname = strrchr(fpath, '@');
  while (*fname = *list++)
  {
    unlink(fpath);	/* Thor: 確定名字就砍 */
  }

  currchrono = nbrd->btime;
  rec_del(xo->dir, sizeof(NBRD), xo->pos, cmpbtime);
  return nbrd_init(xo);
}


static int
nbrd_edit(xo)
  XO *xo;
{
  if (HAS_PERM(PERM_ALLBOARD))
  {
    char fpath[64];
    NBRD *nbrd;

    nbrd = (NBRD *) xo_pool + (xo->pos - xo->top);
    nbrd_fpath(fpath, xo->dir, nbrd);
    vedit(fpath, 0);
    return nbrd_head(xo);
  }

  return XO_NONE;
}


static int
nbrd_setup(xo)
  XO *xo;
{
  int numbers;
  char ans[6];
  NBRD *nbrd, newnh;

  if (!HAS_PERM(PERM_ALLBOARD))
    return XO_NONE;

  vs_bar("連署設定");
  nbrd = (NBRD *) xo_pool + (xo->pos - xo->top);
  memcpy(&newnh, nbrd, sizeof(NBRD));

  prints("看板名稱：%s\n看板說明：%4.4s %s\n連署發起：%s\n",
    newnh.brdname, newnh.class, newnh.title, newnh.owner);
  prints("開始時間：%s\n", Btime(&newnh.btime));
  prints("結束時間：%s\n", Btime(&newnh.etime));
  prints("還需人數：%d\n", newnh.total);

  if (vget(8, 0, "(E)設定 (Q)取消？[Q] ", ans, 3, LCECHO) == 'e')
  {
    vget(11, 0, MSG_BID, newnh.brdname, BNLEN + 1, GCARRY);
    vget(12, 0, "看板分類：", newnh.class, sizeof(newnh.class), GCARRY);
    vget(13, 0, "看板主題：", newnh.title, sizeof(newnh.title), GCARRY);
    sprintf(ans, "%d", newnh.total);
    vget(14, 0, "連署人數：", ans, 6, GCARRY);
    numbers = atoi(ans);
    if (numbers <= 500 && numbers >= 1)
      newnh.total = numbers;

    if (memcmp(&newnh, nbrd, sizeof(newnh)) && vans(msg_sure_ny) == 'y')
    {
      memcpy(nbrd, &newnh, sizeof(NBRD));
      currchrono = nbrd->btime;
      rec_put(xo->dir, nbrd, sizeof(NBRD), xo->pos, cmpbtime);
    }
  }

  return nbrd_head(xo);
}


static int
nbrd_uquery(xo)
  XO *xo;
{
  NBRD *nbrd;

  nbrd = (NBRD *) xo_pool + (xo->pos - xo->top);

  move(1, 0);
  clrtobot();
  my_query(nbrd->owner);
  return nbrd_head(xo);
}


static int
nbrd_usetup(xo)
  XO *xo;
{
  NBRD *nbrd;
  ACCT acct;

  if (!HAS_PERM(PERM_ALLACCT))
    return XO_NONE;

  nbrd = (NBRD *) xo_pool + (xo->pos - xo->top);
  if (acct_load(&acct, nbrd->owner) < 0)
    return XO_NONE;

  move(3, 0);
  acct_setup(&acct, 1);
  return nbrd_head(xo);
}


static int
nbrd_help(xo)
  XO *xo;
{
  xo_help("cosign");
  return nbrd_head(xo);
}


static KeyFunc nbrd_cb[] =
{
#ifdef HAVE_LIGHTBAR
  XO_ITEM, nbrd_item_bar,
#endif
  XO_INIT, nbrd_init,
  XO_LOAD, nbrd_load,
  XO_HEAD, nbrd_head,
  XO_BODY, nbrd_body,

  'y', nbrd_reply,
  'r', nbrd_browse,
  'o', nbrd_open,
#ifdef SYSOP_START_COSIGN
  's', nbrd_start,
#endif
  'c', nbrd_finish,
  'd', nbrd_delete,
  'E', nbrd_edit,
  'B', nbrd_setup,

  Ctrl('P'), nbrd_add,
  Ctrl('Q'), nbrd_uquery,
  Ctrl('O'), nbrd_usetup,

  'h', nbrd_help
};


int
XoNewBoard()
{
  XO *xo;
  char fpath[64];

  check_adm = 0;
  sprintf(fpath, "run/newbrd/%s", fn_dir);
  xz[XZ_COSIGN - XO_ZONE].xo = xo = xo_new(fpath);
  xz[XZ_COSIGN - XO_ZONE].cb = nbrd_cb;
  xo->key = XZ_COSIGN;
  xo->pos = XO_TAIL;
  xover(XZ_COSIGN);
  free(xo);

  return 0;
}
#endif	/* HAVE_COSIGN */
