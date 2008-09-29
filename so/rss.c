/*-------------------------------------------------------*/
/* rss.c	( NTHU CS MapleBBS Ver 3.20 )		 */
/*-------------------------------------------------------*/
/* target : RSS						 */
/* create : 08/08/16					 */
/* update : // 						 */
/* author : smiler.bbs@bbs.cs.nthu.edu.tw		 */
/*-------------------------------------------------------*/

#include "bbs.h"

extern XZ xz[];
extern char xo_pool[];


/* ----------------------------------------------------- */
/* RSS 功能表						 */
/* ----------------------------------------------------- */


static int rss_body();
static int rss_head();
static int rss_add();
static rss_currchrono;

static int IS_URL(url)
  char *url;
{
  if (strchr(url, ';') || strchr(url, '"') || strchr(url, '|') || strchr(url, '&') || strchr(url, ' ')) /* security reason */
    return 0;

  return 1;
}


static int
rss_cmpchrono(rss)
  RSS *rss;
{
  return rss->chrono == rss_currchrono;
}


static int
rss_init(xo)
  XO *xo;
{
  xo_load(xo, sizeof(RSS));
  return rss_head(xo);
}


static int
rss_load(xo)
  XO *xo;
{
  xo_load(xo, sizeof(RSS));
  return rss_body(xo);
}


static void
rss_item(num, rss)
  int num;
  RSS *rss;
{
  prints("%6d ", num);							//7
  prints("%c",   (rss->xmode & RSS_RESTRICT) ? ')' : ' ');		//1
  prints("%s",(rss->xmode & RSS_RESTART) ? "\033[1;33m" : "");
  if ((rss->xmode & RSS_RESTRICT) && (!(bbstate & STAT_BOARD))  &&  (strcmp(rss->owner,cuser.userid)) )
  {
    prints("%-70.70s","<<資料保密>>");
    prints("%s",(rss->xmode & RSS_RESTART) ? "\033[m" : "");
    prints("\n");
    return;
  }
  prints("\033[1;3%dm%s\033[m ", cal_day(rss->date) + 1, rss->date + 3);//6
  prints("%s",(rss->xmode & RSS_RESTART) ? "\033[1;33m" : "");
  prints("%-12.12s ",rss->bookmark);					//13
  prints("%s %s %s ", (rss->xmode & RSS_UTF8)  ? "Ｕ"    : "※",		//3*3=9
			(rss->xmode & RSS_TXT)   ? "Ｔ"    : "Ｈ",
			(rss->xmode & RSS_START) ? "◆"    : "◇");
  prints("%-42.40s",rss->url);						//42
  prints("%s\n",(rss->xmode & RSS_RESTART) ? "\033[m" : "");

  return;
}


#ifdef HAVE_LIGHTBAR
static int
rss_item_bar(xo, mode)
  XO *xo;
  int mode;
{
  RSS *rss;
  int num;

  rss = (RSS *) xo_pool + xo->pos - xo->top;
  num = xo->pos + 1;

  prints("\033[m%s",mode ? UCBAR[UCBAR_RSS] : "");
  prints("%6d ", num);							//7
  prints("%c",   (rss->xmode & RSS_RESTRICT) ? ')' : ' ');		//1
  prints("%s",(rss->xmode & RSS_RESTART) ? "\033[1;33m" : "");
  if ((rss->xmode & RSS_RESTRICT) && (!(bbstate & STAT_BOARD))  &&  (strcmp(rss->owner,cuser.userid)) )
  {
    prints("%-70.70s","<<資料保密>>");
    prints("%s",((rss->xmode & RSS_RESTART) || mode) ? "\033[m" : "");
    return 0;
  }
  prints("\033[1;3%dm%s\033[m", cal_day(rss->date) + 1, rss->date + 3);	//5
  prints("%s ",mode ? UCBAR[UCBAR_RSS] : "");				//1
  prints("%s",(rss->xmode & RSS_RESTART) ? "\033[1;33m" : "");
  prints("%-12.12s ",rss->bookmark);					//13
  prints("%s %s %s ", (rss->xmode & RSS_UTF8)  ? "Ｕ"    : "※",		//3*3=9
			    (rss->xmode & RSS_TXT)   ? "Ｔ"    : "Ｈ",
			    (rss->xmode & RSS_START) ? "◆"    : "◇");
  prints("%-42.40s",rss->url);						//42
  prints("%s", ((rss->xmode & RSS_RESTART) || mode) ? "\033[m" : "");

  return 0;
}
#endif


