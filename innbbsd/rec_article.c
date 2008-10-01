/*-------------------------------------------------------*/
/* rec_article.c( NTHU CS MapleBBS Ver 3.10 )		 */
/*-------------------------------------------------------*/
/* target : innbbsd receive article			 */
/* create : 95/04/27					 */
/* update :   /  /  					 */
/* author : skhuang@csie.nctu.edu.tw			 */
/* modify : itoc.bbs@bbs.tnfsh.tn.edu.tw		 */
/*-------------------------------------------------------*/


#if 0
   收到之文章內容和檔頭分別在
   內文 (body)   在 char *BODY
   檔頭 (header) 在 char *SUBJECT, *FROM, *SITE, *DATE, *PATH, *GROUP, *MSGID, *POSTHOST, *CONTROL;
#endif


#include "innbbsconf.h"
#include "bbslib.h"
#include "inntobbs.h"

static char bbs_dog_str[80];
static char bbs_dog_title[80];

/* ----------------------------------------------------- */
/* board：shm 部份須與 cache.c 相容			 */
/* ----------------------------------------------------- */


static BCACHE *bshm;


void
init_bshm()
{
  /* itoc.030727: 在開啟 bbsd 之前，應該就要執行過 account，
     所以 bshm 應該已設定好 */

  bshm = shm_new(BRDSHM_KEY, sizeof(BCACHE));

  if (bshm->uptime <= 0)	/* bshm 未設定完成 */
    exit(0);
}


/* ----------------------------------------------------- */
/* 處理 DATE						 */
/* ----------------------------------------------------- */


#if 0	/* itoc.030303.註解: RFC 822 的 DATE 欄位；RFC 1123 將 year 改成 4-DIGIT */

date-time := [ wday "," ] date time ; dd mm yy, hh:mm:ss zzz 
wday      :=  "Mon" / "Tue" / "Wed" / "Thu" / "Fri" / "Sat" / "Sun" 
date      :=  1*2DIGIT month 4DIGIT ; mday month year
month     :=  "Jan" / "Feb" / "Mar" / "Apr" / "May" / "Jun" / "Jul" / "Aug" / "Sep" / "Oct" / "Nov" / "Dec" 
time      :=  hour zone ; ANSI and Military 
hour      :=  2DIGIT ":" 2DIGIT [":" 2DIGIT] ; 00:00:00 - 23:59:59 
zone      :=  "UT" / "GMT" / "EST" / "EDT" / "CST" / "CDT" / "MST" / "MDT" / "PST" / "PDT" / 1ALPHA / ( ("+" / "-") 4DIGIT )

#endif

static time4_t datevalue;

static void
parse_date()		/* 把符合 "dd mmm yyyy hh:mm:ss" 的格式，轉成 time_t */
{
  static char months[12][4] = {"jan", "feb", "mar", "apr", "may", "jun", "jul", "aug", "sep", "oct", "nov", "dec"};
  int i;
  char *ptr, *str, buf[80];
  struct tm ptime;

  str_ncpy(buf, DATE, sizeof(buf));
  str_lower(buf, buf);			/* 通通換小寫，因為 Dec DEC dec 各種都有人用 */

  str = buf + 2;
  for (i = 0; i < 12; i++)
  {
    if (ptr = strstr(str, months[i]))
      break;
  }

  if (ptr)
  {
    ptr[-1] = ptr[3] = ptr[8] = ptr[11] = ptr[14] = ptr[17] = '\0';

    ptime.tm_sec = atoi(ptr + 15);
    ptime.tm_min = atoi(ptr + 12);
    ptime.tm_hour = atoi(ptr + 9);
    ptime.tm_mday = (ptr == buf + 2 || ptr == buf + 7) ? atoi(ptr - 2) : atoi(ptr - 3);	/* RFC 822 允許 mday 是 1- 或 2- DIGIT */
    ptime.tm_mon = i;
    ptime.tm_year = atoi(ptr + 4) - 1900;
    ptime.tm_isdst = 0;
#ifndef CYGWIN
    ptime.tm_zone = "GMT";
    ptime.tm_gmtoff = 0;
#endif

    datevalue = mktime(&ptime);
    str = ptr + 18;
    if (ptr = strchr(str, '+'))
    {
      /* 如果有 +0100 (MET) 等註明時區，先調回 GMT 時區 */
      datevalue -= ((ptr[1] - '0') * 10 + (ptr[2] - '0')) * 3600 + ((ptr[3] - '0') * 10 + (ptr[4] - '0')) * 60;
    }
    else if (ptr = strchr(str, '-'))
    {
      /* 如果有 -1000 (HST) 等註明時區，先調回 GMT 時區 */
      datevalue += ((ptr[1] - '0') * 10 + (ptr[2] - '0')) * 3600 + ((ptr[3] - '0') * 10 + (ptr[4] - '0')) * 60;
    }
    datevalue += 28800;		/* 台灣所在的 CST 時區比 GMT 快八小時 */
  }
  else
  {
    /* 如果分析失敗，那麼拿現在時間來當發文時間 */
    time4(&datevalue);
    /* bbslog("<rec_article> :Warn: parse_date 錯誤：%s\n", DATE); */
  }
}


