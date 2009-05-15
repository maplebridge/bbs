/*-------------------------------------------------------*/
/* bitlbee.c   ( NTHU CS MapleBBS Ver 3.10 )		 */
/*-------------------------------------------------------*/
/* author : ono.bbs@lexel.twbbs.org			 */
/* modify : smiler.bbs@bbs.cs.nthu.edu.tw		 */
/* target : MSN on MapleBBS				 */
/* create : 05/06/08					 */
/* update : 08/11/03					 */
/*-------------------------------------------------------*/

#include "bbs.h"


#ifdef HAVE_BITLBEE

extern XZ xz[];

static XO bit_xo;
static FILE *fw, *fr;
static char buf[512];
extern int bit_sock;


/* 預設上線最多 250 位 */
static BITUSR bit_pool[250];

void bit_abort(void);


static
bit_fgets()
{
  char *tmp;

  fgets(buf, sizeof (buf), fr);
  tmp = strstr(buf, "Error: Someone else logged in with your account");

  if (tmp)
  {
    bit_abort();
    vmsg ("連線中斷！您已在其他地方登入 msn！");
  }
}


static void
bit_item(num, pp)
  int num;
  BITUSR *pp;
{
  prints("%5d   \033[1;37m%-18.17s\033[m  \033[30;1m%-*.*s\033[m \033[1;%-18.17s\033[m \n",
    num, pp->nick, d_cols + 34, d_cols + 33, pp->addr,
    strstr(pp->status, "Online") ? "36m線上" :
    strstr(pp->status, "Away") ? "33m離開" :
    strstr(pp->status, "Busy") ? "31m忙碌" :
    strstr(pp->status, "Idle") ? "34m閒置" :
    strstr(pp->status, "Right") ? "35m馬上回來" :
    strstr(pp->status, "Phone") ? "32m電話中" :
    strstr(pp->status, "Lunch") ? "m外出吃飯" : pp->status);
}


#ifdef HAVE_LIGHTBAR
static int
bit_item_bar(xo, mode)
  XO *xo;
  int mode;	/* 1:上光棒  0:去光棒 */
{
  BITUSR *pp;

  //pp = bit_pool + xo->pos - xo->top;
  pp = bit_pool + xo->pos;

  prints("%s%5d   \033[1;37m%-18.17s\033[m%s  \033[1;%dm%-*.*s\033[m%s ",
    mode ? UCBAR[UCBAR_USR] : "", xo->pos + 1, pp->nick,
    mode ? UCBAR[UCBAR_USR] : "", mode ? 37 : 30,
    d_cols + 34, d_cols + 33, pp->addr, mode ? UCBAR[UCBAR_USR] : "");

  prints("\033[1;%-18.18s\033[m",
    strstr(pp->status, "Online") ? "36m線上" :
    strstr(pp->status, "Away") ? "33m離開" :
    strstr(pp->status, "Busy") ? "31m忙碌" :
    strstr(pp->status, "Idle") ? "34m閒置" :
    strstr(pp->status, "Right") ? "35m馬上回來" :
    strstr(pp->status, "Phone") ? "32m電話中" :
    strstr(pp->status, "Lunch") ? "37m外出吃飯" : "");

  return XO_NONE;
}
#endif


static int
bit_body(xo)
  XO *xo;
{
  BITUSR *pp;
  int num, max, tail;

  max = xo->max;
  num = xo->top;
  pp = &bit_pool[num];
  tail = num + XO_TALL;
  if (max > tail)
    max = tail;

  move(3, 0);
  do
  {
    bit_item(++num, pp++);
  } while (num < max);
  clrtobot();

  return XO_FOOT;
}


static int
bit_head(xo)
  XO *xo;
{
  clear();
  vs_head("MSN 列表", str_site);
  move(1, 0);
  prints(
    " [w]傳訊 [c]改暱稱 [^k]斷線 [a]增刪聯絡人 [d]刪除聯絡人 [l]msn紀錄 [h]說明   \n"
    "\033[30;47m 編號   代   號             信          箱%-*.*s狀  態         \033[m",
    d_cols + 21, d_cols + 21, "");

  return bit_body(xo);
}