static int
rss_body(xo)
  XO *xo;
{
  RSS *rss;
  int num, max, tail;

  max = xo->max;
  if (max <= 0)
  {
    if (bbstate & STAT_BOARD)
    {
      if (vans("要新增資料嗎(Y/N)？[N] ") == 'y')
	return rss_add(xo);
    }
    else
    {
      vmsg("本看板尚無RSS");
    }
    return XO_QUIT;
  }

  rss = (RSS *) xo_pool;
  num = xo->top;
  tail = num + XO_TALL;
  if (max > tail)
    max = tail;

  move(3, 0);
  do
  {
    rss_item(++num, rss++);
  } while (num < max);
  clrtobot();
  /* return XO_NONE; */
  return XO_FOOT;	/* itoc.010403: 把 b_lines 填上 feeter */
}


static int
rss_add(xo)
  XO *xo;
{
  RSS rss;
  char buf[2];
  int ans;
  int i;

  if (!(bbstate & STAT_BOARD))
    return XO_NONE;

  move(3,0);
  clrtobot();

  while (1)
  {
    move(i = 4, 0);
    clrtobot();

    rss.xmode = 0;

    if(!vget(i, 0, "請輸入RSS標籤名稱：", rss.bookmark, IDLEN + 1, DOECHO))
    {
      rss.bookmark[0]= '\0';
    }

    i += 2;
    prints("\n請輸入網址：");
    i++;
    if (!vget(i, 0, "", rss.url, 73, DOECHO))
    {
      return XO_BODY;
    }
    if ( strlen(rss.url) < 7  || strncmp(rss.url, "http://", 7))
    {
      vmsg("網址格式需為 http:// ，請重新輸入 !!");
      continue;
    }
    if (!IS_URL(rss.url))
    {
      vmsg("網址格式含不正確字符 ; \" | & space ，請重新輸入 !!");
      continue;
    }

    i += 2;
    if(!vget(i, 0, "是UTF8編碼或其他編碼： U)UTF8  Q)其他  [U] ", buf, 1 + 1, DOECHO))
    {
      strcpy(buf,"u");
    }

    if ((buf[0] == 'Q') || (buf[0]=='q'))
    {
      rss.xmode &= (~RSS_UTF8);
      i += 2;
      if (!vget(i, 0, "輸入編碼名稱：", rss.code_type, 64, DOECHO))
      {
	vmsg("未輸入編碼名稱 !!");
	return XO_INIT;
      }

    }
    else
      rss.xmode |= RSS_UTF8;

    i += 2;
    if (vget(i, 0, "是否要將html格式轉譯為txt格式：Y)要 N)否 [Y] ", buf, 2, LCECHO) == 'n')
      rss.xmode &= (~RSS_TXT);
    else
      rss.xmode |= RSS_TXT;

    i += 2;
    prints("\n請輸入相關說明：");
    i++;
    if (!vget(i, 0, "", rss.info, 73, DOECHO))
    {
      strcpy(rss.info," ");
    }

    i += 2;
    if (vget(i, 0, "是否隱藏此RSS選項：Y)是 N)否 [N] ", buf, 2, LCECHO) == 'y')
      rss.xmode |= RSS_RESTRICT;
    else
      rss.xmode &= (~RSS_RESTRICT);

    i += 2;
    if (vget(i, 0, "是否立即啟用RSS：Y)是 N)否 [Y] ", buf, 2, DOECHO) == 'n')
      rss.xmode &= (~RSS_START);
    else
      rss.xmode |= RSS_START;

    ans = vans("選擇：S)儲存 E)繼續 Q)離開 [Q] ");

    if (ans != 's' && ans != 'e')
      return XO_BODY;
    else if(ans == 's')
      break;
  }

  time(&(rss.chrono));
  struct tm *ptime;

  ptime = localtime(&(rss.chrono));
  sprintf(rss.date,"%02d/%02d/%02d",ptime->tm_year % 100, ptime->tm_mon + 1, ptime->tm_mday);

  sprintf(rss.owner,cuser.userid);

  rss.xid = 0;				/* smiler.080819: 保留，目前暫時未用到 */

  sprintf(rss.modified,"start");	/* smiler.080823: 初始rss.modified */

  ans = vans("存放位置 A)新增 I)插入 N)下一個 Q)離開 [A] ");
  switch (ans)
  {
  case 'q':
    break;

  case 'i':
  case 'n':
    rec_ins(xo->dir, &rss, sizeof(RSS), xo->pos + (ans == 'n'), 1);
    break;

  default:
    rec_add(xo->dir, &rss, sizeof(RSS));
    break;
  }

  return rss_init(xo);
}


