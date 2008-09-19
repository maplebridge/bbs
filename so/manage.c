/*-------------------------------------------------------*/
/* manage.c	( NTHU CS MapleBBS Ver 3.10 )		 */
/*-------------------------------------------------------*/
/* target : 看板管理				 	 */
/* create : 95/03/29				 	 */
/* update : 96/04/05				 	 */
/*-------------------------------------------------------*/


#include "bbs.h"


extern BCACHE *bshm;


#ifdef HAVE_TERMINATOR
/* ----------------------------------------------------- */
/* 站長功能 : 拂楓落葉斬				 */
/* ----------------------------------------------------- */


extern char xo_pool[];


#define MSG_TERMINATOR	"《拂楓落葉斬》"

int
post_terminator(xo)		/* Thor.980521: 終極文章刪除大法 */
  XO *xo;
{
  int mode, type;
  HDR *hdr;
  char keyOwner[80], keyTitle[TTLEN + 1], buf[80];


  /* smiler.1111 在Deletelog 以及 Editlog 板 不接受 post_terminator(xo) */
 if((!strcmp(xo->dir,"brd/Deletelog/.DIR")) || (!strcmp(xo->dir,"brd/Editlog/.DIR")))
   return XO_NONE;


  if (!HAS_PERM(PERM_ALLBOARD))
    return XO_FOOT;

  mode = vans(MSG_TERMINATOR "刪除 (1)本文作者 (2)本文標題 (3)自定？[Q] ") - '0';

  if (mode == 1)
  {
    hdr = (HDR *) xo_pool + (xo->pos - xo->top);
    strcpy(keyOwner, hdr->owner);
  }
  else if (mode == 2)
  {
    hdr = (HDR *) xo_pool + (xo->pos - xo->top);
    strcpy(keyTitle, str_ttl(hdr->title));		/* 拿掉 Re: */
  }
  else if (mode == 3)
  {
    if (!vget(b_lines, 0, "作者：", keyOwner, 73, DOECHO))
      mode ^= 1;
    if (!vget(b_lines, 0, "標題：", keyTitle, TTLEN + 1, DOECHO))
      mode ^= 2;
  }
  else
  {
    return XO_FOOT;
  }

  type = vans(MSG_TERMINATOR "刪除 (1)轉信板 (2)非轉信板 (3)所有看板？[Q] ");
  if (type < '1' || type > '3')
    return XO_FOOT;

  sprintf(buf, "刪除%s：%.35s 於%s板，確定嗎(Y/N)？[N] ", 
    mode == 1 ? "作者" : mode == 2 ? "標題" : "條件", 
    mode == 1 ? keyOwner : mode == 2 ? keyTitle : "自定", 
    type == '1' ? "轉信" : type == '2' ? "非轉信" : "所有看");

  if (vans(buf) == 'y')
  {
    BRD *bhdr, *head, *tail;
    char tmpboard[BNLEN + 1];

    /* Thor.980616: 記下 currboard，以便復原 */
    strcpy(tmpboard, currboard);

    head = bhdr = bshm->bcache;
    tail = bhdr + bshm->number;
    do				/* 至少有 note 一板 */
    {
      int fdr, fsize, xmode;
      FILE *fpw;
      char fpath[64], fnew[64], fold[64];
      HDR *hdr;

      char Deletelog_folder[64],Deletelog_title[64],copied[64]; /* smiler.1111: 若有啟動editlog_use deletelog_use */
      HDR  Deletelog_hdr;	/* 則將被砍的資料移至 Deletelog 板備份 */

      xmode = head->battr;
      if ((type == '1' && (xmode & BRD_NOTRAN)) || (type == '2' && !(xmode & BRD_NOTRAN)))
	continue;

      /* smiler.1111: 保護Editlog Deletelog此兩記錄看板不被刪除資料 */
      if ((!strcmp(head->brdname,"Deletelog")) || (!strcmp(head->brdname,"Editlog")))
	continue;

      /* Thor.980616: 更改 currboard，以 cancel post */
      strcpy(currboard, head->brdname);

      sprintf(buf, MSG_TERMINATOR "看板：%s \033[5m...\033[m", currboard);
      outz(buf);
      refresh();

      brd_fpath(fpath, currboard, fn_dir);

      if ((fdr = open(fpath, O_RDONLY)) < 0)
	continue;

      if (!(fpw = f_new(fpath, fnew)))
      {
	close(fdr);
	continue;
      }

      fsize = 0;
      mgets(-1);
      while (hdr = mread(fdr, sizeof(HDR)))
      {
	xmode = hdr->xmode;

	if ((xmode & POST_MARKED) || 
	  ((mode & 1) && strcmp(keyOwner, hdr->owner)) ||
	  ((mode & 2) && strcmp(keyTitle, str_ttl(hdr->title))))
	{
	  if ((fwrite(hdr, sizeof(HDR), 1, fpw) != 1))
	  {
	    fclose(fpw);
	    close(fdr);
	    goto contWhileOuter;
	  }
	  fsize++;
	}
	else
	{
	  /* 砍文並連線砍信 */

	  cancel_post(hdr);

	  /* smiler.1111: 若deletelog_use有啟動,則將這筆被刪的資料備份至Deletelog板 */
      if(deletelog_use)
	  {
         hdr_fpath(copied, fpath, hdr);
         brd_fpath(Deletelog_folder,"Deletelog", FN_DIR);
         hdr_stamp(Deletelog_folder, HDR_COPY | 'A', &Deletelog_hdr, copied);
         strcpy(Deletelog_hdr.title , hdr->title);
         strcpy(Deletelog_hdr.owner , cuser.userid);
         strcpy(Deletelog_hdr.nick  , cuser.username);
         Deletelog_hdr.xmode = POST_OUTGO;
         rec_bot(Deletelog_folder, &Deletelog_hdr, sizeof(HDR));
         btime_update(brd_bno("Deletelog"));
	  }

	  hdr_fpath(fold, fpath, hdr);
	  unlink(fold);
      if (xmode & POST_RESTRICT)
        RefusePal_kill(currboard, hdr);

	}
      }
      close(fdr);
      fclose(fpw);

      sprintf(fold, "%s.o", fpath);
      rename(fpath, fold);
      if (fsize)
	rename(fnew, fpath);
      else
  contWhileOuter:
	unlink(fnew);

      btime_update(brd_bno(currboard));
    } while (++head < tail);

    /* 還原 currboard */
    strcpy(currboard, tmpboard);
    return XO_LOAD;
  }

  return XO_FOOT;
}
#endif	/* HAVE_TERMINATOR */


