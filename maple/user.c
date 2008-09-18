/*-------------------------------------------------------*/
/* user.c	( NTHU CS MapleBBS Ver 3.00 )		 */
/*-------------------------------------------------------*/
/* target : account / user routines		 	 */
/* create : 95/03/29				 	 */
/* update : 96/04/05				 	 */
/*-------------------------------------------------------*/


#include "bbs.h"


extern char *ufo_tbl[];
extern char *usr_show_tbl[];


void
tn_user_show()	/* 使用者上站，初使化與載入使用者文章列表顯示喜好 */
{
  FILE *fp;
  char fpath[64];

  /* 讀出使用者個人設定的 USR_SHOW */
  usr_fpath(fpath, cuser.userid, "MY_USR_SHOW");
  if (fp = fopen(fpath, "r"))	//若檔案存在則讀出來
  {
    fscanf(fp, "%ud", &USR_SHOW);
    fclose(fp);
  }
  else					//反之則將我們 initial 的 USR_SHOW 寫回去
  {
    /* initialization USR_SHOW */
    USR_SHOW = -1 & ~(-1 << NUM_USR_SHOW);
    fp = fopen(fpath, "w");
    fprintf(fp, "%ud", USR_SHOW);
    fclose(fp);
  }
}

/* ----------------------------------------------------- */
/* 上站時初始化個人光棒					 */
/* ----------------------------------------------------- */

static char *DEFCBAR[CBAR_NUM] = {
		COLORBAR_MENU, COLORBAR_BRD, COLORBAR_POST, COLORBAR_GEM,
		COLORBAR_PAL, COLORBAR_USR, COLORBAR_BMW, COLORBAR_MAIL,
		COLORBAR_ALOHA, COLORBAR_VOTE, COLORBAR_NBRD, COLORBAR_SONG, COLORBAR_RSS };

#if 1
static void
trans_bar_set()		/* 過渡時期，使用者上站時轉檔 */
{
  FILE *fp;
  char fpath[64];
  int i;

  char color[32];
  char barname_in[24];
  char *old_set[CBAR_NUM] = {"_MENU", "_BRD", "_POST", "_GEM", "_PAL", "_USR",
		"_BMW", "_MAIL", "_ALOHA", "_VOTE", "_NBRD", "_SONG", "_RSS"};

  for (i = 0; i < CBAR_NUM; i++)
  {
    sprintf(barname_in, "%s.bar", old_set[i]);
    usr_fpath(fpath, cuser.userid, barname_in);
    if (fp = fopen(fpath, "r"))
    {
      if (fgets(color,32,fp))
      {
	if (color[0]!='\0' && color[0]!=' ')
	{
	  strcpy(UCBAR[i], color);
	}
      }
      fclose(fp);
      unlink(fpath);	/* 舊的設定檔沒用了 */
    }
    else	/* 沒有舊設定檔，以站預設載入 */
      strcpy(UCBAR[i], DEFCBAR[i]);
  }

  /* 建立新的設定檔 */
  usr_fpath(fpath, cuser.userid, ".BARSET");
  if (fp = fopen(fpath, "w"))
  {
    for (i = 0; i < CBAR_NUM; i++)
    {
      fprintf(fp, "%s\n", UCBAR[i]);
    }
    fclose(fp);
  }
}
#endif


void
tn_user_bar()		/* 使用者上站，初使化與載入使用者設定檔 */
{
  FILE *fp;
  char fpath[64];
  char color[32];
  int i;

#if 1		/* 過渡時期，使用者上站時轉檔 */
  usr_fpath(fpath, cuser.userid, ".BARSET");
  if (!dashf(fpath))	/* 沒有 .BARSET, 表示之前無任何相關設定，或是舊設定尚未轉檔 */
    trans_bar_set();
#endif

  if (fp = fopen(fpath, "r"))
  {
    for (i = 0; i < CBAR_NUM; i++)
    {
      fgets(color, 32, fp);
      if (color[0] != '\0' && color[0] != ' ')
      {
	 color[strlen(color) - 1] = '\0';
	 strcpy(UCBAR[i], color);
      }
    }
    fclose(fp);
  } 
}


/* ----------------------------------------------------- */
/* 設定個人光棒						 */
/* ----------------------------------------------------- */