static int
bit_set(xo)
  XO *xo;
{
  char *tmp, max = 0;
  char seps[] = " \t";

  fprintf(fw, "PRIVMSG root :blist\r\n");
  fflush(fw);

  /* NICK 剛好在第 48 個字, 前面的訊息略過 */
  do
  {
    bit_fgets();
    if (bit_sock <= 0)
      return XO_QUIT;
    tmp = strstr(buf, "Nick");
  }
  while (!tmp);

  for (;;)
  {
    bit_fgets ();
    if (bit_sock <= 0)
      return XO_QUIT;

    tmp = strstr(buf, "msn(");

    if (!tmp)
      break;
    else
    {
      tmp = strtok(buf, ":");
      tmp = strtok(NULL, seps);
      sprintf (bit_pool[max].nick, "%s", tmp);
      tmp = strtok(NULL, seps);
      sprintf (bit_pool[max].addr, "%s", tmp);
      tmp = strtok(NULL, seps);
      tmp = strtok(NULL, "\n");

      do
      {
	*tmp++;
      }
      while (*tmp == ' ');

      sprintf(bit_pool[max].status, "%s", tmp);
      max++;
    }
  };

  xo->max = max;
  if (xo->pos >= max)
    xo->pos = xo->top = 0;

  return bit_head(xo);
}


static int
bit_help(xo)
  XO *xo;
{
  xo_help("msn");
  return XO_HEAD;
}


static int
bit_write(xo)
  XO *xo;
{
  int pos;
  char hint[30], *nick, str[65], file[128];
  screenline sl[b_lines + 1];
  FILE *fp;

  pos = xo->pos;
  nick = bit_pool[pos].nick;
  sprintf (hint, "★[%s] ", nick);

  vs_save(sl);

  if (vget(b_lines - 1, 0, hint, str, 60, DOECHO) &&
    vans("確定要送出 MSN 訊息 (Y/N)？[Y] ") != 'n')
  {
    usr_fpath(file, cuser.userid, FN_MSN);
    fp = fopen(file, "a");
    fprintf(fp, "To %s (@msn)：%s\n", nick, str);
    fclose(fp);

    fprintf(fw, "PRIVMSG %s :%s\r\n", nick, str);
    fflush(fw);
  }

  vs_restore(sl);
  return bit_body(xo);
//  return XO_INIT;
}


/* smiler.080319: 回覆水球 */
void
bit_reply(nick, msg)
  char *nick;
  char *msg;
{
  char hint[30];
  char file[128];
  FILE *fp;

  sprintf(hint, "★<%s> ", nick);

  usr_fpath(file, cuser.userid, FN_MSN);
  fp = fopen(file, "a");
  fprintf(fp, "To %s (@msn)：%s\n", nick, msg);
  fclose(fp);

  fprintf(fw, "PRIVMSG %s :%s\r\n", nick, msg);
  fflush(fw);
}


static int
bit_unblock(xo)
  XO *xo;
{
  if (vans("確定要解除封鎖他 (Y/N)？[N]") == 'y')
  {
    int pos;
    char *nick;

    pos = xo->pos;
    nick = bit_pool[pos].nick;
    fprintf(fw, "PRIVMSG root :allow %s\r\n", nick);
    fflush(fw);
  }
  return XO_FOOT;
}


static int
bit_block(xo)
  XO *xo;
{
  if (vans("確定要封鎖他 (Y/N)？[N]") == 'y')
  {
    int pos;
    char *nick;

    pos = xo->pos;
    nick = bit_pool[pos].nick;
    fprintf(fw, "PRIVMSG root :block %s\r\n", nick);
    fflush(fw);
  }
  return XO_FOOT;
}


#if 0
static int
bit_save(xo)
  XO *xo;
{
  vmsg ("設定好別人暱稱後存起來，重新上站後不用再設定一次 :p");
  vmsg ("不過我還沒寫好啦 ^^;;");
  return XO_FOOT;
}
#endif


static int
bit_delpal(xo)
  XO *xo;
{
  int pos;
  char *nick;

  pos = xo->pos;
  nick = bit_pool[pos].nick;

  if (vans("確定要刪除好友 (Y/N)？[N]") == 'y')
  {
    fprintf(fw, "PRIVMSG root :remove %s\r\n", nick);
    fflush(fw);
  }
  return XO_INIT;
}