/* ----------------------------------------------------- */
/* 板主功能 : 修改板名					 */
/* ----------------------------------------------------- */


static int
post_brdtitle(xo)
  XO *xo;
{
  BRD *oldbrd, newbrd;

  oldbrd = bshm->bcache + currbno;
  memcpy(&newbrd, oldbrd, sizeof(BRD));

  /* itoc.註解: 其實呼叫 brd_title(bno) 就可以了，沒差，蠻幹一下好了 :p */
  if (vans("是否修改中文板名敘述(Y/N)？[N] ") == 'y')
  {
    vget(b_lines, 0, "看板主題：", newbrd.title, BTLEN + 1, GCARRY);

    if (memcmp(&newbrd, oldbrd, sizeof(BRD)) && vans(msg_sure_ny) == 'y')
    {
      memcpy(oldbrd, &newbrd, sizeof(BRD));
      rec_put(FN_BRD, &newbrd, sizeof(BRD), currbno, NULL);
      return XO_HEAD;	/* itoc.011125: 要重繪中文板名 */
    }
  }

  return XO_HEAD;
}


/* ----------------------------------------------------- */
/* 板主功能 : 修改進板畫面				 */
/* ----------------------------------------------------- */


static int
post_memo_edit(xo)
  XO *xo;
{
  int mode;
  char fpath[64];

  mode = vans("進板畫面 (D)刪除 (E)修改 (Q)取消？[E] ");

  if (mode != 'q')
  {
    brd_fpath(fpath, currboard, fn_note);

    if (mode == 'd')
    {
      unlink(fpath);
      return XO_HEAD;
    }

    if (vedit(fpath, 0))	/* Thor.981020: 注意被talk的問題 */
      vmsg(msg_cancel);
    return XO_HEAD;
  }
  return XO_HEAD;
}


/* smiler.080203: 各板自訂擋信機制 */
static int
post_spam_edit(xo)
  XO *xo;
{
  int mode;
  char fpath[64];

  mode = vans("擋信列表 (D)刪除 (E)修改 (Q)取消？[E] ");

  if (mode != 'q')
  {
    brd_fpath(fpath, currboard, "spam");

    if (mode == 'd')
    {
      unlink(fpath);
      return XO_HEAD;
    }
    else
    {
	  more("etc/help/post/post_spam_ed",NULL); 
      if (vedit(fpath, 0))      /* Thor.981020: 注意被talk的問題 */
        vmsg(msg_cancel);
    }

    return XO_HEAD;
  }
  return XO_HEAD;
}

#ifdef POST_PREFIX
/* ----------------------------------------------------- */
/* 板主功能 : 修改發文類別                               */
/* ----------------------------------------------------- */
                                                                                                                                                                
