/*-------------------------------------------------------*/
/* bmtrans.c   ( NTHU CS MapleBBS Ver 3.00 )		 */
/*-------------------------------------------------------*/
/* target : 板主異動					 */
/* create : 08/12/07					 */
/* update : //						 */
/*-------------------------------------------------------*/


#include "bbs.h"

#if 0
  PAL 檔案規劃
                  PAL_BAD        PAL_MATE     pal.ship[0]
    old master                       o            'o'
    old slave        o                            'o'

    new master                       o            'c'
    new slave        o                            'c'

    not change
    have completed                                'b'

  需要附議
    *. 新任正板主	(PALMATE | c) && !(PAL_MATE | o)

  不需附議
    *. 新舊任皆為副板主
    *. 卸任板主

  已完成所需附議條件
    *. 名單中已沒有任何 (PALMATE | c)

  設定新板主權限
    *. 增加 (PALMATE | b) 板主權限
    *. 增加 (PAL_BAD | c) 板主權限
    *. 更新 .BRD 與 bcache 中的板主名單
    *. 砍除 PAL 檔
#endif

extern BCACHE *bshm;


#define	MSG_SEPERATE	"\033[36m"MSG_SEPERATOR"\033[m\n"
#define BRD_NEWBM	"newbm"		/* 提供申請異動板主的板名 */


static int
bmt_setperm(xo, hdr)
  XO *xo;
  HDR *hdr;
{
  ACCT acct;
  PAL pal;
  BRD *brd;
  int pos, num;
  char fpath[64], buf[256];
  time_t now;
  struct tm *ptime;

  sprintf(fpath, "brd/%s/bmt/%s", BRD_NEWBM, hdr->xname);
  num = rec_num(fpath, sizeof(PAL));
  pos = 0;

  while (pos < num)
  {
    rec_get(fpath, &pal, sizeof(PAL), pos);

    if (acct_load(&acct, pal.userid) >= 0)
      acct_setperm(&acct, PERM_BM, 0);
    pos++;
  }

  if (!strcmp(pal.userid, "--------"))
  {
    brd = bshm->bcache + pal.userno;
    strcpy(brd->BM, pal.ship);
    rec_put(FN_BRD, brd, sizeof(BRD), pal.userno, NULL);
    unlink(fpath);

    time(&now);
    ptime = localtime(&now);
    sprintf(buf, "\033[1;30m== sysop\033[m：\033[1;33m%-51s\033[1;30m%02d/%02d %02d:%02d:%02d\033[m\n",
      "申請案通過",
      ptime->tm_mon + 1, ptime->tm_mday, ptime->tm_hour, ptime->tm_min, ptime->tm_sec);
    hdr_fpath(fpath, xo->dir, hdr);
    f_cat(fpath, buf);

    hdr->xmode |= (POST_MARKED | POST_SCORE);
    currchrono = hdr->chrono;
    rec_put(xo->dir, hdr, sizeof(HDR),
      xo->key == XZ_XPOST ? hdr->xid : xo->pos, cmpchrono);

    vmsg("申請案通過");
    return 1;
  }

  vmsg("申請案設定失敗，請至 sysop 板回報");
  return 0;
}