#if 0
static int
bit_onick(xo)
  XO *xo;
{
  int pos;
  char *nick, str[10];

  pos = xo->pos;
  nick = bit_pool[pos].nick;

  vmsg("暫時只能取英文");
  move(b_lines - 1, 0);
  clrtoeol ();

  if (!vget(b_lines, 0, "幫他取個新名字吧：", str, 10, DOECHO))
    return XO_FOOT;
  if (strchr(str, ';') || strchr(str, ','))
    return XO_FOOT;

  fprintf(fw, "PRIVMSG root :rename %s %s\r\n", nick, str);
  fflush(fw);

  bit_fgets();
  if (bit_sock <= 0)
    return XO_QUIT;

  while (!strstr(buf, "Nick"))
  {
    bit_fgets();
    if (bit_sock <= 0)
      return XO_QUIT;

  };

  sleep(1);

  return XO_INIT;
}
#endif


static int
bit_mynick()
{
  char nick[40];

  if (!vget(b_lines, 0, "我的新暱稱：", nick, 38, DOECHO))
    return XO_FOOT;
  if (strchr(nick, ';') || strchr(nick, ','))
    return XO_FOOT;

  fprintf(fw, "PRIVMSG root :nick 0 %s\r\n", nick);
  fflush(fw);

  return XO_FOOT;
}


static int
bit_addpal()
{
  char addr[40];

  if (!vget(b_lines, 0, "輸入新增好友信箱：", addr, 38, DOECHO))
    return XO_FOOT;

  fprintf(fw, "PRIVMSG root :add 0 %s\r\n", addr);
  fflush(fw);

  return XO_INIT;
}


void
bit_abort()
{
  if (bit_sock > 0)
  {
    fprintf(fw, "QUIT :bye\r\n");
    fflush(fw);
    fclose(fw);
    fclose(fr);
    bit_sock = 0;
  }
}


static int
bit_close()
{
  if (vans("確定要中斷 MSN 連線？(y/N) [N]") != 'y')
    return XO_FOOT;

  bit_abort();

  zmsg("請稍候 ..... ");
  sleep (1);
  vmsg("連線中斷！");

  return XO_QUIT;
}


static int
bit_show(xo)
  XO *xo;
{
  bit_display();
  return bit_head(xo);
}


#if 0
static int
bit_test()
{
  char smiler_buf[32];
  sprintf(smiler_buf, "%d", cutmp->pid);
  vmsg(smiler_buf);
  return XO_INIT;
}
#endif


#ifndef NEW_KeyFunc
static KeyFunc bit_cb[] = 
{
#ifdef  HAVE_LIGHTBAR
  XO_ITEM, bit_item_bar,
#endif
  XO_INIT, bit_set,	/* bit_init */
  XO_LOAD, bit_body,	/* bit_load */
  XO_HEAD, bit_head,
//  XO_BODY, bit_body,

  'b', bit_unblock,	/* 解除封鎖 */
  'B', bit_block,	/* 封鎖連絡人 */
  'a', bit_addpal,	/* 新增連絡人 */
  'd', bit_delpal,	/* 刪除連絡人 */
  'l', bit_show,	/* 回顧 FN_MSN 訊息 */
//  's', bit_save,
//  'n', bit_onick,
  'c', bit_mynick,	/* 更改自己的暱稱 */
  'w', bit_write,	/* 送 MSN 訊息 */
  Ctrl ('K'), bit_close,/* 中斷連線 */
  'h', bit_help
};
#else
static NewKeyFunc bit_cb[] = 
{
#ifdef  HAVE_LIGHTBAR
  XO_ITEM, bit_item_bar,        XO_ITEM,        'n',    "XO_ITEM",      NULL,
#endif
  /* bit_init */
  XO_INIT, bit_set,             XO_INIT,        'n',    "XO_INIT",      NULL,
  /* bit_load */
  XO_LOAD, bit_body,            XO_LOAD,        'n',    "XO_LOAD",      NULL,
  XO_HEAD, bit_head,            XO_HEAD,        'n',    "XO_HEAD",      NULL,
//  XO_BODY, bit_body,

  /* 解除封鎖 */
  'b', bit_unblock,             'b',    'p',    "解除封鎖",     NULL,
  /* 封鎖連絡人 */
  'B', bit_block,               'B',    'p',    "封鎖聯若人",   NULL,
  /* 新增連絡人 */
  'a', bit_addpal,              'a',    'p',    "新增聯絡人",   NULL,
  /* 刪除連絡人 */
  'd', bit_delpal,              'd',    'p',    "刪除聯絡人",   NULL,
  /* 回顧 FN_MSN 訊息 */
  'l', bit_show,                'l',    'p',    "回顧訊息",     NULL,
//  's', bit_save,
//  'n', bit_onick,
  /* 更改自己的暱稱 */
  'c', bit_mynick,              'c',    'p',    "更改自己暱稱", NULL,
  /* 送 MSN 訊息 */
  'w', bit_write,               'w',    'p',    "傳送MSN訊息",  NULL,
  /* 中斷連線 */
  Ctrl ('K'), bit_close,        Ctrl ('K'),     'p',    "中斷MSN連線",  NULL,
  'h', bit_help                 'h',    'z',    "\功\能說明",   NULL
};
#endif