static int
post_prefix_edit(xo)
  XO *xo;
{
#define NUM_PREFIX      8
  int i;
  FILE *fp;
  char fpath[64], buf[20], prefix[NUM_PREFIX][20], *menu[NUM_PREFIX + 3];
  char *prefix_def[NUM_PREFIX] =   /* 預設的類別 */
  {
	  "[閒聊] ", "[公告] ", "[問題] ", "[建議] ", "[討論] ", "[心得] ", "[請益] ", "[情報] "
   // "公告", "測試", "閒聊", "灌水", "無聊", "打混"
  };

  if (!(bbstate & STAT_BOARD))
    return XO_NONE;
  
  i = vans("類別 (D)刪除 (E)修改 (Q)取消？[E] ");

  if (i == 'q')
    return XO_HEAD;

  brd_fpath(fpath, currboard, "prefix");

  if (i == 'd')
  {
    unlink(fpath);
    return XO_HEAD;
  }

  i = 0;

  if (fp = fopen(fpath, "r"))
  {
    for (; i < NUM_PREFIX; i++)
    {
      if (fscanf(fp, "%10s", buf) != 1)
        break;
      sprintf(prefix[i], "%d.%s", i + 1, buf);
    }
    fclose(fp);
  }

  /* 填滿至 NUM_PREFIX 個 */
  for (; i < NUM_PREFIX; i++)
    sprintf(prefix[i], "%d.%s", i + 1, prefix_def[i]);

  menu[0] = "10";
  for (i = 1; i <= NUM_PREFIX; i++)
    menu[i] = prefix[i - 1];
  menu[NUM_PREFIX + 1] = "0.離開";
  menu[NUM_PREFIX + 2] = NULL;

  do
  {
    /* 在 popupmenu 裡面按 左鍵 離開 */
    i = pans(3, 20, "文章類別", menu) - '0';
    if (i >= 1 && i <= NUM_PREFIX)
    {
      strcpy(buf, prefix[i - 1] + 2);
      if (vget(b_lines, 0, "類別：", buf, 10, GCARRY))
        strcpy(prefix[i - 1] + 2, buf);
    }
  } while (i);
                                                                                
  if (fp = fopen(fpath, "w"))
  {
    for (i = 0; i < NUM_PREFIX; i++)
      fprintf(fp, "%s ", prefix[i] + 2);
    fclose(fp);
  }

  return XO_HEAD;
}
#endif      /* POST_PREFIX */


static int
post_brd_prefix(xo)
  XO *xo;
{
  BRD *oldbrd, newbrd;

  oldbrd = bshm->bcache + currbno;

  memcpy(&newbrd, oldbrd, sizeof(BRD));

  switch (vans("使用文章類別 (1)使用 (2)不使用 (3)設定類別 (Q)取消？[Q] "))
  {
  case '1':
    newbrd.battr &= ~BRD_PREFIX;
    break;
  case '2':
    newbrd.battr |= BRD_PREFIX;
    break;
  case '3':
    post_prefix_edit(xo);
  default:
    return XO_HEAD;
  }

  if (memcmp(&newbrd, oldbrd, sizeof(BRD)) && vans(msg_sure_ny) == 'y')
  {
    memcpy(oldbrd, &newbrd, sizeof(BRD));
    rec_put(FN_BRD, &newbrd, sizeof(BRD), currbno, NULL);
  }

  return XO_HEAD;
}


/* ----------------------------------------------------- */
/* 板主功能 : 看板屬性					 */
/* ----------------------------------------------------- */


#ifdef HAVE_SCORE
static int
post_battr_noscore(xo)
  XO *xo;
{
  BRD *oldbrd, newbrd;

  oldbrd = bshm->bcache + currbno;
  memcpy(&newbrd, oldbrd, sizeof(BRD));

  switch (vans("開放評分 (1)允許\ (2)不許\ (Q)取消？[Q] "))
  {
  case '1':
    newbrd.battr &= ~BRD_NOSCORE;
    break;
  case '2':
    newbrd.battr |= BRD_NOSCORE;
    break;
  default:
    return XO_HEAD;
  }

  if (memcmp(&newbrd, oldbrd, sizeof(BRD)) && vans(msg_sure_ny) == 'y')
  {
    memcpy(oldbrd, &newbrd, sizeof(BRD));
    rec_put(FN_BRD, &newbrd, sizeof(BRD), currbno, NULL);
  }

  return XO_HEAD;
}
#endif	/* HAVE_SCORE */

static int
post_rlock(xo)
  XO *xo;
{
  BRD *oldbrd, newbrd;

  oldbrd = bshm->bcache + currbno;

  if (oldbrd->battr & BRD_PUBLIC)  /* 公眾板不允許隨意更動 */
    return XO_HEAD; 

  memcpy(&newbrd, oldbrd, sizeof(BRD));

  switch (vans("開放鎖文 (1)允許\ (2)不許\ (Q)取消？[Q] "))
  {
  case '1':
    newbrd.battr &= ~BRD_NOL;
    break;
  case '2':
    newbrd.battr |= BRD_NOL;
    break;
  default:
    return XO_HEAD;
  }

  if (memcmp(&newbrd, oldbrd, sizeof(BRD)) && vans(msg_sure_ny) == 'y')
  {
    memcpy(oldbrd, &newbrd, sizeof(BRD));
    rec_put(FN_BRD, &newbrd, sizeof(BRD), currbno, NULL);
  }

  return XO_HEAD;
}

