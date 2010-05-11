/*-------------------------------------------------------*/
/* admutil.c	( NTHU CS MapleBBS Ver 3.10 )		 */
/*-------------------------------------------------------*/
/* target : �������O					 */
/* create : 95/03/29					 */
/* update : 01/03/01					 */
/*-------------------------------------------------------*/


#include "bbs.h"


extern BCACHE *bshm;
extern UCACHE *ushm;


/* ----------------------------------------------------- */
/* ���ȫ��O						 */
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


#define	IAS_Can_Delete
#undef	IAS_Can_Delete

/* smiler.090311: ���������]�i�ۦ浹�� id �֧Q */
int
a_ias_bank()
{
  int ans, ch, fd;
  char fpath[80], reason[50], buf[512];
  ACCT acct;

  move(1, 0);
  clrtobot();

  while (ans = acct_get(msg_uid, &acct))
  {
    if (ans > 0)
    {
      if (!vget(b_lines, 0, "�п�J�[��/�]�w��ƪ��z�ѡG", reason, 50, DOECHO))
      {
        alog("IAS_Bank_�[�ݸ��", acct.userid);
        return;
      }
      else
      {
        sprintf(buf, "%s �z�ѡG%s", acct.userid, reason);
        alog("IAS_Bank_�]�w���", buf);
      }

      sprintf(reason, "�ä[�O�d�b�� : %s", (acct.userlevel & PERM_XEMPT) ? "��" : "�L");
      sprintf(buf, "%s �]�w�e  %s", acct.userid, reason);
      alog("IAS_Bank_�]�w���", buf);

      sprintf(reason, "�ä[�K�{��   : %s", (acct.userlevel & PERM_XVALID) ? "��" : "�L");
      sprintf(buf, "%s �]�w�e  %s", acct.userid, reason);
      alog("IAS_Bank_�]�w���", buf);

      usr_fpath(fpath, acct.userid, fn_acct);
      fd = open(fpath, O_RDWR);
      if (fd > 0)
      {
	ch = 1;
#ifdef	IAS_Can_Delete
	while (ch >= 1 && ch <= 4)
#else
	while (ch >= 1 && ch <= 2)
#endif
	{
	  move(5, 0);

	  prints("�b�� : \033[1;33m%s\033[m\n\n", acct.userid);
	  prints("�ä[�O�d�b�� : %s\n\n", (acct.userlevel & PERM_XEMPT) ? "\033[1;33m��\033[m" : "\033[1;30m�L\033[m");
	  prints("�ä[�K�{��   : %s\n", (acct.userlevel & PERM_XVALID) ? "\033[1;33m��\033[m" : "\033[1;30m�L\033[m");

#ifdef	IAS_Can_Delete
	  switch(ch = vans("�� �W�R�֧Q 1)�W�ëO 2)�W�çK 3)�R�ëO 4)�R�çK�G[Q] ") - '0')
#else
	  switch(ch = vans("�� �����֧Q 1)�W�ëO 2)�W�çK�G[Q] ") - '0')
#endif
	  {
	    case 1:
	      acct.userlevel |= PERM_XEMPT;
	      break;
	    case 2:
	      acct.userlevel |= PERM_XVALID;
	      break;
#ifdef	IAS_Can_Delete
	    case 3:
	      acct.userlevel &= (~PERM_XEMPT);
	      break;
	    case 4:
	      acct.userlevel &= (~PERM_XVALID);
	      break;
#endif
	    default:
	      ch = 0;
	      break;
	  }
	}

	ch = vans("�� Y)�T�w N)�����G[N] ");

	if (ch == 'y')
	{
	  lseek(fd, (off_t) 0, SEEK_SET);
	  write(fd, &acct, sizeof(ACCT));
	  close(fd);
	  vmsg("�]�w�����A�N��ϥΪ̤U���W���ɦ۰ʥͮ� !!");
	}
	else
	  vmsg("������� !!");
      }

      sprintf(reason, "�ä[�O�d�b�� : %s", (acct.userlevel & PERM_XEMPT) ? "��" : "�L");
      sprintf(buf, "%s �]�w�e  %s", acct.userid, reason);
      alog("IAS_Bank_�]�w���", buf);

      sprintf(reason, "�ä[�K�{��   : %s", (acct.userlevel & PERM_XVALID) ? "��" : "�L");
      sprintf(buf, "%s �]�w�e  %s", acct.userid, reason);
      alog("IAS_Bank_�]�w���", buf);
    }

    move(1, 0);
    clrtobot();
  }

  return 0;
}