static int
rss_delete(xo)
  XO *xo;
{
  RSS *rss;

  rss = (RSS *) xo_pool + (xo->pos - xo->top);

  if (!(bbstate & STAT_BOARD))
    return XO_NONE;

  if (vans(msg_del_ny) == 'y')
  {

    if (!rec_del(xo->dir, sizeof(RSS), xo->pos, NULL))
      return rss_load(xo);
  }

  return XO_FOOT;
}


static int
rss_head(xo)
  XO *xo;
{
  vs_head(currBM, currboard);
  prints(NECKER_RSS, d_cols, "");
  return rss_body(xo);
}


static int
rss_edit(xo)
  XO *xo;
{
  RSS *frss, mrss;
  int pos,cur,ans,i;
  char buf[2];

  if (!(bbstate & STAT_BOARD))
    return XO_NONE;

  pos = xo->pos;
  cur = pos - xo->top;

  frss = (RSS *) xo_pool + xo->pos - xo->top;
  memcpy(&mrss, frss, sizeof(RSS));

  move(3,0);
  clrtobot();

  while (1)
  {
    move(i = 4, 0);
    clrtobot();

    if (!vget(i, 0, "請輸入RSS標籤名稱：", mrss.bookmark, IDLEN + 1, GCARRY))
    {
      strcpy(mrss.bookmark," ");
    }

    i += 2;
    prints("\n請輸入網址：");
    i++;
    if (!vget(i, 0, "", mrss.url, 73, GCARRY))
    {
      return XO_BODY;
    }
    if ( strlen(mrss.url) < 7  || strncmp(mrss.url, "http://", 7))
    {
      vmsg("網址格式需為 http:// ，請重新輸入 !!");
      continue;
    }

    if(!IS_URL(mrss.url))
    {
      char warm_message[64];
      sprintf(warm_message,"網址格式含不正確字符 ; \" | & space ，請重新輸入 !!");
      vmsg(warm_message);
      continue;
    }

    sprintf(mrss.modified,"start");		/* smiler.080823: 初始rss.modified */

    i += 2;
    if (vget(i, 0, "是UTF8編碼或其他編碼： U)UTF8  Q)其他  [U] ", buf, 2, LCECHO) == 'q')
    {
      mrss.xmode &= (~RSS_UTF8);
      i += 2;
      if (!vget(i, 0, "輸入編碼名稱：", mrss.code_type, 64, DOECHO))
      {
	vmsg("未輸入編碼名稱 !!");
	return XO_INIT;
      }

    }
    else
      mrss.xmode |= RSS_UTF8;

    i += 2;
    if (vget(i, 0, "是否要將html格式轉譯為txt格式：Y)要 N)否 [Y] ", buf, 2, LCECHO) == 'n')
      mrss.xmode &= (~RSS_TXT);
    else
      mrss.xmode |= RSS_TXT;

    i += 2;
    prints("\n請輸入相關說明：");
    i++;
    if(!vget(i, 0, "", mrss.info, 73, GCARRY))
    {
      strcpy(mrss.info," ");
    }

    i += 2;
    if (vget(i, 0, "是否隱藏此RSS選項：Y)是 N)否 [N] ", buf, 2, LCECHO) == 'y')
      mrss.xmode |= RSS_RESTRICT;
    else
      mrss.xmode &= (~RSS_RESTRICT);

    i += 2;
    if (vget(i, 0, "是否立即啟用RSS：Y)是 N)否 [Y] ", buf, 2, LCECHO) == 'n')
      mrss.xmode &= (~RSS_START);
    else
      mrss.xmode |= RSS_START;

    ans = vans("選擇：S)儲存 E)繼續 Q)離開 [Q] ");

    if (ans != 's' && ans != 'e')
      return XO_BODY;
    else if(ans == 's')
      break;
  }

  sprintf(mrss.modified,"start");	/* smiler.080823: 將 feed->modified 清掉，重新記錄 */

  struct tm *ptime;
  ptime = localtime(&(mrss.chrono));
  sprintf(mrss.date,"%02d/%02d/%02d",ptime->tm_year % 100, ptime->tm_mon + 1, ptime->tm_mday);

  if (memcmp(frss, &mrss, sizeof(RSS)) && vans(msg_sure_ny) == 'y')
  {
    memcpy(frss, &mrss, sizeof(RSS));
    rss_currchrono = frss->chrono;
    rec_put(xo->dir, frss, sizeof(RSS), pos, rss_cmpchrono);

    move(3 + cur, 0);
    rss_item(++pos, frss);
  }
  return XO_BODY;
}