int
bmt_sign(xo, hdr)
  XO *xo;
  HDR *hdr;
{
  PAL pal;
  int pos, num, mode;
  char fpath[64];

  sprintf(fpath, "brd/%s/bmt/%s", BRD_NEWBM, hdr->xname);
  num = rec_num(fpath, sizeof(PAL)) - 1;
  pos = mode = 0;

  while (pos < num)
  {
    rec_get(fpath, &pal, sizeof(PAL), pos);

    if (!strcmp(pal.userid, cuser.userid))
    {
      if (!pal.ftype)
	vmsg("您已附議過了");
      else if (vans("確定要附議(y/n)？[Q] ") == 'y')
      {
	pal.ftype = 0;
	rec_put(fpath, &pal, sizeof(PAL), pos, NULL);
        post_t_score(xo, "附議此篇申請案", hdr);
	vmsg("附議成功\");
      }
      if (!mode)
        mode = 1;
    }

    if (mode && pal.ftype)
      mode = -1;	/* not completed */
    pos++;
  }

  if (!mode)
    vmsg("您不需要附議此申請案");
  else if (mode > 0)	/* completed */
    bmt_setperm(xo, hdr);

  return 0;
}


int
bmt_add(xo)
  XO *xo;
{
  BRD *brd;
  ACCT acct;
  PAL pal;
  HDR hdr;
  char fpath[64], buf[80];
  char BMlist[BMLEN + 1];
  char *blist, *ptr;
  int bno, BMlen, len;
  FILE *fp;

  move(b_lines - 20, 0);
  clrtobot();
  prints("%s\n", MSG_SEPERATE);
  prints("\033[1;33m楓橋驛站 " BRD_NEWBM " 板主異動系統\033[m\n"
        "本系統功\用在協助各公眾看板進行板主名單異動，\033[1;31m請勿濫用\033[m\n"
        "若發現有不當使用情形，將以站規處理。\n"
        "有任何使用上的問題，麻煩至 \033[1;33msysop\033[m 板回報，謝謝！\n\n");
  prints("%s\n", MSG_SEPERATE);

  if (!vget(b_lines, 0, "英文板名：", buf, BNLEN + 1, DOECHO))
    return 0;

  if ((bno = brd_bno(buf)) < 0)
  {
    vmsg("板名錯誤！");
    return 0;
  }

// get old BM list
// check if master BM!
  brd = bshm->bcache + bno;

  blist = brd->BM;
  if (!(cuser.userlevel & PERM_BM) || blist[0] <= ' ' ||
    (is_bm(blist, cuser.userid) != 1))
  {
    vmsg("您不為正板主，無法申請！");
    return 0;
  }

#if 0
  if (!(brd->battr & BRD_PUBLIC))	/* 只有公眾才需要申請 */
  {
    vmsg("此看板為公眾板，可由正板主自行設定名單");
    return 0;
  }
#endif

  move(10, 0);
  prints("英文板名：%s\n中文板名：%s\n\n原板主名單：%s\n新板主名單：%s\n%s\n",
    brd->brdname, brd->title, blist, blist, MSG_SEPERATE);

// while(vget())
//   get new bm list
//   check if has BM basic request
//   check if less than BNLEN
  strcpy(BMlist, brd->BM);
  BMlen = strlen(BMlist);

  while(1)
  {
    if (!vget(b_lines, 0, "請輸入新任板主名單，或移除舊板主，結束請按 Enter：", buf, IDLEN + 1, DOECHO))
    {
      if (vans("結束輸入 1)是的，名單輸入無誤 2)否，我要重新輸入 ？[1] ") == '2')
      {
	strcpy(BMlist, brd->BM);
	BMlen = strlen(BMlist);
	continue;
      }
      else
	break;
    }

    if (is_bm(BMlist, buf))	/* 刪除舊有的板主 */
    {
      len = strlen(buf);
      if (BMlen == len)
      {
	BMlist[0] = '\0';
      }
      else if (!str_cmp(BMlist + BMlen - len, buf))	/* 名單上最後一位，ID 後面不接 '/' */
      {
	BMlist[BMlen - len - 1] = '\0';			/* 刪除 ID 及前面的 '/' */
	len++;
      }
      else						/* ID 後面會接 '/' */
      {
	str_lower(buf, buf);
	strcat(buf, "/");
	len++;
	ptr = str_str(BMlist, buf);
        strcpy(ptr, ptr + len);
      }
      BMlen -= len;
    }
    else if (acct_load(&acct, buf) >= 0 && !is_bm(BMlist, buf))	/* 輸入新板主 */
    {
      if (!(acct.userlevel & PERM_VALID))
      {
	vmsg("該使用者尚未通過認証");
	continue;
      }
#if 0
      if (acct.numlogins < 300)
      {
	vmsg("需上站 300 次以上才能擔任板主");
	continue;
      }
      if (acct.staytime < 100 * 3600)
      {
	vmsg("上站時數需達 100 小時以上才能擔任板主");
	continue;
      }
#endif

      len = strlen(buf);
      if (BMlen)
      {
	len++;		/* '/' + userid */
	if (BMlen + len > BMLEN)
	{
	  vmsg("板主名單過長，無法將這 ID 設為板主");
	  continue;
	}
	sprintf(BMlist + BMlen, "/%s", acct.userid);
	BMlen += len;
      }
      else
      {
	strcpy(BMlist, acct.userid);
	BMlen = len;
      }
    }
    else
      continue;

    move(14, 0);
    prints("新板主名單：%s\n", BMlist);
  }

  if (!strcmp(BMlist, blist))
  {
    vmsg("無任何更動");
    return 0;
  }
  if (!strlen(BMlist))
  {
    vmsg("唯一的板主請辭需由站務手動審核，請另行申請");
    return 0;
  }

  if (vans("請核對資料是否正確 1)正確，提出申請 2)取消 ？[1] ") == '2')
  {
    unlink("pal_file");
    vmsg("取消申請");
    return 0;
  }

// auto post to board BM
  sprintf(fpath, "tmp/%s.bmt", cuser.userid);
  sprintf(buf, "[申請板主異動] %s", brd->brdname);
  if (fp = fopen(fpath, "w"))
  {
    fprintf(fp, "作者: %s (%s) 站內: %s\n", cuser.userid, cuser.username, BRD_NEWBM);
    fprintf(fp, "標題: %s\n", buf);
    fprintf(fp, "時間: %s\n\n\n", Now());
    fprintf(fp, "英文板名：%s\n", brd->brdname);
    fprintf(fp, "中文板名：%s\n", brd->title);
    fprintf(fp, "原板主名單：%s\n", blist);
    fprintf(fp, "新板主名單：%s\n", BMlist);
    fprintf(fp, "\n\n\n--\n※ 楓橋驛站 - 板主交接系統\n");
    fclose(fp);
    add_post(BRD_NEWBM, fpath, buf, cuser.userid, cuser.username, 0, &hdr);
    unlink(fpath);
  }

// add PAL file that contains BM changes
// set file mode ?
// add function key to let BM changes
// add detect function if all new BM got sign

  sprintf(fpath, "brd/%s/bmt/%s", BRD_NEWBM, hdr.xname);
  blist = BMlist;

  do
  {
    if (ptr = strchr(blist, '/'))
    {
      str_ncpy(buf, blist, ptr - blist + 1);
      blist = ptr + 1;
    }
    else	/* last one */
      strcpy(buf, blist);

    memset(&pal, 0, sizeof(PAL));
    acct_load(&acct, buf);
    strcpy(pal.userid, acct.userid);
    pal.userno = acct.userno;
    if (is_bm(BMlist, acct.userid) == 1 && is_bm(brd->BM, acct.userid) != 1)
      pal.ftype = PAL_MATE;	/* new master */
    else if (is_bm(BMlist, acct.userid) && !is_bm(brd->BM, acct.userid))
      pal.ftype = PAL_BAD;	/* new slave */

    if (pal.ftype)
      rec_add(fpath, &pal, sizeof(PAL));
  } while (ptr);

  memset(&pal, 0, sizeof(PAL));
  strcpy(pal.userid, "--------");
  pal.userno = bno;
  strcpy(pal.ship, BMlist);
  rec_add(fpath, &pal, sizeof(PAL));

  if (rec_num(fpath, sizeof(PAL)) == 1)
    bmt_setperm(xo, &hdr);
  else
    vmsg("申請完成，請新任板主附議");

  return 0;
}