/* smiler.071110: ���Ȩt�γ]�w:����]�w */
int
a_system_setup()
{
  FILE *fp;
  char buf[64];
  char set_path[64], set_tmp[64], tmp_space[64];
  int i, ans;

  int host_sight_number_tmp = 0;	/* ��ñ�ӼƳ]�w */
  int host_sight_select_tmp = 0;	/* ��ñ�������w */
  int model_number_tmp = 0;		/* ��ñ���ӼƳ]�w */
  int model_select_tmp = 0;		/* ��ñ���ϥΥ������w */
  int editlog_use_tmp = 0;		/* �O�_�ҥ�Editlog�\��? */
  int deletelog_use_tmp = 0;		/* �O�_�ҥ�Deletelog�\��? */
  int mail_to_newone_tmp = 0;		/* �O�_�H�H���s�ӽЪ�? */

  move(i = 1, 0);
  clrtobot();

#ifdef HAVE_MULTI_SIGN
  /* ��ñ�ӼƳ]�w */  
  sprintf(buf, "%d", host_sight_number);
  vget(++i, 0, "��ñ�ӼƳ]�w�G", buf, 10, GCARRY);
  host_sight_number_tmp = atoi(buf);
  if (host_sight_number_tmp <= 0)
  {
    vmsg("�̤j��ñ�Ƥ��o�p�󵥩� 0, ���ƥج� 1 ");
    host_sight_number_tmp = 1;
  }
  sprintf(tmp_space, "gem/@/@host_%d", host_sight_number_tmp - 1);
  if (!dashf(tmp_space))
  {
    vmsg("�̤j��ñ�ƥؤ��s�b, ���ƥج� 1 ");
    host_sight_number_tmp = 1;
  }

  /* ��ñ�������w */
  sprintf(buf, "%d", host_sight_select);
  vget(++i, 0, "��ñ�������w(�� 0 �������w)�G", buf, 10, GCARRY);
  host_sight_select_tmp = atoi(buf);
  if(host_sight_select_tmp != 0)
  {
    if (host_sight_select_tmp < 0)
    {
      vmsg("���w��ñ�Ƥ��o�p�󵥩� 0, �����w�� 1 ");
      host_sight_select_tmp = 1;
    }
    sprintf(tmp_space, "gem/@/@host_%d", host_sight_select_tmp - 1);
    if (!dashf(tmp_space))
    {
      vmsg("���w��ñ���s�b, �����w�� 1 ");
      host_sight_select_tmp = 1;
    }
  }

  /* ��ñ���ӼƳ]�w */
  sprintf(buf, "%d", model_number);
  vget(++i, 0, "��ñ���ӼƳ]�w�G", buf, 10, GCARRY);
  model_number_tmp = atoi(buf);
  if (model_number_tmp <= 0)
  {
    vmsg("��ñ���ӼƤ��o�p�󵥩� 0, ���ƥج� 1 ");
    model_number_tmp = 1;
  }
  sprintf(tmp_space, "gem/@/@model_%d", model_number_tmp - 1);
  if (!dashf(tmp_space))
  {
    vmsg("�̤j��ñ���ӼƤ��s�b, ���ƥج� 1 ");
    model_number_tmp = 1;
  }

  /* ��ñ���������w */
  sprintf(buf, "%d", model_select);
  vget(++i, 0, "��ñ���ϥΥ������w(�� 0 �������w)�G", buf, 10, GCARRY);
  model_select_tmp=atoi(buf);
  if (model_select_tmp !=0)
  {
    if (model_select_tmp < 0)
    {
      vmsg("��ñ�����w�s�����o�p�󵥩� 0, �����w�� 1 ");
      model_select_tmp = 1;
    }
    sprintf(tmp_space,"gem/@/@model_%d",model_select_tmp-1);
    if (!dashf(tmp_space))
    {
      vmsg("���w��ñ�����s�b, �����w�� 1 ");
      model_select_tmp = 1;
    }
  }
#endif

  /* �ҥ� Editlog �\�� */
  sprintf(buf, "%d", editlog_use);
  vget(++i, 0, "�O�_�ҥ� Editlog �\\��? (�O:1 �_:0)�G", buf, 10, GCARRY);
  if (*buf == '1' || *buf == '0')
    editlog_use_tmp=atoi(buf);
  else
    editlog_use_tmp=0;

  sprintf(buf, "%d", deletelog_use);
  vget(++i, 0, "�O�_�ҥ� Deletelog �\\��? (�O:1 �_:0)�G", buf, 10, GCARRY);
  if (*buf == '1' || *buf == '0')
    deletelog_use_tmp = atoi(buf);
  else
    deletelog_use_tmp = 0;

  sprintf(buf, "%d", mail_to_newone);
  vget(++i, 0, "�O�_�H�H���s�ӽЪ�? (�O:1 �_:0)�G", buf, 10, GCARRY);
  if(*buf=='1' || *buf=='0')
    mail_to_newone_tmp = atoi(buf);
  else
    mail_to_newone=0;

  ans = vans("�]�w 1)�T�w Q)���� [Q] ");
  {
    if (ans == '1')
    {
      sprintf(set_path, ".SET");
      sprintf(set_tmp, ".SET.TMP");

      /* �N�W�z���ק�g�^ .USR �H�ι�������P�B��s */
      if (fp = fopen(set_tmp, "w"))
      {
	host_sight_number=host_sight_number_tmp;
	fprintf(fp, "%d\n", host_sight_number_tmp);

	host_sight_select=host_sight_select_tmp;
	fprintf(fp, "%d\n", host_sight_select_tmp);

	model_number=model_number_tmp;
	fprintf(fp, "%d\n", model_number_tmp);

	model_select=model_select_tmp;
	fprintf(fp, "%d\n", model_select_tmp);

	editlog_use=editlog_use_tmp;
	fprintf(fp, "%d\n", editlog_use_tmp);

	deletelog_use=deletelog_use_tmp;
	fprintf(fp, "%d\n", deletelog_use_tmp);

	mail_to_newone=mail_to_newone_tmp;
	fprintf(fp, "%d\n", mail_to_newone_tmp);

	fclose(fp);
      }
      unlink(set_path);              /* smiler.071110: �W�z�]�w�O�s�@���� ~/.SET */
      rename(set_tmp, set_path);     /* �קﯸ�ȳ]�w�]�w�ﶵ��,�O�o�n���K�� bbsd.c ���� setuploader() */
    }
  }
  return 0;
}