static int
rss_url(xo)
  XO *xo;
{
  RSS *frss, mrss;
  int pos,cur;

  if (!(bbstate & STAT_BOARD))
    return XO_NONE;

  pos = xo->pos;
  cur = pos - xo->top;

  frss = (RSS *) xo_pool + xo->pos - xo->top;
  memcpy(&mrss, frss, sizeof(RSS));

  move(b_lines-1, 0);
  clrtoeol();
  prints("請輸入網址：");
  char buf_url[73];
  strcpy(buf_url,mrss.url);
  if(!vget(b_lines, 0, "", mrss.url, 73, GCARRY))
    strcpy(mrss.url,buf_url);


  if ( strlen(mrss.url) < 7  || strncmp(mrss.url, "http://", 7))
  {
    strcpy(mrss.url,buf_url);
  }
  if (!IS_URL(mrss.url))
  {
    char warm_message[64];
    sprintf(warm_message,"網址格式含不正確字符 ; \" | & space ，請重新輸入 !!");
    vmsg(warm_message);
    return XO_BODY;
  }

  sprintf(mrss.modified,"start");              /* smiler.080823: 初始rss.modified */

  struct tm *ptime;
  ptime = localtime(&(mrss.chrono));
  sprintf(mrss.date,"%02d/%02d/%02d",ptime->tm_year % 100, ptime->tm_mon + 1, ptime->tm_mday);

  if (memcmp(frss, &mrss, sizeof(RSS)) && vans(msg_sure_ny) == 'y')
  {
    memcpy(frss, &mrss, sizeof(RSS));
    rss_currchrono = frss->chrono;
    rec_put(xo->dir, frss, sizeof(RSS), pos, rss_cmpchrono);

    move(3 + cur, 0);
    rss_item(++pos, frss);
  }
  return XO_BODY;
}