static int
u_set_bar(bar)
  int bar;
{
  FILE *fp;
  char bright[2];
  char flash[2];
  char front[3];
  char back[3];
  char imaplecolor[32];
  char orgcolor[32];
  char color_write[32];
  char fpath[64];
  char ans;
  int i;

  /* load 楓橋原始光棒 */
  if (bar >= 13)
    strcpy(imaplecolor, "\033[m");
  else
  {
    strcpy(imaplecolor, DEFCBAR[bar]);
    strcpy(orgcolor, UCBAR[bar]);		/* load 使用者自定光棒 */
  }

  move(i = 1, 0);
  clrtobot();
  move(2, 0);
  prints("\033[1;37m亮   : \033[m\033[1;31m31\033[32m32\033[33m33\033[34m34\033[35m35\033[36m36\033[37m37\033[m\n");
  prints("\033[0;37m暗   : \033[m\033[31m31\033[32m32\033[33m33\033[34m34\033[35m35\033[36m36\033[37m37\033[m\n");
  prints("\033[0m底色 : \033[m\033[41m41\033[42m42\033[43m43\033[44m44\033[45m45\033[46m46\033[47m47\033[m\n");
  prints("\033[m楓橋設定 : %s測試\033[m\n", imaplecolor);
  prints("\033[m目前設定 : \033[m%s測試\033[m", orgcolor);

  i = 8;
  vget(i, 0, "前景：亮(1)/暗(0)/略過(Enter)", bright, 2, DOECHO);
  if (bright[0]!='\0' && bright[0] != '0' && bright[0] != '1')
  {
    return vmsg("輸入錯誤 !!");
  }

  sprintf(color_write,"\033[m\033[%s%s",
	(bright[0] == '\0') ? "" : bright, (bright[0] == '\0') ? "" : ";");
  color_write[strlen(color_write) - 1] = 'm';
  prints("\033[m目前設定 : \033[m%s%s測試\033[m", orgcolor, color_write);

  i += 2;
  vget(i, 0, "前景：閃爍(5)/略過(Enter)", flash, 2, DOECHO);
  if ((flash[0]!='\0') && (flash[0]!='5'))
  {
    return vmsg("輸入錯誤 !!");
  }
  sprintf(color_write,"\033[m\033[%s%s%s%s",
	(bright[0]=='\0') ? "" : bright, (bright[0]=='\0') ? "" : ";",
	(flash[0]=='\0')  ? "" : flash , (flash[0]=='\0')  ? "" : ";");
  color_write[strlen(color_write) - 1] = 'm';
  prints("\033[m目前設定 : \033[m%s%s測試\033[m", orgcolor, color_write);
	
  i += 2;
  vget(i, 0, "前景：色碼(31~37)/略過(Enter)", front, 3, DOECHO);
  if ((front[0] != '\0') && ((front[0] != '3') && (front[1] < '1' || front[1] > '7')))
  {
    return vmsg("輸入錯誤 !!");
  }
  sprintf(color_write,"\033[m\033[%s%s%s%s%s%s",
	(bright[0] == '\0') ? "" : bright, (bright[0] == '\0') ? "" : ";",
	(flash[0] == '\0')  ? "" : flash , (flash[0] == '\0')  ? "" : ";",
	(front[0] == '\0')  ? "" : front , (front[0] == '\0')  ? "" : ";");
  color_write[strlen(color_write) - 1] = 'm';
  prints("\033[m目前設定 : \033[m%s%s測試\033[m", orgcolor, color_write);

  i += 2;
  vget(i, 0, "背景：色碼(41~47)/略過(Enter)", back, 3, DOECHO);
  if ((back[0] != '\0') && ((back[0] != '4') && (back[1] < '1' || back[1] > '7')))
  {
    return vmsg("輸入錯誤 !!");
  }

  sprintf(color_write,"\033[m\033[%s%s%s%s%s%s%s%s",
	(bright[0] == '\0') ? "" : bright, (bright[0] == '\0') ? "" : ";",
	(flash[0] == '\0')  ? "" : flash , (flash[0] == '\0')  ? "" : ";",
	(front[0] == '\0')  ? "" : front , (front[0] == '\0')  ? "" : ";",
	(back[0] == '\0')   ? "" : back  , (back[0] == '\0')   ? "" : ";");
  color_write[strlen(color_write) - 1] = 'm';
  prints("\033[m目前設定 : \033[m%s%s測試\033[m", orgcolor, color_write);

  if (bright[0] == '\0' && flash[0] == '\0' && front[0] == '\0' && back[0] == '\0')
  {
    return vmsg("取消更動 !!");
  }

  switch (ans = vans("◎ 選擇 Y)確定 N)取消 D)預設 [Q] "))
  {
    case 'd' :
      strcpy(UCBAR[bar], DEFCBAR[bar]);
      break;
    case 'y' :
      strcpy(UCBAR[bar], color_write);
      break;
    default :
      return 0;
  }

  usr_fpath(fpath, cuser.userid, ".BARSET");
  if (fp = fopen(fpath, "w"))		/* 更新完直接寫檔 */
  {
    for (i = 0; i < CBAR_NUM; i++)
    {
      fprintf(fp, "%s\n", UCBAR[i]);
    }
    fclose(fp);
    return vmsg("更新成功\！");
  }
  else
    return vmsg("更新失敗，請重試一次！");
}


