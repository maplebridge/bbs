/*-------------------------------------------------------*/
/* util/snap2brd.c	( NTHU CS MapleBBS Ver 3.10 )	 */
/*-------------------------------------------------------*/
/* target : M3 BRD 轉換程式				 */
/* create : 03/07/09					 */
/* update :   /  /  					 */
/* author : itoc.bbs@bbs.tnfsh.tn.edu.tw		 */
/*-------------------------------------------------------*/


#include "snap.h"
#include <netinet/in.h>

#define FN_BRD_TMP	"./.BRD.tmp"
//#define FN_BRD		"./.BRD"
//char FN_BRD_TMP[]=".BRD.tmp";
//char FN_BRD[]=".BRD";

usint transbattr(usint originbattr);

int main()
{
  boardheader bh;
  BRD brd;
  FILE *fp;
  /*printf("%d %d", sizeof(bh), sizeof(brd));
  return 0;*/
  
  chdir(BBSHOME);/*/記得將這行回復*/
  if (!(fp = fopen(FN_BRD, "r")))
  {
    return -1;
  }
  while (fread(&bh, sizeof(bh), 1, fp) == 1)
  {
    if (!*bh.brdname)	/* 此板已被刪除 */
      continue;

    memset(&brd, 0, sizeof(BRD));

    /* 轉換的動作在此 */
    str_ncpy(brd.brdname, bh.brdname, sizeof(brd.brdname));
    str_ncpy(brd.class, bh.bclass, sizeof(brd.class));		/* 分類不用重設 */
    str_ncpy(brd.title, bh.title /*+ 3*/, sizeof(brd.title));	/* 跳過 "□ " */
    str_ncpy(brd.BM, bh.BM, sizeof(brd.BM));
    brd.bvote = bh.bvote;
    brd.bstamp = ntohl(bh.bstamp);//系統若一樣不用加ntohl
    brd.battr = transbattr(ntohl(bh.battr));
    brd.readlevel = ntohl(bh.readlevel);
    brd.postlevel = ntohl(bh.postlevel);
    if( brd.readlevel == 0x80000000)
    {
      printf("%s\n",brd.brdname);	/*debug*/
      brd.readlevel = brd.readlevel | 0xF8000000;
      brd.postlevel = brd.postlevel | 0xF8000000;
    }
    brd.btime = ntohl(bh.btime);
    brd.bpost = ntohl(bh.bpost);
    brd.blast = 0;

    rec_add(FN_BRD_TMP, &brd, sizeof(BRD));
  }

  fclose(fp);

  /* 刪除舊的，把新的更名 */
  unlink(FN_BRD);
  rename(FN_BRD_TMP, FN_BRD);

  return 0;
}

usint transbattr(usint originbattr)
{
  usint newbattr=originbattr;
  if(originbattr & 0x40 != 0)
  {
    newbattr &= (~0x40);/*BRD_VIP已無;*/
  }
  newbattr |= 0x80;
}
