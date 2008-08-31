/*-------------------------------------------------------*/
/* admutil.c	( NTHU CS MapleBBS Ver 3.10 )		 */
/*-------------------------------------------------------*/
/* target : 站長指令					 */
/* create : 95/03/29					 */
/* update : 01/03/01					 */
/*-------------------------------------------------------*/


#include "bbs.h"


extern BCACHE *bshm;
extern UCACHE *ushm;

//extern void setuploader();

/* ----------------------------------------------------- */
/* 站務指令						 */
/* ----------------------------------------------------- */


int
a_user()
{
  int ans;
  ACCT acct;

  move(1, 0);
  clrtobot();

  while (ans = acct_get(msg_uid, &acct))
  {
    if (ans > 0)
      acct_setup(&acct, 1);
  }
  return 0;
}

/* smiler.071110: 站務系統設定:控制設定 */
int
a_system_setup()
{
  int i;
  char buf[64];
  char set_path[64],set_tmp[64],tmp_space[64];
  FILE *fp;
  FILE *f_tmp;
  int ans;


  int host_sight_number_tmp=0;      /* 站簽個數設定 */
  int host_sight_select_tmp=0;      /* 站簽全站指定 */
  int model_number_tmp=0;           /* 站簽娃個數設定 */
  int model_select_tmp=0;           /* 站簽娃使用全站指定 */           
  int editlog_use_tmp=0;            /* 是否啟用Editlog功能? */
  int deletelog_use_tmp=0;          /* 是否啟用Deletelog功能? */
  int mail_to_newone_tmp=0;         /* 是否寄信給新申請者? */


  move(i=1, 0);
  clrtobot();

  /* 站簽個數設定 */  
  sprintf(buf, "%d", host_sight_number);
  vget(++i, 0, "站簽個數設定：", buf, 10, GCARRY);
  host_sight_number_tmp=atoi(buf);
  if(host_sight_number_tmp <= 0)
  {
	  vmsg("最大站簽數不得小於等於0,更改數目為 1 ");
	  host_sight_number_tmp=1;
  }
  sprintf(tmp_space,"gem/@/@host_%d",host_sight_number_tmp-1);
  if(f_tmp=fopen(tmp_space,"r"))
	  fclose(f_tmp);
  else
  {
	  vmsg("最大站簽數目不存在,更改數目為 1 ");
	  host_sight_number_tmp=1;
  }

  /* 站簽全站指定 */
  sprintf(buf, "%d", host_sight_select);
  vget(++i, 0, "站簽全站指定(按0取消指定)：", buf, 10, GCARRY);
  host_sight_select_tmp=atoi(buf);
  if(host_sight_select_tmp !=0)
  {
    if(host_sight_select_tmp < 0)
	{
	    vmsg("指定站簽數不得小於等於0,更改指定為 1 ");
	    host_sight_select_tmp=1;
	}
    sprintf(tmp_space,"gem/@/@host_%d",host_sight_select_tmp-1);
    if(f_tmp=fopen(tmp_space,"r"))
	    fclose(f_tmp);
    else
	{
	    vmsg("指定站簽不存在,更改指定為 1 ");
	    host_sight_select_tmp=1;
	}
  }


  /* 站簽娃個數設定 */
  sprintf(buf, "%d", model_number);
  vget(++i, 0, "站簽娃個數設定：", buf, 10, GCARRY);
  model_number_tmp=atoi(buf);
  if(model_number_tmp <= 0)
  {
	  vmsg("站簽娃個數不得小於等於0,更改數目為 1 ");
	  model_number_tmp=1;
  }
  sprintf(tmp_space,"gem/@/@model_%d",model_number_tmp-1);
  if(f_tmp=fopen(tmp_space,"r"))
	  fclose(f_tmp);
  else
  {
	  vmsg("最大站簽娃個數不存在,更改數目為 1 ");
	  model_number_tmp=1;
  }



  /* 站簽娃全站指定 */
  sprintf(buf, "%d", model_select);
  vget(++i, 0, "站簽娃使用全站指定(按0取消指定)：", buf, 10, GCARRY);
  model_select_tmp=atoi(buf);
  if(model_select_tmp !=0)
  {
   if(model_select_tmp < 0)
   {
 	  vmsg("站簽娃指定編號不得小於等於0,更改指定為 1 ");
	  model_select_tmp=1;
   }
   sprintf(tmp_space,"gem/@/@model_%d",model_select_tmp-1);
   if(f_tmp=fopen(tmp_space,"r"))
 	  fclose(f_tmp);
   else
   {
	  vmsg("指定站簽娃不存在,更改指定為 1 ");
	  model_select_tmp=1;
   }
  }


  /* 啟用Editlog功能 */
  sprintf(buf, "%d", editlog_use);
  vget(++i, 0, "是否啟用Editlog功\能?(是:1 否:0)：", buf, 10, GCARRY);
  if(buf[0]=='1' || buf[0]=='0')
    editlog_use_tmp=atoi(buf);
  else
	  editlog_use_tmp=0;

  sprintf(buf, "%d", deletelog_use);
  vget(++i, 0, "是否啟用Deletelog功\能?(是:1 否:0)：", buf, 10, GCARRY);
  if(buf[0]=='1' || buf[0]=='0')
    deletelog_use_tmp=atoi(buf);
  else
	  deletelog_use_tmp=0;

  sprintf(buf, "%d", mail_to_newone);
  vget(++i, 0, "是否寄信給新申請者?(是:1 否:0)：", buf, 10, GCARRY);
  if(buf[0]=='1' || buf[0]=='0')
    mail_to_newone_tmp=atoi(buf);
  else
	  mail_to_newone=0;


  ans = vans("設定 1)確定 Q)取消 [Q] ");
  {
	  if(ans=='1')
	  {
        sprintf(set_path,".SET");
        sprintf(set_tmp,".SET.TMP");

		/* 將上述的修改寫回 .USR 以及對全站做同步更新 */
        fp=fopen(set_tmp,"w");
		{
          host_sight_number=host_sight_number_tmp;
          fprintf(fp,"%d\n",host_sight_number_tmp);

          host_sight_select=host_sight_select_tmp;
          fprintf(fp,"%d\n",host_sight_select_tmp);

          model_number=model_number_tmp;
          fprintf(fp,"%d\n",model_number_tmp);

          model_select=model_select_tmp;
		  fprintf(fp,"%d\n",model_select_tmp);

          editlog_use=editlog_use_tmp;
          fprintf(fp,"%d\n",editlog_use_tmp);

          deletelog_use=deletelog_use_tmp;
          fprintf(fp,"%d\n",deletelog_use_tmp);

          mail_to_newone=mail_to_newone_tmp;
          fprintf(fp,"%d\n",mail_to_newone_tmp);

		  fclose(fp);
		}
        unlink(set_path);              /* smiler.071110: 上述設定保存一份於 .USR */
        rename(set_tmp, set_path);     /* 修改站務設定設定選項時,記得要順便改  */
		                               /* bbsd.c 內的 setuploader() */
	  }
  }
  return 0;
}