static int
rss_bookmark(xo)
  XO *xo;
{
  RSS *frss, mrss;
  int pos,cur;

  if (!(bbstate & STAT_BOARD))
    return XO_NONE;

  pos = xo->pos;
  cur = pos - xo->top;

  frss = (RSS *) xo_pool + xo->pos - xo->top;
  memcpy(&mrss, frss, sizeof(RSS));

  if (!vget(b_lines, 0, "請輸入RSS標籤名稱：", mrss.bookmark, IDLEN + 1, GCARRY))
    strcpy(mrss.bookmark," ");

  struct tm *ptime;
  ptime = localtime(&(mrss.chrono));
  sprintf(mrss.date,"%02d/%02d/%02d",ptime->tm_year % 100, ptime->tm_mon + 1, ptime->tm_mday);

  if (memcmp(frss, &mrss, sizeof(RSS)) && vans(msg_sure_ny) == 'y')
  {
    memcpy(frss, &mrss, sizeof(RSS));
    rss_currchrono = frss->chrono;
    rec_put(xo->dir, frss, sizeof(RSS), pos, rss_cmpchrono);

    move(3 + cur, 0);
    rss_item(++pos, frss);
  }
  return XO_FOOT;
}


static rss_move(xo)
  XO *xo;
{
  RSS *rss;
  char *dir, buf[40];
  int pos, newOrder;

  if (!(bbstate & STAT_BOARD))
    return XO_NONE;

  pos = xo->pos;
  rss = (RSS *) xo_pool + (pos - xo->top);

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
    if (!rec_del(dir, sizeof(RSS), pos, NULL))
    {
      rec_ins(dir, rss, sizeof(RSS), newOrder, 1);
      xo->pos = newOrder;
      return rss_load(xo);
    }
  }

  return XO_FOOT;
}


static int
rss_mark(xo)
  XO *xo;
{
  RSS *rss;
  int pos, cur, xmode;

  if (!(bbstate & STAT_BOARD))
    return XO_NONE;

  pos = xo->pos;
  cur = pos - xo->top;
  rss = (RSS *) xo_pool + cur;
  xmode = rss->xmode;

  rss->xmode = xmode ^ RSS_RESTRICT;
  rss_currchrono = rss->chrono;
  rec_put(xo->dir, rss, sizeof(RSS), pos, rss_cmpchrono);

  return XO_BODY;
}


static int
rss_utf8(xo)
  XO *xo;
{
  RSS *rss;
  int pos, cur, xmode;

  if (!(bbstate & STAT_BOARD))
    return XO_NONE;

  pos = xo->pos;
  cur = pos - xo->top;
  rss = (RSS *) xo_pool + cur;
  xmode = rss->xmode;

  if (rss->xmode & RSS_UTF8)
  {
    if (vans("取消UTF8編碼，改為自定編碼(Y/N)？[N]") == 'y')
    {
      if (!vget(b_lines, 0, "輸入編碼名稱：", rss->code_type, 64, DOECHO))
      {
	vmsg("未輸入編碼名稱 !!");
	return XO_BODY;
      }
      rss->xmode &= (~RSS_UTF8);
    }
    else
      return XO_BODY;
  }
  else
    rss->xmode |= RSS_UTF8;

  rss_currchrono = rss->chrono;
  rec_put(xo->dir, rss, sizeof(RSS), pos, rss_cmpchrono);

  return XO_BODY;
}


static int
rss_html_txt(xo)
  XO *xo;
{
  RSS *rss;
  int pos, cur, xmode;

  if (!(bbstate & STAT_BOARD))
    return XO_NONE;

  pos = xo->pos;
  cur = pos - xo->top;
  rss = (RSS *) xo_pool + cur;
  xmode = rss->xmode;

  rss->xmode = xmode ^ RSS_TXT;
  rss_currchrono = rss->chrono;
  rec_put(xo->dir, rss, sizeof(RSS), pos, rss_cmpchrono);

  return XO_BODY;
}