int
a_search()	/* itoc.010902: �ɤO�j�M�ϥΪ� */
{
  ACCT acct;
  char c;
  char key[30];

  if (!vget(b_lines, 0, "�п�J����r(�m�W/�ʺ�/�ӷ�/�H�c)�G", key, 30, DOECHO))
    return XEASY;  

  /* itoc.010929.����: �u�O�����ɤO :p �Ҽ{���� reaper ���X�@�� .PASSWDS �A�h�� */

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

	if (vans("�O�_�~��j�M�U�@���H[N] ") != 'y')
	{
	  closedir(dirp);
 	  goto end_search;
 	}
      }
    }
    closedir(dirp);
  }
end_search:
  vmsg("�j�M����");
  return 0;
}


int
a_editbrd()		/* itoc.010929: �ק�ݪO�ﶵ */
{
  int bno;
  BRD *brd;
  char bname[BNLEN + 1];

  if (brd = ask_board(bname, BRD_L_BIT, NULL))	/* �u�n���Ȭݱo��(�Y�϶i���h)���O, ���i�H�� */
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
a_xfile()		/* �]�w�t���ɮ� */
{
  static char *desc[] =
  {
    "�ݪO�峹����",

    "�����{�ҫH��",
    "�{�ҳq�L�q��",
    "���s�{�ҳq��",

#ifdef HAVE_DETECT_CROSSPOST
    "��K���v�q��",
#endif

    "�����W��",
    "���ȦW��",

    "�`��",

#ifdef HAVE_WHERE
    "�G�m IP",
    "�G�m FQDN",
#endif

#ifdef HAVE_TIP
    "�C��p���Z",
#endif

#ifdef HAVE_LOVELETTER
    "���Ѳ��;���w",
#endif

    "�{�ҥզW��",
    "�{�Ҷ¦W��",

    "���H�զW��",
    "���H�¦W��",

#ifdef  HAVE_DETECT_CROSSPOST
    "�iCrosspost",
#endif

#ifdef HAVE_LOGIN_DENIED
    "�����s�u�W��",    /* smiler.070724 */
    "�ڵ��s�u�W��",
#endif
    "���Ȫ�����",
    "BBS �ݪ���",
    "�ݪ����O��",
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
    FN_ETC_BBSDOG_LOG,
    NULL
  };

  x_file(M_XFILES, desc, path);
  return 0;
}


int
a_resetsys()		/* ���m */
{
  switch (vans("�� �t�έ��] 1)�ʺA�ݪO 2)�����s�� 3)���W�ξ׫H 4)�����G[Q] "))
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
/* �٭�ƥ���						 */
/* ----------------------------------------------------- */


static void
show_availability(type)		/* �N BAKPATH �̭��Ҧ��i���^�ƥ����ؿ��L�X�� */
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
    fputs("�� �i�Ѩ��^���ƥ����G\n\n", fp);

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

  ch = vans("�� �٭�ƥ� 1)�ݪO 2)��ذ� 3)�ϥΪ̡G[Q] ") - '1';
  if (ch < 0 || ch >= 3)
    return XEASY;

  type = tpool[ch];
  show_availability(type);

  if (vget(b_lines, 0, "�n���^���ƥ��ؿ��G", date, 20, DOECHO))
  {
    /* �קK�������F�@�Ӧs�b���ؿ��A���O�M type ���X */
    if (strncmp(date, type, strlen(type)))
      return 0;

    sprintf(src, BAKPATH"/%s", date);
    if (!dashd(src))
      return 0;
    ptr = strchr(src, '\0');

    clear();
    move(3, 0);
    outs("���٭�ƥ����ݪO/�ϥΪ̥����w�s�b�C\n"
      "�Y�ӬݪO/�ϥΪ̤w�R���A�Х����s�}�]/���U�@�ӦP�W���ݪO/�ϥΪ̡C\n"
      "�٭�ƥ��ɽнT�{�ӬݪO�L�H�ϥ�/�ϥΪ̤��b�u�W");

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
      /* �ɮפ��s�b�A�q�`�O�]���ƥ��I�ɸӬݪO/�ϥΪ̤w�Q�R���A�άO���ɮڥ��N�٨S���ӬݪO/�ϥΪ� */
      vmsg("�ƥ��ɮפ��s�b�A�иոը�L�ɶ��I���ƥ�");
      return 0;
    }

    if (vans("�٭�ƥ���A�ثe�Ҧ���Ƴ��|�y���A�аȥ��T�w(Y/N)�H[N] ") != 'y')
      return 0;

    alog("�٭�ƥ�", src);

    /* �����Y */
    if (ch == 0)
      ptr = "brd";
    else if (ch == 1)
      ptr = "gem/brd";
    else /* if (ch == 2) */
      sprintf(ptr = date, "usr/%c", *type);
    sprintf(cmd, "tar xfz %s -C %s/", src, ptr);
    /* system(cmd); */

#if 1	/* ��������ʰ��� */
    move(7, 0);
    outs("\n�ХH bbs �����n�J�u�@���A�é�\033[1;36m�a�ؿ�\033[m����\n\n\033[1;33m");
    outs(cmd);
    outs("\033[m\n\n");
#endif

    /* tar ���H��A�٭n������ */
    if (vans("�� ���O Y)�w���\\����H�W���O Q)������G[Q] ") == 'y')
    {
      if (ch == 0)	/* �٭�ݪO�ɡA�n��s�O�� */
      {
	if ((ch = brd_bno(brdname)) >= 0)
	{
	  brd_fpath(src, brdname, fn_pal);
	  bpal = bshm->pcache + ch;
	  bpal->pal_max = image_pal(src, bpal->pal_spool);
	}
      }
      else if (ch == 2)	/* �٭�ϥΪ̮ɡA���٭� userno */
      {
	ch = acct.userno;
	if (acct_load(&acct, type) >= 0)
	{
	  acct.userno = ch;
	  acct_save(&acct);
	}
      }
      vmsg("�٭�ƥ����\\");
      return 0;
    }
  }

  vmsg(msg_cancel);
  return 0;
}