int
a_search()	/* itoc.010902: 暴力搜尋使用者 */
{
  ACCT acct;
  char c;
  char key[30];

  if (!vget(b_lines, 0, "請輸入關鍵字(姓名/暱稱/來源/信箱)：", key, 30, DOECHO))
    return XEASY;  

  /* itoc.010929.註解: 真是有夠暴力 :p 考慮先由 reaper 做出一個 .PASSWDS 再去找 */

  for (c = 'a'; c <= 'z'; c++)
  {
    char buf[64];
    struct dirent *de;
    DIR *dirp;

    sprintf(buf, "usr/%c", c);
    if (!(dirp = opendir(buf)))
      continue;

    while (de = readdir(dirp))
    {
      if (acct_load(&acct, de->d_name) < 0)
	continue;

      if (strstr(acct.realname, key) || strstr(acct.username, key) ||  
	strstr(acct.lasthost, key) || strstr(acct.email, key))
      {
	move(1, 0);
	acct_setup(&acct, 1);

	if (vans("是否繼續搜尋下一筆？[N] ") != 'y')
	{
	  closedir(dirp);
 	  goto end_search;
 	}
      }
    }
    closedir(dirp);
  }
end_search:
  vmsg("搜尋完畢");
  return 0;
}


int
a_editbrd()		/* itoc.010929: 修改看板選項 */
{
  int bno;
  BRD *brd;
  char bname[BNLEN + 1];

  if (brd = ask_board(bname, BRD_R_BIT, NULL))
  {
    bno = brd - bshm->bcache;
    brd_edit(bno);
  }
  else
  {
    vmsg(err_bid);
  }

  return 0;
}