static int
rss_start_idle(xo)
  XO *xo;
{
  RSS *rss;
  int pos, cur, xmode;

  if (!(bbstate & STAT_BOARD))
    return XO_NONE;

  pos = xo->pos;
  cur = pos - xo->top;
  rss = (RSS *) xo_pool + cur;
  xmode = rss->xmode;

  rss->xmode = xmode ^ RSS_START;
  rss_currchrono = rss->chrono;
  rec_put(xo->dir, rss, sizeof(RSS), pos, rss_cmpchrono);

  return XO_BODY;
}


static int
rss_restart(xo)
  XO *xo;
{
  RSS *rss;
  int pos, cur, xmode;

  if (!(bbstate & STAT_BOARD))
    return XO_NONE;

  pos = xo->pos;
  cur = pos - xo->top;
  rss = (RSS *) xo_pool + cur;
  xmode = rss->xmode;

  rss->xmode = xmode ^ RSS_RESTART;
  rss_currchrono = rss->chrono;
  rec_put(xo->dir, rss, sizeof(RSS), pos, rss_cmpchrono);

  return XO_BODY;
}


static int
rss_browse(xo)
  XO *xo;
{
  RSS *rss;
  struct tm *ptime;

  rss = (RSS *) xo_pool + (xo->pos - xo->top);
  ptime = localtime(&(rss->chrono));

  if ((!(bbstate & STAT_BOARD)) && (rss->xmode & RSS_RESTRICT))
    return XO_NONE;

  move(3,0);
  clrtobot();

  prints("\n標籤名稱：%s\n"  ,rss->bookmark);
  prints("添加者  ：%s\n"  ,rss->owner);
  prints("原始日期：%02d/%02d/%2d %2d:%2d:%2d\n"  ,ptime->tm_year % 100, ptime->tm_mon + 1, ptime->tm_mday, ptime->tm_hour, ptime->tm_min, ptime->tm_sec);
  prints("更動日期：%s\n",rss->date);
  prints("RSS 更新: %s\n",rss->modified);
  prints("RSS 網址：\n%s\n",rss->url);
  prints("相關說明：\n%s\n",rss->info);
  prints("RSS 屬性：\n");
  prints("        %s\n",(rss->xmode & RSS_START)    ? "RSS 啟動中"         : "RSS 暫停使用");
  prints("        %s 編碼\n",(rss->xmode & RSS_UTF8 )    ? "原檔案為 UTF8"      : rss->code_type);
  prints("        %s\n",(rss->xmode & RSS_TXT  )    ? "轉為 TXT 格式"      : "保留原Html格式");
  prints("        %s\n",(rss->xmode & RSS_RESTRICT) ? "隱藏狀態"           : "公開狀態");
  prints("        %s\n",(rss->xmode & RSS_START)    ? "啟動中"             : "暫停中");
  prints("        %s\n",(rss->xmode & RSS_RESTART)  ? "啟動重送"           : "");

  vmsg(NULL);
  return XO_BODY;
}


static int
rss_help(xo)
  XO *xo;
{
  xo_help("rss");
  return XO_HEAD;
}


int rss_main()
{
  XO *xo;
  char rss_fpath[64];
  brd_fpath(rss_fpath,currboard,FN_RSS);
  xz[XZ_RSS - XO_ZONE].xo = xo = xo_new(rss_fpath);
  xo->key = XZ_RSS;
  xo->pos = 0;
  xover(XZ_RSS);
  free(xo);
  return 0;
}


KeyFunc rss_cb[] =
{
#ifdef HAVE_LIGHTBAR
  XO_ITEM, rss_item_bar,
#endif
  XO_INIT, rss_init,
  XO_LOAD, rss_load,
  XO_HEAD, rss_head,
  XO_BODY, rss_body,

  'a', rss_add,
  'd', rss_delete,
  'm', rss_move,

  'o', rss_mark,
  'u', rss_utf8,
  'n', rss_html_txt,
  's', rss_start_idle,
  'R', rss_restart,

  'r', rss_browse,
  'E', rss_edit,
  'H', rss_url,
  'T', rss_bookmark,
  Ctrl('P'), rss_add,

  'h', rss_help
};

