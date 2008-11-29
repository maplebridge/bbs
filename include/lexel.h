/*-------------------------------------------------------*/
/* lexel.h	( NTHU CS MapleBBS Ver 3.10 )		 */
/*-------------------------------------------------------*/
/* target : lexel custom config				 */
/* create : 08/11/02				 	 */
/* update :   /  /  				 	 */
/*-------------------------------------------------------*/


#ifdef SITE_LEXEL

#undef BBSNAME
#undef BBSNAME2
#undef BBSNAME3
#undef SYSOPNICK
#undef MYIPADDR
#undef MYHOSTNAME
#undef HOST_ALIASES
#undef BBSHOME
#undef BAKPATH
#undef BBSUID
#undef BBSGID
#undef COLOR1
#undef COLOR2
#undef COLOR3
#undef COLOR4
#undef COLOR5
#undef COLOR6
#undef COLOR7
#undef COLOR8
#undef COLOR9
#undef COLOR10
#undef COLOR11
#undef COLOR_SITE
#undef COLORBAR_MENU
#undef COLORBAR_BRD
#undef EDIT_BANNER
#undef ICON_GAMBLED_BRD
#undef ICON_VOTED_BRD
#undef ICON_NOTRAN_BRD
#undef ICON_TRAN_BRD
#undef GOODBYE_MSG


/* ----------------------------------------------------- */
/* 定義 BBS 站名位址					 */
/* ------------------------------------------------------*/

#define BBSNAME		"來客所"		/* 中文站名 */
#define BBSNAME2	"LexelBBS"		/* 英文站名 */
#define BBSNAME3	"來客所"		/* 短站名 */
#define SYSOPNICK	"sysop"			/* sysop 的暱稱 */

#define MYIPADDR	"140.114.87.7"		/* IP address */
#define MYHOSTNAME	"maplebbs.cs.nthu.edu.tw"	/* 網路地址 FQDN */
#define HOST_ALIASES	{MYHOSTNAME, MYIPADDR, \
			"lexel.twbbs.org", NULL}

#define BBSHOME		"/home/lexel"		/* BBS 的家 */
#define BAKPATH		"/home/lexel/bak"	/* 備份檔的路徑 */

#define BBSUID		1002
#define BBSGID		1002			/* Linux 請設為 999 */


  /* ------------------------------------------------- */
  /* 組態規劃˙註冊認證                                */
  /* ------------------------------------------------- */

#define HAVE_UFO2
#ifdef HAVE_UFO2
#  define HAVE_CHANGE_MODE	/* Bossliaw.081019: LEXEL 自訂/隱藏 動態 */
#  define HAVE_HIDE_FROM	/* Bossliaw.081019: LEXEL 自訂/隱藏 來源 */
				/* 要先 define HAVE_CHANGE_FROM */
#  define HAVE_LOGOUTY		/* bossliaw.081019: LEXEL- 離站顯示, 離站預設習慣 */
#endif

#undef POST_PREFIX	/* itoc.020113: 發表文章時標題可選擇種類 */
#undef HAVE_TEMPLATE

  /* ------------------------------------------------- */
  /* 組態規劃˙看板信箱                                */
  /* ------------------------------------------------- */

#ifdef HAVE_POPUPMENU
#undef HAVE_POPUPMENU		/* 蹦出式選單 */
#  undef POPUP_ANSWER		/* 蹦出式選單 -- 詢問選項 */
#  undef POPUP_MESSAGE		/* 蹦出式選單 -- 訊息視窗 */
#endif

#undef HAVE_MULTI_SIGN		/* 多種站簽供使用者選擇 */

#define MENU_FEAST		/* 選單狀態列額外顯示節日 */


 /* ------------------------------------------------- */
  /* 組態規劃˙外掛程式                                */
  /* ------------------------------------------------- */

#ifdef	HAVE_EXTERNAL
#  undef HAVE_GAME		/* itoc.010208: 提供遊戲 */
#endif


/* ----------------------------------------------------- */
/* 基本顏色定義，以利介面修改				 */
/* ----------------------------------------------------- */

#define COLOR1		"\033[1;33;42m"		/* footer 的前段顏色 */
#define COLOR2		"\033[m\033[30;47m"	/* footer 的後段顏色 */
#define COLOR3		"\033[;30;47m"
#define COLOR4		"\033[1;42m"		/* 光棒 的顏色 */
#define COLOR5		"\033[1;37;42m"		/* more 檔頭的標題顏色 */
#define COLOR6		"\033[0;34;47m"		/* more 檔頭的內容顏色 */
#define COLOR7		"\033[1m"		/* 作者在線上的顏色 */
#define COLOR8		"\033[m\033[32;47m"	/* footer 加強標註(綠/白) */
#define COLOR9		"\033[m\033[31;47m"	/* footer 加強標註(紅/白) */
#define COLOR10		"\033[30;47m"		/* neck 的顏色 */
#define COLOR11		"\033[31;47m"		/* menu footer 的顏色 */
#define COLOR_SITE	"\033[1;37;42m"		/* ryanlei.081017: 站台主色 */


/* ----------------------------------------------------- */
/* 各類光棒顏色						 */
/* ----------------------------------------------------- */

#define COLORBAR_MENU	"\033[1;37;40m"		/* menu.c 選單光棒 */
#define COLORBAR_BRD	"\033[1;42m"		/* board.c, favor.c 選單光棒 */


/* ----------------------------------------------------- */
/* 站台來源簽名						 */
/* ----------------------------------------------------- */

#define EDIT_BANNER "\n--\n" \
			"\033[;32m※ %s從 (\033[1;30m%s\033[0;32m) 上了 \033[1;37m" \
			"來客所 \033[0;32m<lexel.twbbs.org>\033[m\n"


/* ----------------------------------------------------- */
/* 其他訊息字串						 */
/* ----------------------------------------------------- */

#define ICON_GAMBLED_BRD	"＄"	/* 舉行賭盤中的看板 */
#define ICON_VOTED_BRD		"！"	/* 舉行投票中的看板 */
#define ICON_NOTRAN_BRD		"☆"			/* 不轉信板 */
#define ICON_TRAN_BRD		"★"			/* 轉信板 */
#define GOODBYE_MSG		"確定不上來客所了嗎 >///< ？[Q] "
#define GOODBYE_NMSG		"確定不上來客所了嗎 >///< ？[Y] "

#endif	/* SITE_LEXEL */