int
a_xfile()		/* 設定系統檔案 */
{
  static char *desc[] =
  {
    "看板文章期限",

    "身分認證信函",
    "認證通過通知",
    "重新認證通知",

#ifdef HAVE_DETECT_CROSSPOST
    "跨貼停權通知",
#endif
    
    "不雅名單",
    "站務名單",

    "節日",

#ifdef HAVE_WHERE
    "故鄉 IP",
    "故鄉 FQDN",
#endif

#ifdef HAVE_TIP
    "每日小秘訣",
#endif

#ifdef HAVE_LOVELETTER
    "情書產生器文庫",
#endif

    "認證白名單",
    "認證黑名單",

    "收信白名單",
    "收信黑名單",

#ifdef  HAVE_DETECT_CROSSPOST
    "可Crosspost",
#endif

#ifdef HAVE_LOGIN_DENIED
    "接受連線名單",    /* smiler.070724 */
    "拒絕連線名單",
#endif
    "站務的情書",
	"BBS 看門狗",
    NULL
  };

  static char *path[] =
  {
    FN_ETC_EXPIRE,

    FN_ETC_VALID,
    FN_ETC_JUSTIFIED,
    FN_ETC_REREG,

#ifdef HAVE_DETECT_CROSSPOST
    FN_ETC_CROSSPOST,
#endif
    
    FN_ETC_BADID,
    FN_ETC_SYSOP,

    FN_ETC_FEAST,

#ifdef HAVE_WHERE
    FN_ETC_HOST,
    FN_ETC_FQDN,
#endif

#ifdef HAVE_TIP
    FN_ETC_TIP,
#endif

#ifdef HAVE_LOVELETTER
    FN_ETC_LOVELETTER,
#endif

    TRUST_ACLFILE,
    UNTRUST_ACLFILE,

    MAIL_ACLFILE,
    UNMAIL_ACLFILE,

#ifdef  HAVE_DETECT_CROSSPOST
    FN_ETC_NOCROSSPOST,
#endif

#ifdef HAVE_LOGIN_DENIED
    BBS_ACPFILE,
    BBS_DNYFILE,
#endif
    FN_ETC_SYSMAIL,
	FN_ETC_BBSDOG,
	NULL
  };

  x_file(M_XFILES, desc, path);
  return 0;
}


int
a_resetsys()		/* 重置 */
{
  switch (vans("◎ 系統重設 1)動態看板 2)分類群組 3)指名及擋信 4)全部：[Q] "))
  {
  case '1':
    system("bin/camera");
    break;

  case '2':
    system("bin/account -nokeeplog");
    brh_save();
    board_main();
    break;

  case '3':
    system("kill -1 `cat run/bmta.pid`; kill -1 `cat run/bguard.pid`");
    break;

  case '4':
    system("kill -1 `cat run/bmta.pid`; kill -1 `cat run/bguard.pid`; bin/account -nokeeplog; bin/camera");
    brh_save();
    board_main();
    break;
  }

  return XEASY;
}


/* ----------------------------------------------------- */
/* 還原備份檔						 */
/* ----------------------------------------------------- */