int
u_menu_bar()
{
  return u_set_bar(UCBAR_MENU);
}

int
u_brd_bar()
{
  return u_set_bar(UCBAR_BRD);
}

int
u_post_bar()
{
  return u_set_bar(UCBAR_POST);
}

int
u_gem_bar()
{
  return u_set_bar(UCBAR_GEM);
}

int
u_pal_bar()
{
  return u_set_bar(UCBAR_PAL);
}

int
u_usr_bar()
{
  return u_set_bar(UCBAR_USR);
}

int
u_bmw_bar()
{
  return u_set_bar(UCBAR_BMW);
}

int
u_mail_bar()
{
  return u_set_bar(UCBAR_MAIL);
}

int
u_aloha_bar()
{
  return u_set_bar(UCBAR_ALOHA);
}

int
u_vote_bar()
{
  return u_set_bar(UCBAR_VOTE);
}

int
u_newbrd_bar()
{
  return u_set_bar(UCBAR_NBRD);
}

int
u_song_bar()
{
  return u_set_bar(UCBAR_SONG);
}

int
u_rss_bar()
{
  return u_set_bar(UCBAR_RSS);
}


/* ----------------------------------------------------- */
/* 認證用函式						 */
/* ----------------------------------------------------- */


void
justify_log(userid, justify)	/* itoc.010822: 拿掉 .ACCT 中 justify 這欄位，改記錄在 FN_JUSTIFY */
  char *userid;
  char *justify;	/* 認證資料 RPY:email-reply  KEY:認證碼  POP:pop3認證  REG:註冊單 */
{
  char fpath[64];
  FILE *fp;

  usr_fpath(fpath, userid, FN_JUSTIFY);
  if (fp = fopen(fpath, "a"))		/* 用附加檔案，可以保存歷次認證記錄 */
  {
    fprintf(fp, "%s\n", justify);
    fclose(fp);
  }
}


static int
ban_addr(addr)
  char *addr;
{
  char *host;
  char foo[128];	/* SoC: 放置待檢查的 email address */

  /* Thor.991112: 記錄用來認證的email */
  sprintf(foo, "%s # %s (%s)\n", addr, cuser.userid, Now());
  f_cat(FN_RUN_EMAILREG, foo);

  /* SoC: 保持原 email 的大小寫 */
  str_lower(foo, addr);

  /* check for acl (lower case filter) */

  host = (char *) strchr(foo, '@');
  *host = '\0';

  /* *.bbs@xx.yy.zz、*.brd@xx.yy.zz 一律不接受 */
  if (host > foo + 4 && (!str_cmp(host - 4, ".bbs") || !str_cmp(host - 4, ".brd")))
    return 1;

  /* 不在白名單上或在黑名單上 */
  return (!acl_has(TRUST_ACLFILE, foo, host + 1) ||
    acl_has(UNTRUST_ACLFILE, foo, host + 1) > 0);
}


/* ----------------------------------------------------- */
/* POP3 認證						 */
/* ----------------------------------------------------- */


#ifdef HAVE_POP3_CHECK