static int
post_vpal(xo)
  XO *xo;
{
  BRD *oldbrd, newbrd;

  oldbrd = bshm->bcache + currbno;

  if (oldbrd->battr & BRD_PUBLIC)  /* 公眾板不允許隨意更動 */
    return XO_HEAD; 

  memcpy(&newbrd, oldbrd, sizeof(BRD));

  switch (vans("開放觀看板友名單 (1)允許\ (2)不許\ (Q)取消？[Q] "))
  {
  case '1':
    newbrd.battr &= ~BRD_SHOWPAL;
    break;
  case '2':
    newbrd.battr |= BRD_SHOWPAL;
    break;
  default:
    return XO_HEAD;
  }

  if (memcmp(&newbrd, oldbrd, sizeof(BRD)) && vans(msg_sure_ny) == 'y')
  {
    memcpy(oldbrd, &newbrd, sizeof(BRD));
    rec_put(FN_BRD, &newbrd, sizeof(BRD), currbno, NULL);
  }

  return XO_HEAD;
}

static int
post_noforward(xo)
  XO *xo;
{
  BRD *oldbrd, newbrd;

  oldbrd = bshm->bcache + currbno;
  memcpy(&newbrd, oldbrd, sizeof(BRD));

  switch (vans("轉錄文章 (1)允許\ (2)禁止 (Q)取消？[Q] "))
  {
  case '1':
    newbrd.battr &= ~BRD_NOFORWARD;
    break;
  case '2':
    newbrd.battr |= BRD_NOFORWARD;
    break;
  default:
    return XO_HEAD;
  }

  if (memcmp(&newbrd, oldbrd, sizeof(BRD)) && vans(msg_sure_ny) == 'y')
  {
    memcpy(oldbrd, &newbrd, sizeof(BRD));
    rec_put(FN_BRD, &newbrd, sizeof(BRD), currbno, NULL);
  }

  return XO_HEAD;
}

static int
post_showturn(xo)
  XO *xo;
{
  BRD *oldbrd, newbrd;

  oldbrd = bshm->bcache + currbno;
  memcpy(&newbrd, oldbrd, sizeof(BRD));

  switch (vans("轉錄記錄 (1)打開 (2)關閉 (Q)取消？[Q] "))
  {
  case '1':
    newbrd.battr |= BRD_SHOWTURN;
    break;
  case '2':
	newbrd.battr &= ~BRD_SHOWTURN;
    break;
  default:
    return XO_HEAD;
  }

  if (memcmp(&newbrd, oldbrd, sizeof(BRD)) && vans(msg_sure_ny) == 'y')
  {
    memcpy(oldbrd, &newbrd, sizeof(BRD));
    rec_put(FN_BRD, &newbrd, sizeof(BRD), currbno, NULL);
  }

  return XO_HEAD;
}

/* ----------------------------------------------------- */
/* 板主功能 : 修改板主名單				 */
/* ----------------------------------------------------- */


