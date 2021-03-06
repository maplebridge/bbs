/*-------------------------------------------------------*/
/* battr.h	( NTHU CS MapleBBS Ver 2.36 )		 */
/*-------------------------------------------------------*/
/* target : Board Attribution				 */
/* create : 95/03/29				 	 */
/* update : 95/12/15				 	 */
/*-------------------------------------------------------*/


#ifndef	_BATTR_H_
#define	_BATTR_H_


/* ----------------------------------------------------- */
/* Board Attribution : flags in BRD.battr		 */
/* ----------------------------------------------------- */


#define BRD_NOZAP	0x00000001	/* 不可 zap */
#define BRD_NOTRAN	0x00000002	/* 不轉信 */
#define BRD_NOCOUNT	0x00000004	/* 不計文章發表篇數 */
#define BRD_NOSTAT	0x00000008	/* 不納入熱門話題統計 */
#define BRD_NOVOTE	0x00000010	/* 不公佈投票結果於 [record] 板 */
#define BRD_ANONYMOUS	0x00000020	/* 匿名看板 */
#define BRD_NOSCORE	0x00000040	/* 不評分看板 */
#define BRD_NOL		0x00000100	/* 不可鎖文 */
#define	BRD_HIDEPAL	0x00000200	/* 隱藏板友名單 */
#define BRD_NOPREFIX	0x00000400	/* 關閉看板 POST_PREFIX 功能 */
#define BRD_NOFORWARD	0x00000800	/* 看板禁止轉錄 */
#define BRD_SHOWTURN	0x00001000	/* 文章註記轉錄記錄看板 */
#define BRD_IAS		0x00002000	/* 藝文館看板 */
#define BRD_ATOM	0x00004000	/* ATOM 成員看板 */
#define BRD_BBS_DOG	0x00008000	/* 加入imaple BBS DOG 計畫 */
#define BRD_NODELETE	0x00010000	/* 禁止刪除|修文 */
#define BRD_POST_IP	0x00020000	/* 1:推文以ip顯示 0:以ip代碼顯示 */

/* ----------------------------------------------------- */
/* 各種旗標的中文意義					 */
/* ----------------------------------------------------- */


#define NUMBATTRS	18

#define STR_BATTR	"zTcsvA%PLGpFRIaBEi"			/* itoc: 新增旗標的時候別忘了改這裡啊 */


#ifdef _ADMIN_C_
static char *battr_tbl[NUMBATTRS] =
{
  "不可 Zap",		/* BRD_NOZAP */
  "不轉信出去",		/* BRD_NOTRAN */
  "不記錄篇數",		/* BRD_NOCOUNT */
  "不做熱門話題統計",	/* BRD_NOSTAT */
  "不公開投票結果",	/* BRD_NOVOTE */
  "匿名看板",		/* BRD_ANONYMOUS */
  "不評分看板",		/* BRD_NOSCORE */
  "保留",		/* */
  "不可鎖文",		/* BRD_NOL */
  "隱藏(不顯示)板友名單",/* BRD_HIDEPAL */
  "停止使用文章類別", 	/* BRD_NOPREFIX */
  "看板禁止轉錄",	/* BRD_NOFORWARD */
  "轉錄記錄",		/* BRD_SHOWTURN */
  "藝文館看板",		/* BRD_IAS */
  "ATOM成員看板",	/* BRD_ATOM */
  "加入BBS DOG 計畫",	/* BRD_BBS_DOG */
  "禁止刪除修改文章",	/* BRD_NOEDIT */
  "推文顯示ip或ip代碼"	/* BRD_POST_IP */
};

#endif

#endif				/* _BATTR_H_ */