static int		/* >=0:socket -1:連線失敗 */
Get_Socket(site)	/* site for hostname */
  char *site;
{
  int sock;
  struct sockaddr_in sin;
  struct hostent *host;

  sock = 110;

  /* Getting remote-site data */

  memset((char *) &sin, 0, sizeof(sin));
  sin.sin_family = AF_INET;
  sin.sin_port = htons(sock);

  if (!(host = gethostbyname(site)))
    sin.sin_addr.s_addr = inet_addr(site);
  else
    memcpy(&sin.sin_addr.s_addr, host->h_addr, host->h_length);

  /* Getting a socket */

  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    return -1;
  }

  /* perform connecting */

  if (connect(sock, (struct sockaddr *) & sin, sizeof(sin)) < 0)
  {
    close(sock);
    return -1;
  }

  return sock;
}


static int		/* 0:成功 */
POP3_Check(sock, account, passwd)
  int sock;
  char *account, *passwd;
{
  FILE *fsock;
  char buf[512];

  if (!(fsock = fdopen(sock, "r+")))
  {
    outs("\n傳回錯誤值，請重試幾次看看\n");
    return -1;
  }

  sock = 1;

  while (1)
  {
    switch (sock)
    {
    case 1:		/* Welcome Message */
      fgets(buf, sizeof(buf), fsock);
      break;

    case 2:		/* Verify Account */
      fprintf(fsock, "user %s\r\n", account);
      fflush(fsock);
      fgets(buf, sizeof(buf), fsock);
      break;

    case 3:		/* Verify Password */
      fprintf(fsock, "pass %s\r\n", passwd);
      fflush(fsock);
      fgets(buf, sizeof(buf), fsock);
      sock = -1;
      break;

    default:		/* 0:Successful 4:Failure  */
      fprintf(fsock, "quit\r\n");
      fclose(fsock);
      return sock;
    }

    if (!strncmp(buf, "+OK", 3))
    {
      sock++;
    }
    else
    {
      outs("\n遠端系統傳回錯誤訊息如下：\n");
      prints("%s\n", buf);
      sock = -1;
    }
  }
}


static int		/* -1:不支援 0:密碼錯誤 1:成功 */
do_pop3(addr)		/* itoc.010821: 改寫一下 :) */
  char *addr;
{
  int sock, i;
  char *ptr, *str, buf[80], username[80];
  char *alias[] = {"", "pop.", "pop3.", "mail.", NULL};
  ACCT acct;

  strcpy(username, addr);
  *(ptr = strchr(username, '@')) = '\0';
  ptr++;

  clear();
  move(2, 0);
  prints("主機: %s\n帳號: %s\n", ptr, username);
  outs("\033[1;5;36m連線遠端主機中...請稍候\033[m\n");
  refresh();

  for (i = 0; str = alias[i]; i++)
  {
    sprintf(buf, "%s%s", str, ptr);	/* itoc.020120: 主機名稱加上 pop3. 試試看 */
    if ((sock = Get_Socket(buf)) >= 0)	/* 找到這機器且對方支援 POP3 */
      break;
  }

  if (sock < 0)
  {
    outs("您的電子郵件系統不支援 POP3 認證，使用認證信函身分確認\n\n\033[1;36;5m系統送信中...\033[m");
    return -1;
  }

  if (vget(15, 0, "請輸入以上所列出之工作站帳號的密碼：", buf, 20, NOECHO))
  {
    move(17, 0);
    outs("\033[5;37m身分確認中...請稍候\033[m\n");

    if (!POP3_Check(sock, username, buf))	/* POP3 認證成功 */
    {
      /* 提升權限 */
      sprintf(buf, "POP: %s", addr);
      justify_log(cuser.userid, buf);
      strcpy(cuser.email, addr);
      if (acct_load(&acct, cuser.userid) >= 0)
      {
	time(&acct.tvalid);
	acct_setperm(&acct, PERM_VALID, 0);
      }

      /* 寄信通知使用者 */
      mail_self(FN_ETC_JUSTIFIED, str_sysop, msg_reg_valid, 0);
      cutmp->status |= STATUS_BIFF;
      vmsg(msg_reg_valid);

      close(sock);
      return 1;
    }
  }

  close(sock);

  /* POP3 認證失敗 */
  outs("您的密碼或許\打錯了，使用認證信函身分確認\n\n\033[1;36;5m系統送信中...\033[m");
  return 0;
}
#endif