static int
post_changeBM(xo)
  XO *xo;
{
  char buf[80], userid[IDLEN + 2], *blist;
  BRD *oldbrd, newbrd;
  ACCT acct;
  int BMlen, len;

  oldbrd = bshm->bcache + currbno;

  blist = oldbrd->BM;
  if (is_bm(blist, cuser.userid) != 1)	/* 只有正板主可以設定板主名單 */
    return XO_HEAD;

  if (oldbrd->battr & BRD_PUBLIC)  /* 公眾板不允許隨意更動 */
    return XO_HEAD;      

  memcpy(&newbrd, oldbrd, sizeof(BRD));

  move(3, 0);
  clrtobot();

  move(8, 0);
  prints("目前板主為 %s\n請輸入新的板主名單，或按 [Return] 不改", oldbrd->BM);

  strcpy(buf, oldbrd->BM);
  BMlen = strlen(buf);

  while (vget(10, 0, "請輸入副板主，結束請按 Enter，清掉所有副板主請打「無」：", userid, IDLEN + 1, DOECHO))
  {
    if (!strcmp(userid, "無"))
    {
      strcpy(buf, cuser.userid);
      BMlen = strlen(buf);
    }
    else if (is_bm(buf, userid))	/* 刪除舊有的板主 */
    {
      len = strlen(userid);
      if (!str_cmp(cuser.userid, userid))
      {
	vmsg("不可以將自己移出板主名單");
	continue;
      }
      else if (!str_cmp(buf + BMlen - len, userid))	/* 名單上最後一位，ID 後面不接 '/' */
      {
	buf[BMlen - len - 1] = '\0';			/* 刪除 ID 及前面的 '/' */
	len++;
      }
      else						/* ID 後面會接 '/' */
      {
	str_lower(userid, userid);
	strcat(userid, "/");
	len++;
	blist = str_str(buf, userid);
	strcpy(blist, blist + len);
      }
      BMlen -= len;
    }
    else if (acct_load(&acct, userid) >= 0 && !is_bm(buf, userid))	/* 輸入新板主 */
    {
      len = strlen(userid) + 1;	/* '/' + userid */
      if (BMlen + len > BMLEN)
      {
	vmsg("板主名單過長，無法將這 ID 設為板主");
	continue;
      }
      sprintf(buf + BMlen, "/%s", acct.userid);
      BMlen += len;

      acct_setperm(&acct, PERM_BM, 0);
    }
    else
      continue;

    move(8, 0);
    prints("目前板主為 %s", buf);
    clrtoeol();
  }
  strcpy(newbrd.BM, buf);

  if (memcmp(&newbrd, oldbrd, sizeof(BRD)) && vans(msg_sure_ny) == 'y')
  {
    memcpy(oldbrd, &newbrd, sizeof(BRD));
    rec_put(FN_BRD, &newbrd, sizeof(BRD), currbno, NULL);

    sprintf(currBM, "板主：%s", newbrd.BM);
    return XO_HEAD;	/* 要重繪檔頭的板主 */
  }

  return XO_HEAD;
}


#ifdef HAVE_MODERATED_BOARD
/* ----------------------------------------------------- */
/* 板主功能 : 看板權限					 */
/* ----------------------------------------------------- */


static int
post_brdlevel(xo)
  XO *xo;
{
  BRD *oldbrd, newbrd;

  oldbrd = bshm->bcache + currbno;
  memcpy(&newbrd, oldbrd, sizeof(BRD));
  
  if (oldbrd->battr & BRD_PUBLIC)  /* 公眾板不允許隨意更動 */
    return XO_HEAD;

  if (oldbrd->battr & BRD_IAS)	/* 藝文館看板不允許隨意更動 */
  {
    vmsg("藝文館看板如需更動屬性請向館務申請!!");
    return XO_HEAD;
  }

  switch (vans("1)公開看板 2)秘密看板 3)好友看板？[Q] "))
  {
  case '1':				/* 公開看板 */
    newbrd.readlevel = 0;
    newbrd.postlevel = PERM_POST;
    newbrd.battr &= ~(BRD_NOSTAT | BRD_NOVOTE);
    break;

  case '2':				/* 秘密看板 */
    newbrd.readlevel = PERM_SYSOP;
    newbrd.postlevel = 0;
    newbrd.battr |= (BRD_NOSTAT | BRD_NOVOTE);
    break;

  case '3':				/* 好友看板 */
    newbrd.readlevel = PERM_BOARD;
    newbrd.postlevel = 0;
    newbrd.battr |= (BRD_NOSTAT | BRD_NOVOTE);
    break;

  default:
    return XO_HEAD;
  }

  if (memcmp(&newbrd, oldbrd, sizeof(BRD)) && vans(msg_sure_ny) == 'y')
  {
    memcpy(oldbrd, &newbrd, sizeof(BRD));
    rec_put(FN_BRD, &newbrd, sizeof(BRD), currbno, NULL);
  }

  return XO_HEAD;
}
#endif	/* HAVE_MODERATED_BOARD */


#ifdef HAVE_MODERATED_BOARD
/* ----------------------------------------------------- */
/* 板友名單：moderated board				 */
/* ----------------------------------------------------- */


static void
bpal_cache(fpath)
  char *fpath;
{
  BPAL *bpal;

  bpal = bshm->pcache + currbno;
  bpal->pal_max = image_pal(fpath, bpal->pal_spool);
}


extern XZ xz[];


static int
XoBM(xo)
  XO *xo;
{
  BRD *oldbrd;
  oldbrd = bshm->bcache + currbno;

  XO *xt;
  char fpath[64];

  brd_fpath(fpath, currboard, fn_pal);
  xz[XZ_PAL - XO_ZONE].xo = xt = xo_new(fpath);
  xt->key = PALTYPE_BPAL;
  xover(XZ_PAL);		/* Thor: 進xover前, pal_xo 一定要 ready */

  /* build userno image to speed up, maybe upgreade to shm */

  bpal_cache(fpath);

  free(xt);

  return XO_INIT;
}
#endif	/* HAVE_MODERATED_BOARD */