#ifdef HAVE_REGISTER_FORM

/* ----------------------------------------------------- */
/* �B�z Register Form					 */
/* ----------------------------------------------------- */


static int
scan_register_form(fd)
  int fd;
{
  static char logfile[] = FN_RUN_RFORM_LOG;
  static char *reason[] = 
  {
    "��J�u��m�W", "�Թ��g�ӽЪ�", "�Զ���}���", "�Զ�s���q��", 
    "�Զ�A�ȳ��B�ξǮըt��", "�Τ����g�ӽг�", 
#ifdef EMAIL_JUSTIFY	/* waynesan.040327: �� E-mail �{�Ҥ~������ */
    "�ĥ� E-mail �{��", 
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

  vs_bar("�f�֨ϥΪ̵��U���");
  agent = cuser.userid;

  while (read(fd, &rform, sizeof(RFORM)) == sizeof(RFORM))
  {
    userid = rform.userid;
    move(2, 0);
    prints("�ӽХN��: %s (�ӽЮɶ��G%s)\n", userid, Btime(&rform.rtime));
    prints("�u��m�W: %s\n", rform.realname);
    prints("�A�ȳ��: %s\n", rform.career);
    prints("�ثe���}: %s\n", rform.address);
    prints("�s���q��: %s\n%s\n", rform.phone, msg_seperator);
    clrtobot();

    if ((acct_load(&acct, userid) < 0) || (acct.userno != rform.userno))
    {
      vmsg("�d�L���H");
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
	vmsg("���b���w�g�������U");
	op = 'd';
      }
      else if (acct.userlevel & PERM_ALLDENY)
      {
	/* itoc.050405: ���������v�̭��s�{�ҡA�]���|�ﱼ�L�� tvalid (���v����ɶ�) */
	vmsg("���b���ثe�Q���v��");
	op = 'd';
      }
      else
      {
	op = vans("�O�_����(Y/N/Q/Del/Skip)�H[S] ");
      }
    }

    switch (op)
    {
    case 'y':

      /* �����v�� */
      sprintf(buf, "REG: %s:%s:%s:by %s", rform.phone, rform.career, rform.address, agent);
      justify_log(acct.userid, buf);
      strcpy(acct.realname, rform.realname);
      time4(&acct.tvalid);
      /* itoc.041025: �o�� acct_setperm() �èS�����b acct_load() �᭱�A�����j�F�@�� vans()�A
         �o�i��y������ acct �h�л\�s .ACCT �����D�C���L�]���O�����~�����v���A�ҥH�N����F */
      acct_setperm(&acct, PERM_VALID, 0);
      utmp_admset(acct.userno, STATUS_DATALOCK);	/* ��w��ơA�H����u�W�ϥΪ̻{�ҫ�U�u�A�m�W�S�Q�g�^ */

      /* �H�H�q���ϥΪ� */
      usr_fpath(folder, userid, fn_dir);
      hdr_stamp(folder, HDR_LINK, &hdr, FN_ETC_JUSTIFIED);
      strcpy(hdr.title, msg_reg_valid);
      strcpy(hdr.owner, str_sysop);
      rec_add(folder, &hdr, sizeof(HDR));

      strcpy(rform.agent, agent);
      rec_add(logfile, &rform, sizeof(RFORM));

      m_biff(rform.userno);

      break;

    case 'q':			/* �Ӳ֤F�A������ */

      do
      {
	rec_add(FN_RUN_RFORM, &rform, sizeof(RFORM));
      } while (read(fd, &rform, sizeof(RFORM)) == sizeof(RFORM));

    case 'd':
      break;

    case 'n':

      move(9, 0);
      prints("�д��X�h�^�ӽЪ���]�A�� <enter> ����\n\n");
      for (n = 0; str = reason[n]; n++)
	prints("%d) ��%s\n", n, str);
      clrtobot();

      if (op = vget(b_lines, 0, "�h�^��]�G", buf, 60, DOECHO))
      {
	int i;

	i = op - '0';
	if (i >= 0 && i < n)
	  strcpy(buf, reason[i]);

	usr_fpath(folder, acct.userid, fn_dir);
	if (fout = fdopen(hdr_stamp(folder, 0, &hdr, fpath), "w"))
	{
	  fprintf(fout, "\t�ѩ�z���Ѫ���Ƥ����Թ�A�L�k�T�{�����A"
	    "\n\n\t�Э��s��g���U����G%s�C\n", buf);
	  fclose(fout);

	  strcpy(hdr.owner, agent);
	  strcpy(hdr.title, "[�h��] �бz���s��g���U����");
	  rec_add(folder, &hdr, sizeof(HDR));
	}

	strcpy(rform.reply, buf);	/* �z�� */
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
  FILE *fp;
  int num;
  char buf[80], tmp[80];

  num = rec_num(FN_RUN_RFORM, sizeof(RFORM));
  if (num <= 0)
  {
    zmsg("�ثe�õL�s���U���");
    return XEASY;
  }

  sprintf(buf, "�@�� %d ����ơA�}�l�f�ֶ�(Y/N)�H[N] ", num);
  num = XEASY;

  if (vans(buf) == 'y')
  {
    sprintf(buf, "%s.tmp", FN_RUN_RFORM);
    sprintf(tmp, "%s.sysop", FN_RUN_RFORM);
    if (dashf(buf))
    {
      if (fp = fopen(tmp, "r"))
      {
	fgets(tmp, sizeof(tmp), fp);
	sprintf(buf, "���� %s ���b�f�ֵ��U�ӽг�", tmp);
	vmsg(buf);
      }
      else
	vmsg("��L SYSOP �]�b�f�ֵ��U�ӽг�");
    }
    else
    {
      int fd;

      if (fp = fopen(tmp, "w"))
      {
	fputs(cuser.userid, fp);
	fclose(fp);
      }

      rename(FN_RUN_RFORM, buf);
      fd = open(buf, O_RDONLY);
      if (fd >= 0)
      {
	scan_register_form(fd);
	close(fd);
	unlink(buf);
	unlink(tmp);
	num = 0;
      }
      else
      {
	vmsg("�L�k�}�ҵ��U��Ƥu�@��");
      }
    }
  }
  return num;
}


int
a_regmerge()			/* itoc.000516: �_�u�ɵ��U��״_ */
{
  char fpath[64];
  FILE *fp;

  sprintf(fpath, "%s.tmp", FN_RUN_RFORM);
  if (dashf(fpath))
  {
    vmsg("�Х��T�w�w�L��L�����b�f�ֵ��U��A�H�K�o���Y���N�~�I");

    if (vans("�T�w�n�Ұʵ��U��״_�\\��(Y/N)�H[N] ") == 'y')
    {
      if (fp = fopen(FN_RUN_RFORM, "a"))
      {
	f_suck(fp, fpath);
	fclose(fp);
	unlink(fpath);
	sprintf(fpath, "%s.sysop", FN_RUN_RFORM);
	unlink(fpath);
      }
      vmsg("�B�z�����A�H��Фp�ߡI");
    }
  }
  else
  {
    zmsg("�ثe�õL�״_���U�椧���n");
  }
  return XEASY;
}
#endif	/* HAVE_REGISTER_FORM */


/* ----------------------------------------------------- */
/* �H�H�������ϥΪ�/�O�D				 */
/* ----------------------------------------------------- */


static void
add_to_list(list, id)
  char *list;
  char *id;		/* ���� end with '\0' */
{
  char *i;

  /* ���ˬd���e�� list �̭��O�_�w�g���F�A�H�K���Х[�J */
  for (i = list; *i; i += IDLEN + 1)
  {
    if (!strncmp(i, id, IDLEN))
      return;
  }

  /* �Y���e�� list �S���A���򪽱����[�b list �̫� */
  str_ncpy(i, id, IDLEN + 1);
}


static void
make_bm_list(list)
  char *list;
{
  BRD *head, *tail;
  char *ptr, *str, buf[BMLEN + 1];

  /* �h bshm ����X�Ҧ� brd->BM */

  head = bshm->bcache;
  tail = head + bshm->number;
  do				/* �ܤ֦� note �@�O�A������ݪO���ˬd */
  {
    ptr = buf;
    strcpy(ptr, head->BM);

    while (*ptr)	/* �� brd->BM �� bm1/bm2/bm3/... �U�� bm ��X�� */
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
  char *title;		/* �H�󪺼��D */
  char *fpath;		/* �H���ɮ� */
  char *list;		/* �H�H���W�� */
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

  if (vans("�n�H�H�������Ҧ��O�D(Y/N)�H[N] ") != 'y')
    return XEASY;

  strcpy(ve_title, "[�O�D�q�i] ");
  if (!vget(1, 0, "���D�G", ve_title, TTLEN + 1, GCARRY))
    return 0;

  usr_fpath(fpath, cuser.userid, "sysmail");
  if (fp = fopen(fpath, "w"))
  {
    fprintf(fp, "�� [�O�D�q�i] �����q�i�A���H�H�G�U�O�D\n");
    fprintf(fp, "-------------------------------------------------------------------------\n");
    fclose(fp);
  }

  curredit = EDIT_MAIL;
  *quote_file = '\0';
  if (vedit(fpath, 1) >= 0)
  {
    vmsg("�ݭn�@�q�������ɶ��A�Э@�ߵ���");

    size = (IDLEN + 1) * MAXBOARD * 4;	/* ���]�C�O�|�ӪO�D�w���� */
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

  if (vans("�n�H�H�������ϥΪ�(Y/N)�H[N] ") != 'y')
    return XEASY;    

  strcpy(ve_title, "[�t�γq�i] ");
  if (!vget(1, 0, "���D�G", ve_title, TTLEN + 1, GCARRY))
    return 0;

  usr_fpath(fpath, cuser.userid, "sysmail");
  if (fp = fopen(fpath, "w"))
  {
    fprintf(fp, "�� [�t�γq�i] �����q�i�A���H�H�G�����ϥΪ�\n");
    fprintf(fp, "-------------------------------------------------------------------------\n");
    fclose(fp);
  }

  curredit = EDIT_MAIL;
  *quote_file = '\0';
  if (vedit(fpath, 1) >= 0)
  {
    vmsg("�ݭn�@�q�������ɶ��A�Э@�ߵ���");

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