/* ----------------------------------------------------- */
/* process post write					 */
/* ----------------------------------------------------- */


static void
update_btime(brdname)
  char *brdname;
{
  BRD *brdp, *bend;

  brdp = bshm->bcache;
  bend = brdp + bshm->number;
  do
  {
    if (!strcmp(brdname, brdp->brdname))
    {
      brdp->btime = -1;
      break;
    }
  } while (++brdp < bend);
}


/* smiler.080830 : 看門狗 */
static int
IS_BRD_DOG_FOOD(fpath, board)
  char *fpath;
  char *board;
{
  char fpath_filter[64];
  char filter[73];

  FILE *fp;
  brd_fpath(fpath_filter, board, FN_BBSDOG);

  if (!(fp = fopen(fpath_filter, "r")))
    return 0; 

  while (fgets(filter, 70, fp))
  {
    if (filter[0]=='\0' || filter[0]=='\n')
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
  char fpath_filter[64];
  char filter[73];

  FILE *fp;
  sprintf(fpath_filter, BBSHOME"/"FN_ETC_BBSDOG);

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


static void
copy_post_to_deletelog(hdr, fpath)
  HDR *hdr;
  char *fpath;
{
  char folder[64];
  HDR post;

  brd_fpath(folder, BN_DELLOG, FN_DIR);
  hdr_stamp(folder, HDR_COPY | 'A', &post, fpath);
  memcpy(post.owner, hdr->owner, TTLEN + 140);
  rec_bot(folder, &post, sizeof(HDR));
}


static int currchrono;

static int
reccmpchrono(hdr)
  HDR *hdr;
{
  return hdr->chrono == currchrono;
}


/* smiler.070916 */
static void
bbspost_topic_add(board, addr, nick ,board_from)
  char *board, *addr, *nick, *board_from;
{
  FILE *fp;
  HDR hdr;
  int cc;
  char *str;
  char folder[64], fpath[64];
  short posted = 0;	/* HBrian.080801: 文章是否有寫入(下面那個大if) */

  char fpath_log[64];
  char content_log[256];

  usint mybattr;
  BRD *brdp, *bend;

  /* 寫入文章內容 */
  brd_fpath(folder, board, FN_DIR);

#if 1		/* 看板已有相同文章就不重複 po 文 */
  int fd, fsize, pos;
  struct stat st;

  cc = 0;
  if ((fd = open(folder, O_RDONLY)) >= 0)
  {
    if (!fstat(fd, &st) && (fsize = st.st_size) >= sizeof(HDR))
    {
      while ((fsize -= sizeof(HDR)) >= 0)
      {
	lseek(fd, fsize, SEEK_SET);
	read(fd, &hdr, sizeof(HDR));
	if (!strncmp(SUBJECT, hdr.title, 10) && !strncmp(FROM, hdr.owner, 10))
	{
	  cc = -1;
	  pos = (fsize / sizeof(HDR)) - 1;
	  break;
	}
	if (++cc > 10)
	  break;
      }
    }
    close(fd);
  }

  if (cc < 0)	/* found it */
  {
    hdr_fpath(fpath, folder, &hdr);
    sprintf(content_log,
      "\033[1;30m== 系統記錄\033[m：\033[1;30m 系統自動轉載自 %s 看板%s\033[m\n",
      board_from, strcmp(board_from, "nthu") ? "" : " , 原看板文章自動刪除");
    f_cat(fpath, content_log);

#if 1
    if (!(hdr.xmode & POST_SCORE))
    {
      hdr.score = 0;
      hdr.xmode |= POST_SCORE;
      currchrono = hdr.chrono;
      rec_put(folder, &hdr, sizeof(HDR), pos, reccmpchrono);
    }
#endif
    return;
  }
#endif

  brdp = bshm->bcache;
  bend = brdp + bshm->number;
  do
  {
    if (!strcmp(board, brdp->brdname))
    {
      mybattr = brdp->battr;
      break;
    }
  } while (++brdp < bend);

  if (fp = fdopen(hdr_stamp(folder, 'A', &hdr, fpath), "w"))
  {
    posted = 1;
    fprintf(fp, "發信人: %.50s 看板: %s\n", FROM, board);
    fprintf(fp, "標  題: %.70s\n", SUBJECT);
    if (SITE)
      fprintf(fp, "發信站: %.27s (%.40s)\n\n", SITE, DATE);
    else
      fprintf(fp, "發信站: %.40s\n\n", DATE);

    /* chuan: header 跟 body 要空行隔開 */
    fprintf(fp, "\033[1;30m> ***  系統自動轉載自 %s 看板%s  *** <\033[m\n\n",
      board_from, strcmp(board_from, "nthu") ? "" : " , 原看板文章自動刪除");

    /* fprintf(fp, "%s", BODY); */
    for (str = BODY; cc = *str; str++)
    {
      if (cc == '.')
      {
	/* for line beginning with a period, collapse the doubled period to a single one. */
	if (str >= BODY + 2 && str[-1] == '.' && str[-2] == '\n')
	  continue;
      }
      fputc(cc, fp);
    }

    fclose(fp);
  }

  /* 造 HDR */

  hdr.xmode = POST_INCOME;
  /* Thor.980825: 防止字串太長蓋過頭 */
  str_ncpy(hdr.owner, addr, sizeof(hdr.owner));
  str_ncpy(hdr.nick, nick, sizeof(hdr.nick));
  str_stamp(hdr.date, &datevalue);	/* 依 DATE: 欄位的日期，與 hdr.chrono 不同步 */
  str_ncpy(hdr.title, SUBJECT, sizeof(hdr.title));

  if ((mybattr & BRD_BBS_DOG) && IS_BBS_DOG_FOOD(fpath))	/* smiler.080910: 讓使用者決定是否加入BBS DOG 計畫 */
  {
    brd_fpath(fpath_log, board, FN_BBSDOG_LOG);
    sprintf(content_log, "%s BBS看門狗計畫: 文章轉送至Deletelog板\n作者: %s\n標題: %s\n\n", Now(), hdr.owner, hdr.title);
    f_cat(fpath_log, content_log);

    sprintf(content_log, "%s BBS看門狗計畫: 文章轉送至Deletelog板\n作者: %s\n看板: %s\n標題: %s\n字串: %s\n\n", Now(), hdr.owner, board, hdr.title, bbs_dog_str);
    f_cat(FN_ETC_BBSDOG_LOG, content_log);

    copy_post_to_deletelog(&hdr, fpath);
    unlink(fpath);
    update_btime(BN_DELLOG);

    HISadd(MSGID, BN_DELLOG, hdr.xname);

    /* HBrian.080801 : 紀錄文章 */
    bbslog("bbspost_add: posted:%d afn:%s brd:Deletelog MSGID:%s SUBJ:%s\n",
      posted, hdr.xname, MSGID, SUBJECT);
  }
  else if (IS_BRD_DOG_FOOD(fpath, board))
  {
    brd_fpath(fpath_log, board, FN_BBSDOG_LOG);
    sprintf(content_log, "%s 文章內容限制: 文章轉送至Deletelog板\n作者: %s\n標題: %s\n\n", Now(), hdr.owner, hdr.title);
    f_cat(fpath_log, content_log);

    sprintf(content_log, "%s 文章內容限制: 文章轉送至Deletelog板\n作者: %s\n看板: %s\n標題: %s\n字串: %s\n\n", Now(), hdr.owner, board, hdr.title, bbs_dog_str);
    f_cat(FN_ETC_BBSDOG_LOG, content_log);

    copy_post_to_deletelog(&hdr, fpath);
    unlink(fpath);
    update_btime(BN_DELLOG);

    HISadd(MSGID, BN_DELLOG, hdr.xname);

    /* HBrian.080801 : 紀錄文章 */
    bbslog("bbspost_add: posted:%d afn:%s brd:Deletelog MSGID:%s SUBJ:%s\n",
      posted, hdr.xname, MSGID, SUBJECT);
  }
  else
  {
    rec_bot(folder, &hdr, sizeof(HDR));

    update_btime(board);

    HISadd(MSGID, board, hdr.xname);

    /* HBrian.080801 : 紀錄文章 */
    bbslog("topic_add: posted:%d afn:%s brd:%s MSGID:%s SUBJ:%s\n",
      posted, hdr.xname, board, MSGID, SUBJECT);
  }
}


/* smiler.080101: 確定是否為title MI5 的轉信文 */
int
IS_MI5(s)
  char *s;
{
  char * find_M;
  int  FIND_MI5;
  int i;

  FIND_MI5 = 0;
  if ((strchr(s, 'M') || strchr(s, 'm')) && (strchr(s, 'I') || strchr(s, 'i')) && strchr(s, '5'))
  {
    find_M=strchr(s,'M');
    //find_m=strchr(s,'m');
    for (i = 0; i <= 1; i++)
    {
      if (find_M)
      {
	while (1)
	{
	  find_M++;
	  if ((*find_M == 'I') || (*find_M == 'i'))
	  {
	    while (1)
	    {
	      find_M++;
	      if (*find_M == '5')
		FIND_MI5 = 1;
	      if (((*find_M <= 'Z') && (*find_M >= 'A')) || ((*find_M <= 'z') && (*find_M >= 'a')))
		break;
	      else if (*find_M=='\0')
		break;
	    }
	  }
	  else if (((*find_M <= 'Z') && (*find_M >= 'A')) || ((*find_M <= 'z') && (*find_M >= 'a')))
	    break;
	  else if (*find_M == '\0')
	    break;
	}
      }
      find_M = strchr(s,'m');
    }
  }
  return FIND_MI5;
}


/* smiler.080203: 各板自訂擋信機制 */
static int
belong_spam(filelist, addr, nick)
  char *filelist, *addr, *nick;
{
  FILE *fp;
  char buf[80];
  char user[80], *host;
  char *type, *spam;
  int rc, checknum;

  rc = 0;
  if (fp = fopen(filelist, "r"))
  {
    str_ncpy(user, addr, sizeof(user));	/* 防止字串太長蓋過頭 */
    if (host = (char *) strchr(user, '@'))
    {
      *host = '\0';
      host++;
    }

    checknum = 0;
    while (fgets(buf, 80, fp))
    {
      if (buf[0] == '#')	/* 加 # 是註解 */
	continue;

      if (++checknum >= 100)	/* 限 100 條 rule */
	break;

      type = (char *) strtok(buf, " \t\r\n");
      if (type && *type)
      {
	spam = (char *) strtok(NULL, " \t\r\n");
	if (spam && *spam)
	{
	  /* nick、host 可以是 NULL，故要檢查 */
	  str_lowest(spam, spam);
	  if ((!strcmp(type, "user") && str_sub(user, spam)) ||
	    (!strcmp(type, "host") && host && str_sub(host, spam)) ||
	    (!strcmp(type, "nick") && nick && str_sub(nick, spam)) ||
	    (!strcmp(type, "subj") && str_sub(SUBJECT, spam)) ||
	    (!strcmp(type, "site") && str_sub(SITE, spam)) ||
	    (!strcmp(type, "from") && str_sub(FROM, spam)) ||
	    (!strcmp(type, "msgi") && str_str(MSGID, spam)))
	  {
	    rc = 1;
	    break;
	  }
	}
      }
    }
    fclose(fp);
  }
  return rc;
}


static void
bbspost_add(board, addr, nick)
  char *board, *addr, *nick;
{
  char folder[64], fpath[64];
  HDR hdr;
  FILE *fp;
  short posted = 0;	/* HBrian.080801 : 紀錄文章是否有被此func發出 */

  char fpath_log[64];
  char content_log[256];

  usint mybattr;
  BRD *brdp, *bend;
  brdp = bshm->bcache;
  bend = brdp + bshm->number;

  /* smiler.080203: 各板自訂擋信機制 */
  brd_fpath(fpath, board, "spam");	/* 每個板自己的 spam */
  if (belong_spam(fpath, addr, nick))
  {
    board = "deleted";		// 把有濾掉的信送去 deleted 板
  }

  /* 寫入文章內容 */

  brd_fpath(folder, board, FN_DIR);

  /* smiler.070916 */
  if (strstr(SUBJECT, "賣") || strstr(SUBJECT, "售") || strstr(SUBJECT, "出清"))
  {
    //strcpy(board2,"forsale");	// smiler.080820: 依站務要求改轉文至 nthu.forsale
				// smiler.080705:依站務要求改轉錄至 forsale
    //if( (!strstr(board,"P_")) && (!strstr(board,"R_")) && 
    //	  (!strstr(board,"LAB_")) && (!strstr(board,"G_")) && (strcmp(board, "nthu.forsale")) )
    /* smiler.080820: 依站務要求僅 nctu nthu 可轉文至 nthu.forsale */

    if (!strcmp(board, "nctu") || !strcmp(board, "nthu"))
      bbspost_topic_add("nthu.forsale", addr, nick , board);

    if (!strcmp(board, "nthu"))
      return;
  }

  if (fp = fdopen(hdr_stamp(folder, 'A', &hdr, fpath), "w"))
  {
    posted = 1;	/* HBrian.080801 */
    fprintf(fp, "發信人: %.50s 看板: %s\n", FROM, board);
    fprintf(fp, "標  題: %.70s\n", SUBJECT);
    if (SITE)
      fprintf(fp, "發信站: %.27s (%.40s)\n", SITE, DATE);
    else
      fprintf(fp, "發信站: %.40s\n", DATE);
    fprintf(fp, "轉信站: %.70s\n\n", PATH);

    fprintf(fp, "%s", BODY);	/* chuan: header 跟 body 要空行格開 */
    fclose(fp);
  }

  /* 造 HDR */
  hdr.xmode = POST_INCOME;

  /* Thor.980825: 防止字串太長蓋過頭 */
  str_ncpy(hdr.owner, addr, sizeof(hdr.owner));
  str_ncpy(hdr.nick, nick, sizeof(hdr.nick));
  str_stamp(hdr.date, &datevalue);	/* 依 DATE: 欄位的日期，與 hdr.chrono 不同步 */
  str_ncpy(hdr.title, SUBJECT, sizeof(hdr.title));

  do
  {
    if (!strcmp(board, brdp->brdname))
    {
      mybattr = brdp->battr;
      break;
    }
  } while (++brdp < bend);

  if ((mybattr & BRD_BBS_DOG) && IS_BBS_DOG_FOOD(fpath)) /* smiler.080910: 讓使用者決定是否加入BBS DOG 計畫 */
  {
    brd_fpath(fpath_log, board, FN_BBSDOG_LOG);
    sprintf(content_log, "%s BBS看門狗計畫: 文章轉送至Deletelog板\n作者: %s\n標題: %s\n\n", Now(), hdr.owner, hdr.title);
    f_cat(fpath_log, content_log);

    sprintf(content_log, "%s BBS看門狗計畫: 文章轉送至Deletelog板\n作者: %s\n看板: %s\n標題: %s\n字串: %s\n\n", Now(), hdr.owner, board, hdr.title, bbs_dog_str);
    f_cat(FN_ETC_BBSDOG_LOG, content_log);

    copy_post_to_deletelog(&hdr, fpath);
    unlink(fpath);
    update_btime(BN_DELLOG);

    HISadd(MSGID, BN_DELLOG, hdr.xname);

    /* HBrian.080801 : 紀錄文章 */
    bbslog("bbspost_add: posted:%d afn:%s brd:Deletelog MSGID:%s SUBJ:%s\n",
    posted, hdr.xname, MSGID, SUBJECT);
  }
  else if (IS_BRD_DOG_FOOD(fpath, board))
  {
    brd_fpath(fpath_log, board, FN_BBSDOG_LOG);
    sprintf(content_log, "%s 文章內容限制: 文章轉送至Deletelog板\n作者: %s\n標題: %s\n\n", Now(), hdr.owner, hdr.title);
    f_cat(fpath_log, content_log);

    sprintf(content_log, "%s 文章內容限制: 文章轉送至Deletelog板\n作者: %s\n看板: %s\n標題: %s\n字串: %s\n\n", Now(), hdr.owner, board, hdr.title, bbs_dog_str);
    f_cat(FN_ETC_BBSDOG_LOG, content_log);

    copy_post_to_deletelog(&hdr, fpath);
    unlink(fpath);
    update_btime(BN_DELLOG);

    HISadd(MSGID, BN_DELLOG, hdr.xname);

    /* HBrian.080801 : 紀錄文章 */
    bbslog("bbspost_add: posted:%d afn:%s brd:Deletelog MSGID:%s SUBJ:%s\n",
    posted, hdr.xname, MSGID, SUBJECT);
  }
  else
  {
    rec_bot(folder, &hdr, sizeof(HDR));
    update_btime(board);

    HISadd(MSGID, board, hdr.xname);

    /* HBrian.080801 : 紀錄文章 */
    bbslog("bbspost_add: posted:%d afn:%s brd:%s MSGID:%s SUBJ:%s\n",
    posted, hdr.xname, board, MSGID, SUBJECT);
  }
}


/* ----------------------------------------------------- */
/* process cancel write					 */
/* ----------------------------------------------------- */


#ifdef _KEEP_CANCEL_
static inline void
move_post(hdr, board, filename)
  HDR *hdr;
  char *board, *filename;
{
  HDR post;
  char folder[64];

  brd_fpath(folder, board, FN_DIR);
  hdr_stamp(folder, HDR_LINK | 'A', &post, filename);
  unlink(filename);

  /* 直接複製 trailing data */

  memcpy(post.owner, hdr->owner, TTLEN + 140);

  sprintf(post.title, "[cancel] %-60.60s", FROM);

  rec_bot(folder, &post, sizeof(HDR));
}
#endif


static void
bbspost_cancel(board, chrono, fpath)
  char *board;
  time_t chrono;
  char *fpath;
{
  HDR hdr;
  struct stat st;
  long size;
  int fd, ent;
  char folder[64];
  off_t off, len;

  /* XLOG("cancel [%s] %d\n", board, time); */

  brd_fpath(folder, board, FN_DIR);
  if ((fd = open(folder, O_RDWR)) == -1)
    return;

  /* flock(fd, LOCK_EX); */
  /* Thor.981205: 用 fcntl 取代flock, POSIX標準用法 */
  f_exlock(fd);

  fstat(fd, &st);
  size = sizeof(HDR);
  ent = ((long) st.st_size) / size;

  /* itoc.030307.註解: 去 .DIR 中藉由比對 chrono 找出是哪一篇 */

  while (1)
  {
    /* itoc.030307.註解: 每 16 篇為一個 block */
    ent -= 16;
    if (ent <= 0)
      break;

    lseek(fd, size * ent, SEEK_SET);
    if (read(fd, &hdr, size) != size)
      break;

    if (hdr.chrono <= chrono)	/* 落在這個 block 裡 */
    {
      do
      {
	if (hdr.chrono == chrono)
	{
	  /* Thor.981014: mark 的文章不被 cancel */
	  if (hdr.xmode & POST_MARKED)
	    break;

#ifdef _KEEP_CANCEL_
	  /* itoc.030613: 保留被 cancel 的文章於 [deleted] */
	  move_post(&hdr, BN_DELETED, fpath);
#else
	  unlink(fpath);
#endif

	  update_btime(board);

	  /* itoc.030307: 被 cancel 的文章不保留 header */

	  off = lseek(fd, 0, SEEK_CUR);
	  len = st.st_size - off;

	  board = (char *) malloc(len);
	  read(fd, board, len);

	  lseek(fd, off - size, SEEK_SET);
	  write(fd, board, len);
	  ftruncate(fd, st.st_size - size);

	  free(board);
	  break;
	}

	if (hdr.chrono > chrono)
	  break;
      } while (read(fd, &hdr, size) == size);

      break;
    }
  }

  /* flock(fd, LOCK_UN); */
  /* Thor.981205: 用 fcntl 取代flock, POSIX標準用法 */
  f_unlock(fd);

  close(fd);
  return;
}


int			/* 0:cancel success  -1:cancel fail */
cancel_article(msgid)
  char *msgid;
{
  int fd;
  char fpath[64], cancelfrom[128], buffer[128];
  char board[BNLEN + 1], xname[9];

  /* XLOG("cancel %s <%s>\n", FROM, msgid); */

  if (!HISfetch(msgid, board, xname))
    return -1;

  str_from(FROM, cancelfrom, buffer);

  /* XLOG("cancel %s (%s)\n", cancelfrom, buffer); */

  sprintf(fpath, "brd/%s/%c/%s", board, xname[7], xname);	/* 去找出那篇文章 */

  /* XLOG("cancel fpath (%s)\n", fpath); */

  if ((fd = open(fpath, O_RDONLY)) >= 0)
  {
    int len;

    len = read(fd, buffer, sizeof(buffer));
    close(fd);

    /* Thor.981221.註解: 外來文章才能被 cancel */
    if ((len > 10) && !memcmp(buffer, "發信人: ", 8))
    {
      char *xfrom, *str;

      xfrom = buffer + 8;
      if (str = strchr(xfrom, ' '))
      {
	*str = '\0';

#ifdef _NoCeM_
	/* gslin.000607: ncm_issuer 可以砍別站發的信 */
	if (strcmp(xfrom, cancelfrom) && !search_issuer(FROM, NULL))
#else
	if (strcmp(xfrom, cancelfrom))
#endif
	{
	  /* itoc.030107.註解: 若 cancelfrom 和本地文章 header 記錄的 xfrom 不同，就是 fake cancel */
	  bbslog("<rec_article> :Warn: 無效的 cancel：%s, sender: %s, path: %s\n", xfrom, FROM, PATH);
	  return -1;
	}

	bbspost_cancel(board, chrono32(xname), fpath);
      }
    }
  }

  return 0;
}


/* ----------------------------------------------------- */
/* check spam rule					 */
/* ----------------------------------------------------- */


static int		/* 1: 符合擋信規則 */
is_spam(board, addr, nick)
  char *board, *addr, *nick;
{
  spamrule_t *spam;
  int i, xmode;
  char *compare, *detail;

  for (i = 0; i < SPAMCOUNT; i++)
  {
    spam = SPAMRULE + i;

    compare = spam->path;
    if (*compare && strcmp(compare, NODENAME))
      continue;

    compare = spam->board;
    if (*compare && strcmp(compare, board))
      continue;

    xmode = spam->xmode;
    detail = spam->detail;

    if (xmode & INN_SPAMADDR)
      compare = addr;
    else if (xmode & INN_SPAMNICK)
      compare = nick;
    else if (xmode & INN_SPAMSUBJECT)
      compare = SUBJECT;
    else if (xmode & INN_SPAMPATH)
      compare = PATH;
    else if (xmode & INN_SPAMMSGID)
      compare = MSGID;
    else if (xmode & INN_SPAMBODY)
      compare = BODY;
    else if (xmode & INN_SPAMSITE && SITE)		/* SITE 可以是 NULL */
      compare = SITE;
    else if (xmode & INN_SPAMPOSTHOST && POSTHOST)	/* POSTHOST 可以是 NULL */
      compare = POSTHOST;
    else
      continue;

    if (str_sub(compare, detail))
      return 1;
  }
  return 0;
}


/* ----------------------------------------------------- */
/* process receive article				 */
/* ----------------------------------------------------- */


#ifndef _NoCeM_
static 
#endif
newsfeeds_t *
search_newsfeeds_bygroup(newsgroup)
  char *newsgroup;
{
  newsfeeds_t nf, *find;

  str_ncpy(nf.newsgroup, newsgroup, sizeof(nf.newsgroup));
  find = bsearch(&nf, NEWSFEEDS_G, NFCOUNT, sizeof(newsfeeds_t), nf_bygroupcmp);
  if (find && !(find->xmode & INN_NOINCOME))
    return find;
  return NULL;
}


int				/* 0:success  -1:fail */
receive_article()
{
  newsfeeds_t *nf;
  char myaddr[128], mynick[128], mysubject[128], myfrom[128], mydate[80];
  char poolx[256];
  char mypath[128], *pathptr;
  char *group;
  int firstboard = 1;

  /* try to split newsgroups into separate group */
  for (group = strtok(GROUP, ","); group; group = strtok(NULL, ","))
  {
    if (!(nf = search_newsfeeds_bygroup(group)))
      continue;

    if (firstboard)	/* opus: 第一個板才需要處理 */
    {
      /* Thor.980825: gc patch: lib/str_decode 只能接受 decode 完 strlen < 256 */ 

      str_ncpy(poolx, SUBJECT, 255);
      str_decode(poolx);
      str_ansi(mysubject, poolx, 70);	/* 70 是 bbspost_add() 標題所需的長度 */
      SUBJECT = mysubject;

      str_ncpy(poolx, FROM, 255);
      str_decode(poolx);
      str_ansi(myfrom, poolx, 128);	/* 雖然 bbspost_add() 發信人所需的長度只需要 50，但是 str_from() 需要長一些 */
      FROM = myfrom;

      str_ncpy(poolx, PATH, 255);
      str_decode(poolx);
      str_ansi(mypath, poolx, 128);
      /* itoc.030115.註解: PATH 如果有 .edu.tw 就截掉 */
      for (pathptr = mypath; pathptr = strstr(pathptr, ".edu.tw");)
        strcpy(pathptr, pathptr + 7);
      mypath[70] = '\0';
      PATH = mypath;

      /* itoc.030218.註解: 處理「發信站」中的時間 */
      parse_date();
      strcpy(mydate, (char *) Btime(&datevalue));
      DATE = mydate;

      if (*nf->charset == 'g')
      {
	gb2b5(BODY);
	gb2b5(FROM);
	gb2b5(SUBJECT);
	if (SITE)
	  gb2b5(SITE);
      }

      strcpy(poolx, FROM);
      str_from(poolx, myaddr, mynick);

      if (is_spam(nf->board, myaddr, mynick))
      {
#ifdef _KEEP_CANCEL_
	bbspost_add(BN_DELETED, myaddr, mynick);
#endif
	break;
      }

      firstboard = 0;
    }
    if (!IS_MI5(SUBJECT))
      bbspost_add(nf->board, myaddr, mynick);
  }		/* for board1,board2,... */

  return 0;
}