/* ----------------------------------------------------- */
/* 看門狗						 */
/* ----------------------------------------------------- */

static int
post_bbs_dog(xo)
  XO *xo;
{
  BRD *oldbrd, newbrd;

  oldbrd = bshm->bcache + currbno;
  memcpy(&newbrd, oldbrd, sizeof(BRD));

  switch (vans("加入BBS看門狗計畫 1)是 2)否 Q)取消？[Q] "))
  {
  case '1':
    newbrd.battr |= BRD_BBS_DOG;
    break;
  case '2':
    newbrd.battr &= (~BRD_BBS_DOG);
    break;
  default:
    return XO_HEAD;
  }

  if (memcmp(&newbrd, oldbrd, sizeof(BRD)) && vans(msg_sure_ny) == 'y')
  {
    memcpy(oldbrd, &newbrd, sizeof(BRD));
    rec_put(FN_BRD, &newbrd, sizeof(BRD), currbno, NULL);
  }

  return XO_HEAD;
}

static int
post_article_filter(xo)
  XO *xo;
{
#define	NUM_DOG		10
#define	LEN_DOG_NAME	70

  int i;
  int choose;
  FILE *fp;
  char fpath[64], buf[LEN_DOG_NAME], input[LEN_DOG_NAME], dog[NUM_DOG][LEN_DOG_NAME];
 
  brd_fpath(fpath, currboard, FN_BBSDOG);

  if (!dashf(fpath))
  {
    fp = fopen(fpath, "w");
    i = 0;
    while (i < 10)
    {
      fprintf(fp, "%c\n",'\0');
      i++;
    }
    fclose(fp);
  }
  
  choose = 0;
  do
  {
    if (choose)
    {
      move(b_lines - 1, 0);
      prints("修改第 %d 項 : ", choose);

      if (!vget(b_lines, 0, "", input, LEN_DOG_NAME, DOECHO))
	input[0] = '\0';

      while (strstr(input," "))
      {
	move(b_lines - 2, 0);
	prints("\033[1;33m輸入之字串中不得有空格，請重新輸入 !!\033[m\n");
	prints("修改第 %d 項 : ", choose);

	if (!vget(b_lines, 0, "", input, LEN_DOG_NAME, DOECHO))
	{
	  input[0] = '\0';
	  break;
	}
      }
      if (input[0] != '\0')
	strcpy(dog[choose - 1], input);
      else
	dog[choose - 1][0] = '\0';

      fp = fopen(fpath, "w");
      for (i = 0 ; i < NUM_DOG ; i++ )
      {
	 if (dog[i][0] == '\0')
	 {
	   fprintf(fp, "%c\n", dog[i][0]);
	 }
	 else
	   fprintf(fp, "%s\n", dog[i]);
      }
      fclose(fp);
    }

    fp = fopen(fpath, "r");
    clear();
    for (i = 0; i < NUM_DOG; i++)
    {
      prints("%d.\n", i+1);

      fscanf(fp, "%s", &buf);

      if ((buf[0] == '\0') || (buf[0] == '\n'))
      {
	prints("\n");
	dog[i][0] = '\0';
      }
      else
      {
	prints("\033[0;30;47m%s\033[m\n", buf);
	strcpy(dog[i], buf);
      }
    }
    fclose(fp);

    switch(vans("選擇 (D)刪除 (E)修改 (Q)取消？[E] "))
    {
    case 'd':
      unlink(fpath);
    case 'q':
      return XO_INIT;
    }

    if (!vget(b_lines, 0, "◎ 選則修改 1)~10) (Q)離開 [Q]", input, 3, DOECHO))
      break;
    choose = atoi(input);

    if (choose > 10 || choose < 0)
      break;
  } while(choose);

  return XO_INIT;
}

/* smiler.080831: 板主自定 可讀/可寫/可列
   目前判斷項次 :
   1. 生日     (年齡限制　: 18禁　or any level)
   2. 性別     (男性專板，女性專板....balabala)
   3. 上線次數 (不得小於多少次)
   4. 文章篇數 (不得小於多少篇)
   5. 優文篇數 (不得小於多少篇)
   6. 劣文篇數 (不得高於多少篇)
   7. 違規次數 (不得高於多少次)
   8. 銀幣　　 (不得少於多少枚)
   9. 金幣　　 (不得少於多少枚)
  10. 發信次數 (不得小於多少次)
  11. 註冊時間 (不得低於多少月)

*/