/* ----------------------------------------------------- */
/* 設定 E-mail address					 */
/* ----------------------------------------------------- */


int
u_addr()
{
  char *msg, addr[64];

  if (HAS_STATUS(STATUS_COINLOCK))
  {
    vmsg(msg_coinlock);
    return XEASY;
  }

  /* itoc.050405: 不能讓停權者重新認證，因為會改掉他的 tvalid (停權到期時間) */
  if (HAS_PERM(PERM_ALLDENY))
  {
    vmsg("您被停權，無法改信箱");
    return XEASY;
  }

  film_out(FILM_EMAIL, 0);

  if (vget(b_lines - 2, 0, "E-mail 地址：", addr, sizeof(cuser.email), DOECHO))
  {
    if (not_addr(addr))
    {
      msg = err_email;
    }
    else if (ban_addr(addr))
    {
      msg = "本站不接受您的信箱做為認證地址";
    }
    else
    {
#ifdef EMAIL_JUSTIFY
      if (vans("修改 E-mail 要重新認證，確定要修改嗎(Y/N)？[Y] ") == 'n')
	return 0;

#  ifdef HAVE_POP3_CHECK
      if (vans("是否使用 POP3 認證(Y/N)？[N] ") == 'y')
      {
	if (do_pop3(addr) > 0)	/* 若 POP3 認證成功，則離開，否則以認證信寄出 */
	  return 0;
      }
#  endif

      if (bsmtp(NULL, NULL, addr, MQ_JUSTIFY) < 0)
      {
	msg = "身分認證信函無法寄出，請正確填寫 E-mail address";
      }
      else
      {
	ACCT acct;

	strcpy(cuser.email, addr);
	cuser.userlevel &= ~PERM_ALLVALID;
	if (acct_load(&acct, cuser.userid) >= 0)
	{
	  strcpy(acct.email, addr);
	  acct_setperm(&acct, 0, PERM_ALLVALID);
	}

	film_out(FILM_JUSTIFY, 0);
	prints("\n%s(%s)您好，由於您更新 E-mail address 的設定，\n\n"
	  "請您儘快到 \033[44m%s\033[m 所在的工作站回覆『身分認證信函』。",
	  cuser.userid, cuser.username, addr);
	msg = NULL;
      }
#else
      msg = NULL;
#endif

    }
    vmsg(msg);
  }

  return 0;
}


/* ----------------------------------------------------- */
/* 填寫註冊單						 */
/* ----------------------------------------------------- */


#ifdef HAVE_REGISTER_FORM

static void
getfield(line, len, buf, desc, hint)
  int line, len;
  char *hint, *desc, *buf;
{
  move(line, 0);
  prints("%s%s", desc, hint);
  vget(line + 1, 0, desc, buf, len, GCARRY);
}


int
u_register()
{
  FILE *fn;
  int ans;
  RFORM rform;

#ifdef JUSTIFY_PERIODICAL
  if (HAS_PERM(PERM_VALID) && cuser.tvalid + VALID_PERIOD - INVALID_NOTICE_PERIOD >= ap_start)
#else
  if (HAS_PERM(PERM_VALID))
#endif
  {
    zmsg("您的身分確認已經完成，不需填寫申請表");
    return XEASY;
  }

  if (fn = fopen(FN_RUN_RFORM, "rb"))
  {
    while (fread(&rform, sizeof(RFORM), 1, fn))
    {
      if ((rform.userno == cuser.userno) && !strcmp(rform.userid, cuser.userid))
      {
	fclose(fn);
	zmsg("您的註冊申請單尚在處理中，請耐心等候");
	return XEASY;
      }
    }
    fclose(fn);
  }

  if (vans("您確定要填寫註冊單嗎(Y/N)？[N] ") != 'y')
    return XEASY;

  move(1, 0);
  clrtobot();
  prints("\n%s(%s) 您好，請據實填寫以下的資料：\n(按 [Enter] 接受初始設定)",
    cuser.userid, cuser.username);

  memset(&rform, 0, sizeof(RFORM));
  for (;;)
  {
    getfield(5, 50, rform.career, "服務單位：", "學校系級或單位職稱");
    getfield(8, 60, rform.address, "目前住址：", "包括寢室或門牌號碼");
    getfield(11, 20, rform.phone, "連絡電話：", "包括長途撥號區域碼");
    ans = vans("以上資料是否正確(Y/N/Q)？[N] ");
    if (ans == 'q')
      return 0;
    if (ans == 'y')
      break;
  }

  rform.userno = cuser.userno;
  strcpy(rform.userid, cuser.userid);
  time(&rform.rtime);
  rec_add(FN_RUN_RFORM, &rform, sizeof(RFORM));
  return 0;
}
#endif


