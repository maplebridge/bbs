/*-------------------------------------------------------*/
/* xyz.c	( NTHU CS MapleBBS Ver 3.10 )		 */
/*-------------------------------------------------------*/
/* target : 雜七雜八的外掛				 */
/* create : 01/03/01					 */
/* update :   /  /  					 */
/*-------------------------------------------------------*/


#include "bbs.h"

extern UCACHE *ushm;
extern char * const compile_time;

#ifdef HAVE_TIP

/* ----------------------------------------------------- */
/* 每日小秘訣						 */
/* ----------------------------------------------------- */

int
x_tip()
{
  int i, j;
  char msg[128];
  FILE *fp;

  if (!(fp = fopen(FN_ETC_TIP, "r")))
    return XEASY;

  fgets(msg, 128, fp);
  j = atoi(msg);		/* 第一行記錄總篇數 */
  i = time(0) % j + 1;
  j = 0;

  while (j < i)			/* 取第 i 個 tip */
  {
    fgets(msg, 128, fp);
    if (msg[0] == '#')
      j++;
  }

  move(12, 0);
  clrtobot();
  fgets(msg, 128, fp);
  prints("\033[1;36m每日小祕訣：\033[m\n");
  prints("            %s", msg);
  fgets(msg, 128, fp);
  prints("            %s", msg);
  vmsg(NULL);
  fclose(fp);
  return 0;
}
#endif	/* HAVE_TIP */


#ifdef HAVE_LOVELETTER 

/* ----------------------------------------------------- */
/* 情書產生器						 */
/* ----------------------------------------------------- */

int
x_loveletter()
{
  FILE *fp;
  int start_show;	/* 1:開始秀 */
  int style;		/* 0:開頭 1:正文 2:結尾 */
  int line;
  char buf[128];
  char header[3][5] = {"head", "body", "foot"};	/* 開頭、正文、結尾 */
  int num[3];

  /* etc/loveletter 前段是#head 中段是#body 後段是#foot */
  /* 行數上限：#head五行  #body八行  #foot五行 */

  if (!(fp = fopen(FN_ETC_LOVELETTER, "r")))
    return XEASY;

  /* 前三行記錄篇數 */
  fgets(buf, 128, fp);
  num[0] = atoi(buf + 5);
  num[1] = atoi(buf + 5);
  num[2] = atoi(buf + 5);

  /* 決定要選第幾篇 */
  line = time(0);
  num[0] = line % num[0];
  num[1] = (line >> 1) % num[1];
  num[2] = (line >> 2) % num[2];

  vs_bar("情書產生器");

  start_show = style = line = 0;

  while (fgets(buf, 128, fp))
  {
    if (*buf == '#')
    {
      if (!strncmp(buf + 1, header[style], 4))  /* header[] 長度都是 5 bytes */
	num[style]--;

      if (num[style] < 0)	/* 已經 fget 到要選的這篇了 */
      {
	outc('\n');
	start_show = 1;
	style++;
      }
      else
      {
	start_show = 0;
      }
      continue;
    }

    if (start_show)
    {
      if (line >= (b_lines - 5))	/* 超過螢幕大小了 */
	break;

      outs(buf);
      line++;
    }
  }

  fclose(fp);
  vmsg(NULL);

  return 0;
}
#endif	/* HAVE_LOVELETTER */

/* ----------------------------------------------------- */
/* 顯示系統資訊						 */
/* ----------------------------------------------------- */

int
x_sysinfo()
{
  vs_bar("系統資訊");
  move(2, 0);
  prints("您現在位於 \033[1;37;41m【\033[1;33m " BBSNAME " \033[1;37;41m】\033[m (" MYIPADDR ")\n"
	 "線上服務人數: %d/%d\n"
	 "編譯時間:     %s\n",
	 ushm->count, MAXACTIVE,
	 compile_time);

  vmsg(NULL);
  return 0;
}

/* ----------------------------------------------------- */
/* 密碼忘記，重設密碼					 */
/* ----------------------------------------------------- */