void
bit_rqst()
{
  FILE *fp;
  char *nick, *msg/*, send[600]*/, file[128];

  while (fgets(buf, sizeof (buf), fr))
  {
    if (msg = strstr(buf, "PRIVMSG"))
    {
      msg = strstr(msg, ":");
      *msg++;
      nick = strtok(buf, "!");
      *nick++;
//      sprintf(send, "\033[1;33;46m★%s (@msn) \033[37;45m %s \033[m", nick, msg);

      usr_fpath(file, cuser.userid, FN_MSN);
      fp = fopen(file, "a");
      fprintf(fp, "\033[1;33;46m★%s (@msn) \033[m：%s", nick, msg);
      fclose(fp);
      cursor_save();

      /***  smiler.080319:送至bmw介面  ***/
      UTMP *up;
      BMW bmw;
      char buf[20];
      char bmw_msg[49];

      up = utmp_find(cuser.userno);
      sprintf(buf, "★<%s>", up->userid);
      if (strlen(msg) < 49)
	strcpy(bmw_msg, msg);
      else
	str_ncpy(bmw_msg, msg, 48);

      bmw_msg[strlen(bmw_msg)-1] = '\0';/* smiler.080319:處理bmw_msg結尾有 '\n' */
      strcpy(bmw.nick, nick);		/* smiler.080319: 用於bmw介面 reply msn */
      strcpy(bmw.msg, bmw_msg);
      bit_bmw_edit(up, buf, &bmw);
      /***********************************/
      cursor_restore();
      refresh();
      bell();
      if (strlen(msg) >= 49)	/* 長度超過水球容許範圍,才印出 */
	vmsg("MSN訊息過長，請至【 回顧 msn 訊息 】觀看完整訊息 !!");
      break;
    }
  }
}


int
bit_start(account, pass)
  char *account, *pass;
{
  int i = 0;
  char *tmp;

  if (bit_sock <= 0)
  {
    if (!account || !pass)	/* 以防萬一 */
      return 0;

    move(b_lines - 1, 0);
    sleep(1);
    clrtoeol();
    bit_sock = dns_open("127.0.0.1", 6667);
    zmsg("連接中 :p");
    sleep(1);

    if (bit_sock > 0)
    {
      zmsg("登入中，快了別急，本來登入就要等一下咩 :p (想像小綠人在轉 ^^O)");

      fr = fdopen(bit_sock, "r");
      fw = fdopen(bit_sock, "w");

      fprintf(fw, "NICK b%d\r\n", cutmp->pid);
      fflush(fw);
      fprintf(fw, "USER bitlbee ono ccy :bitlbee run\r\n");
      fflush(fw);
      fprintf(fw, "JOIN #bitlbee\r\n");
      fflush(fw);

      fprintf(fw, "PRIVMSG root :account add msn %s %s\r\n", account, pass);
      fflush(fw);
      fprintf(fw, "PRIVMSG root :account on\r\n");
      fflush(fw);
      fprintf(fw, "PRIVMSG root :set charset BIG-5\r\n");
      fflush(fw);

      sleep (10);


      i = 0;

      while(1)
      {
        bit_fgets();

	if (bit_sock <= 0)
	  return XO_QUIT;

	if (strstr(buf, "Error"))
	{
	  vmsg("帳號或密碼輸入錯誤喔 :p");
	  bit_abort();
	  sleep(1);
	  return 0;
	}
        else if(i >= 100)
        {
          return vmsg("MSN系統產生異常狀態，請至sysop板回報");
          bit_abort();
          sleep(1);
          return 0;
        }
        else if (strstr(buf, "Logging in: Logged in"))
          break;
	
	i++;
	
      }
    }
  }

  if (bit_sock > 0)
  {
    xz[XZ_BITLBEE - XO_ZONE].xo = &bit_xo;
    xz[XZ_BITLBEE - XO_ZONE].cb = bit_cb;
    if (i)
      bell();
    xover(XZ_BITLBEE);
  }
  else
    vmsg("無法開啟連線，請至 sysop 板回報");

  return 0;
}
#endif