/* ----------------------------------------------------- */
/* 填寫註認碼						 */
/* ----------------------------------------------------- */


#ifdef HAVE_REGKEY_CHECK
int
u_verify()
{
  char buf[80], key[10];
  ACCT acct;

  if (HAS_PERM(PERM_VALID))
  {
    zmsg("您的身分確認已經完成，不需填寫認證碼");
  }
  else
  {
    if (vget(b_lines, 0, "請輸入認證碼：", buf, 8, DOECHO))
    {
      archiv32(str_hash(cuser.email, cuser.tvalid), key);	/* itoc.010825: 不用開檔了，直接拿 tvalid 來比就是了 */

      if (str_ncmp(key, buf, 7))
      {
	zmsg("抱歉，您的認證碼錯誤");
      }
      else
      {
	/* 提升權限 */
	sprintf(buf, "KEY: %s", cuser.email);
	justify_log(cuser.userid, buf);
	if (acct_load(&acct, cuser.userid) >= 0)
	{
	  time(&acct.tvalid);
	  acct_setperm(&acct, PERM_VALID, 0);
	}

	/* 寄信通知使用者 */
	mail_self(FN_ETC_JUSTIFIED, str_sysop, msg_reg_valid, 0);
	cutmp->status |= STATUS_BIFF;
	vmsg(msg_reg_valid);
      }
    }
  }

  return XEASY;
}
#endif


/* ----------------------------------------------------- */
/* 恢復權限						 */
/* ----------------------------------------------------- */


int
u_deny()
{
  ACCT acct;
  time_t diff;
  struct tm *ptime;
  char msg[80];

  if (!HAS_PERM(PERM_ALLDENY))
  {
    zmsg("您沒被停權，不需復權");
  }
  else
  {
    if ((diff = cuser.tvalid - time(0)) < 0)      /* 停權時間到了 */
    {
      if (acct_load(&acct, cuser.userid) >= 0)
      {
	time(&acct.tvalid);
#ifdef JUSTIFY_PERIODICAL
	/* xeon.050112: 在認證快到期前時 Cross-Post，然後 tvalid 就會被設定到未來時間，
	   等復權時間到了去復權，這樣就可以避過重新認證，所以復權後要重新認證。 */
	acct_setperm(&acct, 0, PERM_ALLVALID | PERM_ALLDENY);
#else
	acct_setperm(&acct, 0, PERM_ALLDENY);
#endif
	vmsg("下次請勿再犯，請重新上站");
      }
    }
    else
    {
      ptime = gmtime(&diff);
      sprintf(msg, "您還要等 %d 年 %d 天 %d 時 %d 分 %d 秒才能申請復權",
	ptime->tm_year - 70, ptime->tm_yday, ptime->tm_hour, ptime->tm_min, ptime->tm_sec);
      vmsg(msg);
    }
  }

  return XEASY;
}


/* ----------------------------------------------------- */
/* 個人工具						 */
/* ----------------------------------------------------- */


int
u_info()
{
  char *str, username[UNLEN + 1];

  if (HAS_STATUS(STATUS_COINLOCK))
  {
    vmsg(msg_coinlock);
    return XEASY;
  }

  move(1, 0); 
  strcpy(username, str = cuser.username);
  acct_setup(&cuser, 0);
  if (strcmp(username, str))
    memcpy(cutmp->username, str, UNLEN + 1);
  return 0;
}