int
x_password()
{
  int i;
  ACCT acct;
  FILE *fp;
  char fpath[80], email[60], passwd[PSWDLEN + 1];
  time_t now;

  vmsg("當其他使用者忘記密碼時，重送新密碼至該使用者的信箱");

  if (acct_get(msg_uid, &acct) > 0)
  {
    time(&now);

    if (acct.lastlogin > now - 86400 * 3)
    {
      vmsg("該使用者必須三天以上未上站方可重送密碼");
      return 0;
    }

    vget(b_lines - 2, 0, "請輸入認證時的 Email：", email, 40, DOECHO);

    if (str_cmp(acct.email, email))
    {
      vmsg("這不是該使用者認證時用的 Email");
      return 0;
    }

    if (not_addr(email) || !mail_external(email))
    {
      vmsg(err_email);
      return 0;
    }

    vget(b_lines - 1, 0, "請輸入真實姓名：", fpath, RNLEN + 1, DOECHO);
    if (strcmp(acct.realname, fpath))
    {
      vmsg("這不是該使用者的真實姓名");
      return 0;
    }

    if (vans("資料正確，請確認是否產生新密碼(Y/N)？[N] ") != 'y')
      return 0;

    sprintf(fpath, "%s 改了 %s 的密碼", cuser.userid, acct.userid);
    blog("PASSWD", fpath);

    /* 亂數產生 A~Z 組合的密碼八碼 */
    for (i = 0; i < PSWDLEN; i++)
      passwd[i] = rnd(26) + 'A';
    passwd[PSWDLEN] = '\0';

    /* 重新 acct_load 載入一次，避免對方在 vans() 時登入會有洗錢的效果 */
    if (acct_load(&acct, acct.userid) >= 0)
    {
      str_ncpy(acct.passwd, genpasswd(passwd), PASSLEN + 1);
      acct_save(&acct);
    }

    sprintf(fpath, "tmp/sendpass.%s", cuser.userid);
    if (fp = fopen(fpath, "w"))
    {
      fprintf(fp, "%s 為您申請了新密碼\n\n", cuser.userid);
      fprintf(fp, BBSNAME "ID : %s\n\n", acct.userid);
      fprintf(fp, BBSNAME "新密碼 : %s\n", passwd);
      fclose(fp);

      bsmtp(fpath, BBSNAME "新密碼", email, 0);
      unlink(fpath);

      vmsg("新密碼已寄到該認證信箱");
    }
  }

  return 0;
}


/* ----------------------------------------------------- */
/* 擲骰子程式						 */
/* ----------------------------------------------------- */

#if 0
  ryanlei.081123: 改寫，顯示方式可為：
    1)出現次數 2)逐次顯示 3)兩者皆要，
    並列出相關總和，平均，及其期望值作為參考。
#endif