static int
post_my_level(xo, fname)
  XO *xo;
  char *fname;
{
  FILE *fr, *fw;
  char fpath_r[64], fpath_w[64], buf[64], wd[64];
  int i, wi;

  brd_fpath(fpath_r, currboard, fname);
  sprintf(fpath_w, "%s.tmp", fpath_r);

  if (!dashf(fpath_r))
  {
    fr = fopen(fpath_r, "w");
    for (i = 0; i < 10; i++)
    fprintf(fr, "0\n");
    fclose(fr);
  }

  while (1)
  {
    clear();

    if (!(fr = fopen(fpath_r, "r")))
      return XO_HEAD;

    if (!(fw = fopen(fpath_w, "w")))
    {
      fclose(fr);
      return XO_HEAD;
    }

    i = 3;
    fscanf(fr, "%d", &wi);
    sprintf(wd, "年齡限制 >= [%2d歲]：", wi);
    if (vget(i, 0, wd, buf, 3, DOECHO))
      wi = atoi(buf);
    if (wi >= 0)
      fprintf(fw, "%d\n", wi);
    else
      fprintf(fw, "0\n");

    i++;
    fscanf(fr, "%d", &wi);
    sprintf(wd, "性別限制 (0)不限(1)中性 (2)男性 (3)女性：[%d] ", wi);
    if (vget(i, 0, wd, buf, 2, DOECHO))
      wi = (atoi(buf) % 4);
    if (wi >= 0)
      fprintf(fw, "%d\n", wi);
    else
      fprintf(fw, "0\n");

    i++;
    fscanf(fr, "%d", &wi);
    sprintf(wd, "上線次數 >= [%d次] ", wi);
    if (vget(i, 0, wd, buf, 10, DOECHO))
      wi = atoi(buf);
    if (wi >= 0)
      fprintf(fw, "%d\n", wi);
    else
      fprintf(fw, "0\n");

    i++;
    fscanf(fr, "%d", &wi);
    sprintf(wd, "文章篇數 >= [%d篇] ", wi);
    if (vget(i, 0, wd, buf, 10, DOECHO))
      wi = atoi(buf);
    if (wi >= 0)
      fprintf(fw, "%d\n", wi);
    else
      fprintf(fw, "0\n");

    i++;
    fscanf(fr, "%d", &wi);
    sprintf(wd, "優文篇數 >= [%d篇] ", wi);
    if (vget(i, 0, wd, buf, 10, DOECHO))
      wi = atoi(buf);
    if (wi >= 0)
      fprintf(fw, "%d\n", wi);
    else
      fprintf(fw, "0\n");

    i++;
    fscanf(fr, "%d", &wi);
    sprintf(wd, "劣文篇數 <  [%d篇] (0：取消) ", wi);
    if (vget(i, 0, wd, buf, 10, DOECHO))
      wi = atoi(buf);
    if (wi >= 0)
      fprintf(fw, "%d\n", wi);
    else
      fprintf(fw, "0\n");

    i++;
    fscanf(fr, "%d", &wi);
    sprintf(wd, "違規次數 <  [%d次] (0：取消) ", wi);
    if (vget(i, 0, wd, buf, 10, DOECHO))
      wi = atoi(buf);
    if (wi >= 0)
      fprintf(fw, "%d\n", wi);
    else
      fprintf(fw, "0\n");

    i++;
    fscanf(fr, "%d", &wi);
    sprintf(wd, "銀幣     >= [%d枚] ", wi);
    if (vget(i, 0, wd, buf, 10, DOECHO))
      wi = atoi(buf);
    if (wi >= 0)
      fprintf(fw, "%d\n", wi);
    else
      fprintf(fw, "0\n");

    i++;
    fscanf(fr, "%d", &wi);
    sprintf(wd, "金幣     >= [%d枚] ", wi);
    if (vget(i, 0, wd, buf, 10, DOECHO))
      wi = atoi(buf);
    if (wi >= 0)
      fprintf(fw, "%d\n", wi);
    else
      fprintf(fw, "0\n");

    i++;
    fscanf(fr, "%d", &wi);
    sprintf(wd, "發信次數 >= [%d次] ", wi);
    if (vget(i, 0, wd, buf, 10, DOECHO))
      wi = atoi(buf);
    if (wi >= 0)
      fprintf(fw, "%d\n", wi);
    else
      fprintf(fw, "0\n");

    i++;
    fscanf(fr, "%d", &wi);
    sprintf(wd, "註冊時間 >= [%3d月] ", wi);
    if (vget(i, 0, wd, buf, 4, DOECHO))
      wi = atoi(buf);
    if (wi >= 0)
      fprintf(fw, "%d\n", wi);
    else
      fprintf(fw, "0\n");

    fclose(fr);
    fclose(fw);

    switch (vans("進板畫面 (S)存檔 (E)繼續 (Q)取消？[Q] "))
    {
    case 's':
      unlink(fpath_r);
      rename(fpath_w, fpath_r);
      return XO_INIT;

    case 'e':
      continue;

    default:
      unlink(fpath_w);
      return XO_INIT;
    }
 }

 return XO_INIT;
}