int
u_sign_set()	/*ikulan.080726:將改站簽功能獨立出來*/
{
   FILE *hello;		/* smiler.071030: 站簽個人化內使用者想對大家說的話 */
   FILE *file_host;	/* smiler.071110: 個人選擇站簽 */
   int  choice;		/* 選用第幾個站簽 */
   char buf[ANSILINELEN];
   char helloworld[38];

   /*ikulan.080727: 印出站簽*/
   FILE *host_sign;

#define HOST_SIGN_NUM	4	/*ikulan.080727: 站簽的數目，此有四個站簽*/

   move(1, 0);
   clrtobot();

   /* smiler.071030: 輸入使用者想對大家說的話 */
   usr_fpath(buf, cuser.userid, "hello");
   if (hello = fopen(buf, "r"))
   {
     fgets(helloworld, 38, hello);
     fclose(hello);
     unlink(buf);
   }
   else
     helloworld[0] = '\0';

   if(!vget(9, 0, "想對大家說的話：", helloworld, 38, GCARRY))
   {
     if(!hello)
     {
       hello = fopen(buf, "w");
       fprintf(hello,"歡迎大家多來楓橋逛逛\\(*￣︶￣*)/");
       fclose(hello);
     }
   }
   else
   {
     hello = fopen(buf, "w");
     fputs(helloworld, hello);
     fclose(hello);
   }

   usr_fpath(buf, cuser.userid, "host");
   if (file_host = fopen(buf,"r"))
   {
     fgets(buf, 3, file_host);
     fclose(file_host);
   }
   choice = atoi(buf);

   vmsg(NULL);
   move(1,0);
   clrtobot();

   int i;
   for(i = 1; i <= HOST_SIGN_NUM; i++)
   {
     prints("\n%d號站簽：\n", i);
     sprintf(buf, "gem/@/@host_%d", i - 1);
     if(host_sign = fopen(buf, "r"))
     {
       while(fgets(buf, ANSILINELEN, host_sign))
         prints("%s", buf);
       fclose(host_sign);
     }
   }

   sprintf(buf, "◎ 選擇 0)隨機 1~4)選擇站簽 [%d] ", choice);
   choice = vans(buf);
   if (choice >= '0' && choice <= HOST_SIGN_NUM + '0')
     choice -= '0';
   else
     return 0;

   usr_fpath(buf, cuser.userid, "host");
   if (file_host = fopen(buf, "w"))
   {
     fprintf(file_host, "%d", choice);
     fclose(file_host);
   }

}	/*ikulan.080726:(end of function)將改站簽功能獨立出來*/


int
u_setup()
{
  usint ulevel;
  int len;

  /* itoc.000320: 增減項目要更改 len 大小, 也別忘了改 ufo.h 的旗標 STR_UFO */

  ulevel = cuser.userlevel;
  if (!ulevel)
    len = NUMUFOS_GUEST;
  else if (ulevel & PERM_ALLADMIN)
    len = NUMUFOS;		/* ADMIN 除了可用 acl，還順便也可以用隱身術 */
  else if (ulevel & PERM_CLOAK)
    len = NUMUFOS - 2;		/* 不能用紫隱、acl */
  else
    len = NUMUFOS_USER;

  cuser.ufo = cutmp->ufo = bitset(cuser.ufo, len, len, MSG_USERUFO, ufo_tbl);

  return 0;
}

int
u_usr_show_set()
{
  FILE *fp;
  char filepath[64];
  int len;

  len = NUM_USR_SHOW;
  USR_SHOW = bitset(USR_SHOW, len, len, MSG_USR_SHOW, usr_show_tbl);

  usr_fpath(filepath, cuser.userid, "MY_USR_SHOW");
  fp = fopen(filepath, "w");
  fprintf(fp, "%ud", USR_SHOW);
  fclose(fp);

  return 0;
}

int
u_lock()
{
  char buf[PSWDLEN + 1];

  switch (vans("是否進入螢幕鎖定狀態，將不能傳送/接收水球(Y/N/C)？[N] "))
  {
  case 'c':		/* itoc.011226: 可自行輸入發呆的理由 */
    if (vget(b_lines, 0, "請輸入發呆的理由：", cutmp->mateid, IDLEN + 1, DOECHO))
      break;

  case 'y':
    strcpy(cutmp->mateid, "掛站");
    break;

  default:
    return XEASY;
  }

  bbstate |= STAT_LOCK;		/* lkchu.990513: 鎖定時不可回水球 */
  cutmp->status |= STATUS_REJECT;	/* 鎖定時不收水球 */

  clear();
  move(5, 20);
  prints("\033[1;33m" BBSNAME "    閒置/鎖定狀態\033[m  [%s]", cuser.userid);

  do
  {
    vget(b_lines, 0, "◆ 請輸入密碼，以解除螢幕鎖定：", buf, PSWDLEN + 1, NOECHO);
  } while (chkpasswd(cuser.passwd, buf));

  cutmp->status ^= STATUS_REJECT;
  bbstate ^= STAT_LOCK;

  return 0;
}