int
post_sibala(xo)
  XO *xo;
{
  FILE *fp, *fp2, *fp3;
  HDR hdr;
  struct tm *ptime;
  time_t now;
  char fpath[64], buf[64], title[64], folder[64], FP3[64];
  char *line = "──────────────────\n";
  unsigned short i, j, k, num_sibala_face, num_sibala;
  unsigned short *count, mode;	/* ryanlei.081123: 各點數出現次數統計、顯示模式 */
  unsigned total = 0;		/* 點數總和 */
  float mean, Ex;		/* ryanlei.081123: 平均數、期望值 */

  for (j = 0; j != 'y';)
  {
    j = 0;
    num_sibala = 0;
    num_sibala_face = 0;

    clear();
    move(i = 3, 0);
    if (!vget(i, 0, "輸入標題名稱 : ", title, 60, DOECHO))
      return XO_HEAD;

    /* ryanlei.081123: 減輕 malloc 負荷，只允許兩位數骰子面數 */
    i += 2;
    if (!vget(i, 0, "輸入骰子面數 : ", buf, 3, DOECHO) || !(num_sibala_face = atoi(buf)))
      return XO_HEAD;

    i += 2;
    if (!vget(i, 0, "輸入丟骰次數 : ", buf, 5, DOECHO) || !(num_sibala = atoi(buf)))
      return XO_HEAD;

    /* ryanlei.081123: 讓使用者選擇顯示方式 */
    i += 2;
    mode = vget(i, 0, "顯示方式 : 1)出現次數 2)逐次結果 3)兩者皆要 [1] ", buf, 2, DOECHO) - '0';
    if (mode != 2 && mode != 3)
      mode = 1;		/* default: 1 */

    j = vans("Y)丟出 E)重來 Q)取消？[Q] ");

    if (!j || j == 'q')
      return XO_HEAD;
  }

  time(&now);
  ptime = localtime(&now);

  sprintf(fpath, "tmp/%s.sibala", cuser.userid);
  fp = fopen(fpath, "w");
  fprintf(fp, "丟骰人   : %s\n", cuser.userid);
  fprintf(fp, "來源     : %s\n", fromhost);
  fprintf(fp, "IP位址   : %s\n", get_my_ip());
  fprintf(fp, "丟骰時間 : %02d/%02d/%02d %02d:%02d:%02d\n", 
    ptime->tm_year % 100, ptime->tm_mon + 1, ptime->tm_mday, ptime->tm_hour, ptime->tm_min, ptime->tm_sec);
  fprintf(fp, "標題名稱 : %s\n", title);
  fprintf(fp, "丟骰面數 : %d\n", num_sibala_face);
  fprintf(fp, "丟骰次數 : %d\n", num_sibala);
  fprintf(fp, "顯示方式 : %s\n",
    mode == 2 ? "逐次結果" : mode == 3 ? "兩者皆要" : "出現次數");
  fprintf(fp, "\n丟骰結果 : \n%s", line);
  fclose(fp);

  count = malloc(sizeof(unsigned short) * num_sibala_face);
  memset(count, 0, sizeof(unsigned short) * num_sibala_face);	/* 初始化 */

  sprintf(FP3, "%s.tmp2", fpath);
  fp3 = fopen(FP3, "w");

  if (mode == 3)	/* 兩者皆要=>加分隔線 */
    fprintf(fp3, "%s", line);

  /* 丟骰子開始 */
  for (i = 0, srand(time(0)); i < num_sibala; i++)
  {
    k = rand() % num_sibala_face + 1;
    count[k - 1]++;
    total += k;
    if (mode != 1)
      fprintf(fp3, "第 %4d 次丟骰結果 : %4d\n", i + 1, k);
  }
  fclose(fp3);

  sprintf(folder, "%s.tmp1", fpath);	/* 借用 folder */
  if (mode != 2)
  {
    fp2 = fopen(folder, "w");
    fprintf(fp2, "    點數      出現次數      比例\n");
    for (i = 0; i < num_sibala_face; i++)
    {
      if (count[i] != 0)	/* 出現1次以上才顯示 */
      {
	fprintf(fp2, "    %4d      %4d       %6.3f%%\n",
	  i + 1, count[i], (float)count[i] / (float)num_sibala * 100.f); 
      }
    }
    fprintf(fp2, "%s平均期望次數: %7.2f    %6.3f%%\n",
      line, (float)num_sibala / (float)num_sibala_face, 100.f / (float)num_sibala_face);
    fclose(fp2);
  }
  free(count);

  if (mode != 2)
  {
    sprintf(buf, "/bin/cat %s >> %s", folder, fpath);
    system(buf);
  }
  if (mode != 1)
  {
    sprintf(buf, "/bin/cat %s >> %s", FP3, fpath);
    system(buf);
  }
  unlink(folder);
  unlink(FP3);

  /* 計算平均和期望值 */
  mean = (float)total / (float)num_sibala;
  Ex = (float)(num_sibala_face + 1) / 2.f;
  fp = fopen(fpath, "a");	/* 再開一次檔 */
  fprintf(fp, "\n點數總和 : %6d     期望值 : %9.2f\n", total, Ex * num_sibala);
  fprintf(fp, "點數平均 : %6.2f     期望值 : %9.2f\n", mean, Ex);
  fprintf(fp, "\n--\n※ 楓橋驛站 - 擲骰系統\n");
  fclose(fp);

  if (!(bbstate & STAT_POST))	/* 在 currboard 沒有發表權限，故寄回信箱 */
  {
    usr_fpath(folder, cuser.userid, fn_dir);
    hdr_stamp(folder, HDR_COPY, &hdr, fpath);
    strcpy(hdr.owner, "丟骰結果");
    strcpy(hdr.nick, "賭神");
    hdr.xmode = MAIL_READ | MAIL_NOREPLY;
    sprintf(hdr.title, "%s : %s", cuser.userid, title);
    rec_add(folder, &hdr, sizeof(HDR));
    vmsg("您沒有在此發表文章的權限，丟骰結果已寄回您的信箱");
  }
  else		/* 貼回原看板上 */
  {
    hdr_stamp(xo->dir, HDR_COPY | 'A', &hdr, fpath);
    strcpy(hdr.owner, "丟骰結果");
    strcpy(hdr.nick, "賭神");
    sprintf(hdr.title, "%s : %s", cuser.userid, title);
    rec_bot(xo->dir, &hdr, sizeof(HDR));
    btime_update(brd_bno(currboard));
  }

  /* 貼至 log 板做記錄 */
  brd_fpath(folder, BN_SECURITY, FN_DIR);

  hdr_stamp(folder, HDR_COPY | 'A', &hdr, fpath);
  strcpy(hdr.owner, "丟骰結果");
  strcpy(hdr.nick, "賭神");
  sprintf(hdr.title, "%s : %s", cuser.userid, title);
  rec_bot(folder, &hdr, sizeof(HDR));
  btime_update(brd_bno(BN_SECURITY));

  unlink(fpath);

  vmsg("丟骰完成！");

  return XO_INIT;
}