static int
post_view_bbs_dog_log(xo)
  XO *xo;
{
  char fpath[64],warn[64];

  brd_fpath(fpath, currboard, FN_BBSDOG_LOG);
  more(fpath, NULL);

  switch (vans("擋文記錄 M)寄回自己信箱 D)刪除 Q)離開？[Q] "))
  {
  case 'm':
    sprintf(warn, "%s 板擋文記錄檔", currboard);
    mail_self(fpath, cuser.userid, warn, 0);
    return XO_HEAD;

    case 'd':
    unlink(fpath);
    return XO_HEAD;

  default:
    return XO_HEAD;
  }
  return XO_HEAD;
}

/* ----------------------------------------------------- */
/* 板主選單						 */
/* ----------------------------------------------------- */

static int
post_guard_dog(xo)
  XO *xo;
{
  char fpath[64];
  char *menu[] = 
  {
    "BQ",
    "BBSdog  BBS看門狗計畫",
    "Post    文章內容限制",
    "Write   看板發文限制",
    "Read    看板閱\讀限制",
    "List    看板列出限制",
    "Vlog    擋文log記錄",
    NULL
  };

  sprintf(fpath, BBSHOME"/gem/@/@BBS_DOG_WARN");
  more(fpath, NULL);

  clear();
  switch (pans(3, 20, "BBS 看門狗", menu))
  {
  case 'b':
    return post_bbs_dog(xo);
  case 'p':
    return post_article_filter(xo);
  case 'w':
    return post_my_level(xo, FN_NO_WRITE);
  case 'r':
    return post_my_level(xo, FN_NO_READ);
  case 'l':
    return post_my_level(xo, FN_NO_LIST);
  case 'v':
    return post_view_bbs_dog_log(xo);
  }
  return XO_INIT;
}


int
post_manage(xo)
  XO *xo;
{
  BRD *brd;
#ifdef POPUP_ANSWER
  char *menu[] = 
  {
    "BQ",
    "BTitle  修改看板主題",
    "WMemo   編輯進板畫面",
    "Manager 增減副板主",
#  ifdef HAVE_MODERATED_BOARD
    "Level   公開/好友/秘密",
    "OPal    板友名單",
    "VPal    可否觀看板友名單",
#  endif
#  ifdef POST_PREFIX
    "Prefix  設定文章類別",
#  endif
    "ASpam   看板擋信列表",
#  ifdef HAVE_SCORE
    "Score   設定可否評分",
#  endif
    "RLock   板友可否鎖文",
    "Nfward  看板禁止轉錄",
    "Fshow   轉錄記錄開啟",
    "Guard   BBS 看門狗",
    NULL
  };
#else
  char *menu = "◎ 板主選單 (B)主題 (W)進板 (S)擋信 (M)副板"
#  ifdef HAVE_MODERATED_BOARD
    " (L)權限 (O)板友 (V)名單"
#  endif
#  ifdef POST_PREFIX
    " (P)類別"
#  endif    
    " (A)擋信"
#  ifdef HAVE_SCORE
    " (S)評分"
#  endif
    " (R)鎖文 (N)轉錄 (F)紀錄 (G)看門狗"
    "？[Q] ";
#endif

  if (!(bbstate & STAT_BOARD))
    return XO_HEAD;

  vs_bar("板主管理");
  brd = bshm->bcache + currbno;
  prints("看板名稱：%s\n看板說明：[%s] %s\n板主名單：%s\n",
    brd->brdname, brd->class, brd->title, brd->BM);
  prints("中文敘述：%s\n", brd->title);
#ifdef HAVE_MODERATED_BOARD
  prints("看板權限：%s看板\n", brd->readlevel == PERM_SYSOP ? "秘密" : brd->readlevel == PERM_BOARD ? "好友" : "公開");
#endif

#ifdef POPUP_ANSWER
  switch (pans(3, 20, "板主選單", menu))
#else
  switch (vans(menu))
#endif
  {
  case 'b':
    return post_brdtitle(xo);

  case 'w':
    return post_memo_edit(xo);

  case 'a':
    return post_spam_edit(xo);

  case 'm':
    return post_changeBM(xo);

#ifdef HAVE_SCORE
  case 's':
    return post_battr_noscore(xo);
#endif

#ifdef HAVE_MODERATED_BOARD
  case 'l':
    return post_brdlevel(xo);

  case 'o':
    return XoBM(xo);

  case 'v':
    return post_vpal(xo);
#endif

#ifdef POST_PREFIX
  case 'p':
    return post_brd_prefix(xo);
#endif
  case 'r':
    return post_rlock(xo);
  case 'n':
    return post_noforward(xo);
  case 'f':
    return post_showturn(xo);
  case 'g':
    return post_guard_dog(xo);
  }

  return XO_HEAD;
}