int
u_log()
{
  char fpath[64];

  usr_fpath(fpath, cuser.userid, FN_LOG);
  more(fpath, NULL);
  return 0;
}


/* ----------------------------------------------------- */
/* 設定檔案						 */
/* ----------------------------------------------------- */


/* static */			/* itoc.010110: 給 a_xfile() 用 */
void
x_file(mode, xlist, flist)
  int mode;			/* M_XFILES / M_UFILES */
  char *xlist[];		/* description list */
  char *flist[];		/* filename list */
{
  int n, i;
  char *fpath, *desc;
  char buf[64];

  //move(MENU_XPOS, 0);
  move(1, 0);
  clrtobot();
  n = 0;
  while (desc = xlist[n])
  {
    n++;
    if (n <= 18)			/* itoc.020123: 分二欄，一欄十八個 */
    {
      move(n + 1 - 1, 0);
      //prints("\033[m  (%d)",n);         /* smiler.071112: 修正note色碼多出來 */
      clrtoeol();
      //move(n + 1 - 1, 2);
    }
    else
    {
      move(n + 1 - 19, 40);
    }
    prints("\033[m  (%d) %s", n, desc);

    if (mode == M_XFILES)	/* Thor.980806.註解: 印出檔名 */
    {
      if (n <= 18)
	move(n + 1 - 1, 22);
      else
	move(n + 1 - 19, 62);
      outs(flist[n - 1]);
    }
  }

  vget(b_lines, 0, "請選擇檔案編號，或按 [0] 取消：", buf, 3, DOECHO);
  i = atoi(buf);
  if (i <= 0 || i > n)
    return;

  n = vget(b_lines, 36, "(D)刪除 (E)編輯 [Q]取消？", buf, 3, LCECHO);
  if (n != 'd' && n != 'e')
    return;

  fpath = flist[--i];
  if (mode == M_UFILES)
    usr_fpath(buf, cuser.userid, fpath);
  else			/* M_XFILES */
    strcpy(buf, fpath);

  if (n == 'd')
  {
    if (vans(msg_sure_ny) == 'y')
      unlink(buf);
  }
  else
  {
	  if(mode == M_XFILES)
	  {
		char fpath_info[64];
		sprintf(fpath_info, BBSHOME"/gem/@/@mxfile.info");
		more(fpath_info, NULL);
	  }

    vmsg(vedit(buf, 0) ? "原封不動" : "更新完畢");	/* Thor.981020: 注意被talk的問題  */
  }
}


int
u_xfile()
{
  int i;

  static char *desc[] = 
  {
    "上站地點設定檔",
    "名片檔",
    "暫存檔.1",
    "暫存檔.2",
    "暫存檔.3",
    "暫存檔.4",
    "暫存檔.5",
    "簽名檔.1",
    "簽名檔.2",
    "簽名檔.3",
    "簽名檔.4",
    "簽名檔.5",
    "簽名檔.6",
    "簽名檔.7",
    "簽名檔.8",
    "簽名檔.9",
    NULL
  };

  static char *path[] = 
  {
    "acl",
    "plans",
    "buf.1",
    "buf.2",
    "buf.3",
    "buf.4",
    "buf.5",
    FN_SIGN ".1",
    FN_SIGN ".2",
    FN_SIGN ".3",
    FN_SIGN ".4",
    FN_SIGN ".5",
    FN_SIGN ".6",
    FN_SIGN ".7",
    FN_SIGN ".8",
    FN_SIGN ".9"
  };

  i = HAS_PERM(PERM_ALLADMIN) ? 0 : 1;
  x_file(M_UFILES, &desc[i], &path[i]);
  return 0;
}