static void
show_availability(type)		/* 將 BAKPATH 裡面所有可取回備份的目錄印出來 */
  char *type;
{
  int tlen, len, col;
  char *fname, fpath[64];
  struct dirent *de;
  DIR *dirp;
  FILE *fp;

  if (dirp = opendir(BAKPATH))
  {
    col = 0;
    tlen = strlen(type);

    sprintf(fpath, "tmp/restore.%s", cuser.userid);
    fp = fopen(fpath, "w");
    fputs("※ 可供取回的備份有：\n\n", fp);

    while (de = readdir(dirp))
    {
      fname = de->d_name;
      if (!strncmp(fname, type, tlen))
      {
	len = strlen(fname) + 2;
	if (SCR_WIDTH - col < len)
	{
	  fputc('\n', fp);
	  col = 0;
	}
	else
	{
	  col += len;
	}
	fprintf(fp, "%s  ", fname);
      }
    }

    fputc('\n', fp);
    fclose(fp);
    closedir(dirp);

    more(fpath, (char *) -1);
    unlink(fpath);
  }
}


int
a_restore()
{
  int ch;
  char *type, *ptr;
  char *tpool[3] = {"brd", "gem", "usr"};
  char date[20], brdname[BNLEN + 1], src[64], cmd[256];
  ACCT acct;
  BPAL *bpal;

  ch = vans("◎ 還原備份 1)看板 2)精華區 3)使用者：[Q] ") - '1';
  if (ch < 0 || ch >= 3)
    return XEASY;

  type = tpool[ch];
  show_availability(type);

  if (vget(b_lines, 0, "要取回的備份目錄：", date, 20, DOECHO))
  {
    /* 避免站長打了一個存在的目錄，但是和 type 不合 */
    if (strncmp(date, type, strlen(type)))
      return 0;

    sprintf(src, BAKPATH"/%s", date);
    if (!dashd(src))
      return 0;
    ptr = strchr(src, '\0');

    clear();
    move(3, 0);
    outs("欲還原備份的看板/使用者必須已存在。\n"
      "若該看板/使用者已刪除，請先重新開設/註冊一個同名的看板/使用者。\n"
      "還原備份時請確認該看板無人使用/使用者不在線上");

    if (ch == 0 || ch == 1)
    {
      if (!ask_board(brdname, BRD_L_BIT, NULL))
	return 0;
      sprintf(ptr, "/%s%s.tgz", ch == 0 ? "" : "brd/", brdname);
    }
    else /* if (ch == 2) */
    {
      if (acct_get(msg_uid, &acct) <= 0)
	return 0;
      type = acct.userid;
      str_lower(type, type);
      sprintf(ptr, "/%c/%s.tgz", *type, type);
    }

    if (!dashf(src))
    {
      /* 檔案不存在，通常是因為備份點時該看板/使用者已被刪除，或是當時根本就還沒有該看板/使用者 */
      vmsg("備份檔案不存在，請試試其他時間點的備份");
      return 0;
    }

    if (vans("還原備份後，目前所有資料都會流失，請務必確定(Y/N)？[N] ") != 'y')
      return 0;

    alog("還原備份", src);

    /* 解壓縮 */
    if (ch == 0)
      ptr = "brd";
    else if (ch == 1)
      ptr = "gem/brd";
    else /* if (ch == 2) */
      sprintf(ptr = date, "usr/%c", *type);
    sprintf(cmd, "tar xfz %s -C %s/", src, ptr);
    /* system(cmd); */

#if 1	/* 讓站長手動執行 */
    move(7, 0);
    outs("\n請以 bbs 身分登入工作站，並於\033[1;36m家目錄\033[m執行\n\n\033[1;33m");
    outs(cmd);
    outs("\033[m\n\n");
#endif

    /* tar 完以後，還要做的事 */
    if (vans("◎ 指令 Y)已成功\執行以上指令 Q)放棄執行：[Q] ") == 'y')
    {
      if (ch == 0)	/* 還原看板時，要更新板友 */
      {
	if ((ch = brd_bno(brdname)) >= 0)
	{
	  brd_fpath(src, brdname, fn_pal);
	  bpal = bshm->pcache + ch;
	  bpal->pal_max = image_pal(src, bpal->pal_spool);
	}
      }
      else if (ch == 2)	/* 還原使用者時，不還原 userno */
      {
	ch = acct.userno;
	if (acct_load(&acct, type) >= 0)
	{
	  acct.userno = ch;
	  acct_save(&acct);
	}
      }
      vmsg("還原備份成功\");
      return 0;
    }
  }

  vmsg(msg_cancel);
  return 0;
}


#ifdef HAVE_REGISTER_FORM

/* ----------------------------------------------------- */
/* 處理 Register Form					 */
/* ----------------------------------------------------- */


static int
scan_register_form(fd)
  int fd;
{
  static char logfile[] = FN_RUN_RFORM_LOG;
  static char *reason[] = 
  {
    "輸入真實姓名", "詳實填寫申請表", "詳填住址資料", "詳填連絡電話", 
    "詳填服務單位、或學校系級", "用中文填寫申請單", 
#ifdef EMAIL_JUSTIFY	/* waynesan.040327: 有 E-mail 認證才有此項 */
    "採用 E-mail 認證", 
#endif
    NULL
  };

  ACCT acct;
  RFORM rform;
  HDR hdr;
  FILE *fout;

  int op, n;
  char buf[256], *agent, *userid, *str;
  char folder[64], fpath[64];

  vs_bar("審核使用者註冊資料");
  agent = cuser.userid;

  while (read(fd, &rform, sizeof(RFORM)) == sizeof(RFORM))
  {
    userid = rform.userid;
    move(2, 0);
    prints("申請代號: %s (申請時間：%s)\n", userid, Btime(&rform.rtime));
    prints("服務單位: %s\n", rform.career);
    prints("目前住址: %s\n", rform.address);
    prints("連絡電話: %s\n%s\n", rform.phone, msg_seperator);
    clrtobot();

    if ((acct_load(&acct, userid) < 0) || (acct.userno != rform.userno))
    {
      vmsg("查無此人");
      op = 'd';
    }
    else
    {
      acct_show(&acct, 2);

#ifdef JUSTIFY_PERIODICAL
      if (acct.userlevel & PERM_VALID && acct.tvalid + VALID_PERIOD - INVALID_NOTICE_PERIOD >= acct.lastlogin)
#else
      if (acct.userlevel & PERM_VALID)
#endif
      {
	vmsg("此帳號已經完成註冊");
	op = 'd';
      }
      else if (acct.userlevel & PERM_ALLDENY)
      {
	/* itoc.050405: 不能讓停權者重新認證，因為會改掉他的 tvalid (停權到期時間) */
	vmsg("此帳號目前被停權中");
	op = 'd';
      }
      else
      {
	op = vans("是否接受(Y/N/Q/Del/Skip)？[S] ");
      }
    }

    switch (op)
    {
    case 'y':

      /* 提升權限 */
      sprintf(buf, "REG: %s:%s:%s:by %s", rform.phone, rform.career, rform.address, agent);
      justify_log(acct.userid, buf);
      time(&acct.tvalid);
      /* itoc.041025: 這個 acct_setperm() 並沒有緊跟在 acct_load() 後面，中間隔了一個 vans()，
         這可能造成拿舊 acct 去覆蓋新 .ACCT 的問題。不過因為是站長才有的權限，所以就不改了 */
      acct_setperm(&acct, PERM_VALID, 0);

      /* 寄信通知使用者 */
      usr_fpath(folder, userid, fn_dir);
      hdr_stamp(folder, HDR_LINK, &hdr, FN_ETC_JUSTIFIED);
      strcpy(hdr.title, msg_reg_valid);
      strcpy(hdr.owner, str_sysop);
      rec_add(folder, &hdr, sizeof(HDR));

      strcpy(rform.agent, agent);
      rec_add(logfile, &rform, sizeof(RFORM));

      m_biff(rform.userno);

      break;

    case 'q':			/* 太累了，結束休息 */

      do
      {
	rec_add(FN_RUN_RFORM, &rform, sizeof(RFORM));
      } while (read(fd, &rform, sizeof(RFORM)) == sizeof(RFORM));

    case 'd':
      break;

    case 'n':

      move(9, 0);
      prints("請提出退回申請表原因，按 <enter> 取消\n\n");
      for (n = 0; str = reason[n]; n++)
	prints("%d) 請%s\n", n, str);
      clrtobot();

      if (op = vget(b_lines, 0, "退回原因：", buf, 60, DOECHO))
      {
	int i;

	i = op - '0';
	if (i >= 0 && i < n)
	  strcpy(buf, reason[i]);

	usr_fpath(folder, acct.userid, fn_dir);
	if (fout = fdopen(hdr_stamp(folder, 0, &hdr, fpath), "w"))
	{
	  fprintf(fout, "\t由於您提供的資料不夠詳實，無法確認身分，"
	    "\n\n\t請重新填寫註冊表單：%s。\n", buf);
	  fclose(fout);

	  strcpy(hdr.owner, agent);
	  strcpy(hdr.title, "[退件] 請您重新填寫註冊表單");
	  rec_add(folder, &hdr, sizeof(HDR));
	}

	strcpy(rform.reply, buf);	/* 理由 */
	strcpy(rform.agent, agent);
	rec_add(logfile, &rform, sizeof(RFORM));

	break;
      }

    default:			/* put back to regfile */

      rec_add(FN_RUN_RFORM, &rform, sizeof(RFORM));
    }
  }
}


int
a_register()
{
  int num;
  char buf[80];

  num = rec_num(FN_RUN_RFORM, sizeof(RFORM));
  if (num <= 0)
  {
    zmsg("目前並無新註冊資料");
    return XEASY;
  }

  sprintf(buf, "共有 %d 筆資料，開始審核嗎(Y/N)？[N] ", num);
  num = XEASY;

  if (vans(buf) == 'y')
  {
    sprintf(buf, "%s.tmp", FN_RUN_RFORM);
    if (dashf(buf))
    {
      vmsg("其他 SYSOP 也在審核註冊申請單");
    }
    else
    {
      int fd;

      rename(FN_RUN_RFORM, buf);
      fd = open(buf, O_RDONLY);
      if (fd >= 0)
      {
	scan_register_form(fd);
	close(fd);
	unlink(buf);
	num = 0;
      }
      else
      {
	vmsg("無法開啟註冊資料工作檔");
      }
    }
  }
  return num;
}


int
a_regmerge()			/* itoc.000516: 斷線時註冊單修復 */
{
  char fpath[64];
  FILE *fp;

  sprintf(fpath, "%s.tmp", FN_RUN_RFORM);
  if (dashf(fpath))
  {
    vmsg("請先確定已無其他站長在審核註冊單，以免發生嚴重意外！");

    if (vans("確定要啟動註冊單修復功\能(Y/N)？[N] ") == 'y')
    {
      if (fp = fopen(FN_RUN_RFORM, "a"))
      {
	f_suck(fp, fpath);
	fclose(fp);
	unlink(fpath);
      }
      vmsg("處理完畢，以後請小心！");
    }
  }
  else
  {
    zmsg("目前並無修復註冊單之必要");
  }
  return XEASY;
}
#endif	/* HAVE_REGISTER_FORM */


/* ----------------------------------------------------- */
/* 寄信給全站使用者/板主				 */
/* ----------------------------------------------------- */


static void
add_to_list(list, id)
  char *list;
  char *id;		/* 未必 end with '\0' */
{
  char *i;

  /* 先檢查先前的 list 裡面是否已經有了，以免重覆加入 */
  for (i = list; *i; i += IDLEN + 1)
  {
    if (!strncmp(i, id, IDLEN))
      return;
  }

  /* 若之前的 list 沒有，那麼直接附加在 list 最後 */
  str_ncpy(i, id, IDLEN + 1);
}


static void
make_bm_list(list)
  char *list;
{
  BRD *head, *tail;
  char *ptr, *str, buf[BMLEN + 1];

  /* 去 bshm 中抓出所有 brd->BM */

  head = bshm->bcache;
  tail = head + bshm->number;
  do				/* 至少有 note 一板，不必對看板做檢查 */
  {
    ptr = buf;
    strcpy(ptr, head->BM);

    while (*ptr)	/* 把 brd->BM 中 bm1/bm2/bm3/... 各個 bm 抓出來 */
    {
      if (str = strchr(ptr, '/'))
	*str = '\0';
      add_to_list(list, ptr);
      if (!str)
	break;
      ptr = str + 1;
    }      
  } while (++head < tail);
}


static void
make_all_list(list)
  char *list;
{
  int fd;
  SCHEMA schema;

  if ((fd = open(FN_SCHEMA, O_RDONLY)) < 0)
    return;

  while (read(fd, &schema, sizeof(SCHEMA)) == sizeof(SCHEMA))
    add_to_list(list, schema.userid);

  close(fd);
}


static void
send_list(title, fpath, list)
  char *title;		/* 信件的標題 */
  char *fpath;		/* 信件的檔案 */
  char *list;		/* 寄信的名單 */
{
  char folder[64], *ptr;
  HDR mhdr;

  for (ptr = list; *ptr; ptr += IDLEN + 1)
  {
    usr_fpath(folder, ptr, fn_dir);
    if (hdr_stamp(folder, HDR_LINK, &mhdr, fpath) >= 0)
    {
      strcpy(mhdr.owner, str_sysop);
      strcpy(mhdr.title, title);
      mhdr.xmode = 0;
      rec_add(folder, &mhdr, sizeof(HDR));
    }
  }
}


static void
biff_bm()
{
  UTMP *utmp, *uceil;

  utmp = ushm->uslot;
  uceil = (void *) utmp + ushm->offset;
  do
  {
    if (utmp->pid && (utmp->userlevel & PERM_BM))
      utmp->status |= STATUS_BIFF;
  } while (++utmp <= uceil);
}


static void
biff_all()
{
  UTMP *utmp, *uceil;

  utmp = ushm->uslot;
  uceil = (void *) utmp + ushm->offset;
  do
  {
    if (utmp->pid)
      utmp->status |= STATUS_BIFF;
  } while (++utmp <= uceil);
}


int
m_bm()
{
  char *list, fpath[64];
  FILE *fp;
  int size;

  if (vans("要寄信給全站所有板主(Y/N)？[N] ") != 'y')
    return XEASY;

  strcpy(ve_title, "[板主通告] ");
  if (!vget(1, 0, "標題：", ve_title, TTLEN + 1, GCARRY))
    return 0;

  usr_fpath(fpath, cuser.userid, "sysmail");
  if (fp = fopen(fpath, "w"))
  {
    fprintf(fp, "※ [板主通告] 站長通告，收信人：各板主\n");
    fprintf(fp, "-------------------------------------------------------------------------\n");
    fclose(fp);
  }

  curredit = EDIT_MAIL;
  *quote_file = '\0';
  if (vedit(fpath, 1) >= 0)
  {
    vmsg("需要一段蠻長的時間，請耐心等待");

    size = (IDLEN + 1) * MAXBOARD * 4;	/* 假設每板四個板主已足夠 */
    if (list = (char *) malloc(size))
    {
      memset(list, 0, size);

      make_bm_list(list);
      send_list(ve_title, fpath, list);

      free(list);
      biff_bm();
    }
  }
  else
  {
    vmsg(msg_cancel);
  }

  unlink(fpath);

  return 0;
}


int
m_all()
{
  char *list, fpath[64];
  FILE *fp;
  int size;

  if (vans("要寄信給全站使用者(Y/N)？[N] ") != 'y')
    return XEASY;    

  strcpy(ve_title, "[系統通告] ");
  if (!vget(1, 0, "標題：", ve_title, TTLEN + 1, GCARRY))
    return 0;

  usr_fpath(fpath, cuser.userid, "sysmail");
  if (fp = fopen(fpath, "w"))
  {
    fprintf(fp, "※ [系統通告] 站長通告，收信人：全站使用者\n");
    fprintf(fp, "-------------------------------------------------------------------------\n");
    fclose(fp);
  }

  curredit = EDIT_MAIL;
  *quote_file = '\0';
  if (vedit(fpath, 1) >= 0)
  {
    vmsg("需要一段蠻長的時間，請耐心等待");

    size = (IDLEN + 1) * rec_num(FN_SCHEMA, sizeof(SCHEMA));
    if (list = (char *) malloc(size))
    {
      memset(list, 0, size);

      make_all_list(list);
      send_list(ve_title, fpath, list);

      free(list);
      biff_all();
    }
  }
  else
  {
    vmsg(msg_cancel);
  }

  unlink(fpath);

  return 0;
}
