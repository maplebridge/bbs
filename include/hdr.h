/*-------------------------------------------------------*/
/* lib/hdr.h            ( NTHU CS MapleBBS Ver 3.00 )    */
/*-------------------------------------------------------*/
/* target : header file for HDR routines                 */
/* create : 96/11/20                                     */
/* update : 96/12/15                                     */
/*-------------------------------------------------------*/


#ifndef	_HDR_H_
#define	_HDR_H_


#include <sys/types.h>


#include "time4.h"


/* ----------------------------------------------------- */
/* DIR of post / mail struct : 256 bytes		 */
/* ----------------------------------------------------- */


typedef struct
{
  time4_t chrono;		/* timestamp */
  int xmode;

  int xid;			/* reserved */

  char xname[28];               /* 檔案名稱 */
  time4_t parent_chrono;         /* 置底文的正本是哪篇 */
  char owner[76];               /* 作者 (E-mail address) */
  time4_t stamp;                 /* 最近一次更改檔案的時間 */
  char nick[40];		/* 暱稱 */
  char value[9];		/* 文章價值/匿名文章userno */
  char score;			/* 文章評比分數 */

  char date[9];			/* 96/12/31 */
  /* Thor.990329:特別注意, date只供顯示用, 不作比較, 以避免 y2k 問題,
                 定義 2000為 00, 2001為01 */

  char title[73];		/* 主題 TTLEN + 1 */
}          HDR;


/* gopher url 字串：xname + owner + nick + date */

#define	GEM_URLEN	(32 + 80 + 50 + 9 - 1)


/* ----------------------------------------------------- */
/* post.xmode 的定義					 */
/* ----------------------------------------------------- */


#define POST_READ	0x00000001	/* already read */
#define POST_MARKED	0x00000002	/* marked */
#define POST_NOFORWARD	0x00000004	/* 不能轉寄 */
#define POST_NOSCORE	0x00000008	/* 不能評分 */
#define POST_GOOD	0x00000010	/* 優文 */
#define POST_GEM	0x00000020	/* 收錄精華區標記 */
#define POST_BOTTOM	0x00000040	/* 置底 */
#define POST_DELETE	0x00000080	/* 標記待砍的 */
#define	POST_INCOME	0x00000100	/* 轉信進來的 */
#define	POST_FRIEND	0x00000200	/* 好友文章 */
#define	POST_OUTGO	0x00000400	/* 須轉信出去 */
#define	POST_RESTRICT	0x00000800	/* 限制級文章，須 manager/owner 才能看 */
#define POST_RESERVED	0x00001000	/* 限制級文章，須 sysop 才能更改 */
#define POST_14		0x00002000
#define POST_SCORE	0x00004000	/* 標記評分過的 */
#define POST_16		0x00008000


/* ----------------------------------------------------- */
/* mail.xmode 的定義					 */
/* ----------------------------------------------------- */


#define MAIL_READ	0x00000001	/* already read */
#define MAIL_MARKED	0x00000002	/* marked */
#define MAIL_REPLIED	0x00000004	/* 已經回過信了 */
#define	MAIL_MULTI	0x00000008	/* mail list */
#define	MAIL_5		0x00000010
#define	MAIL_NOREPLY	0x00000020	/* 不可回信 */
#define MAIL_BOTTOM	0x00000040	/* 置底 */
#define MAIL_DELETE	0x00000080	/* 將遭刪除 */
#define	MAIL_INCOME	0x00000100	/* bbsmail 進來的 */
#define MAIL_10		0x00000200
#define MAIL_11		0x00000400
#define	MAIL_RESTRICT	0x00000800	/* 限制級信件，須 manager/owner 才能看 */
#define MAIL_RESERVED	0x00001000	/* 限制級信件，須 sysop 才能更改 */
#define MAIL_14		0x00002000
#define MAIL_15		0x00004000
#define MAIL_16		0x00008000


/* ----------------------------------------------------- */
/* gem(gopher).xmode 的定義				 */
/* ----------------------------------------------------- */


/* itoc.010602.註解: GEM_RESTRICT/RESERVED 和 POST_RESTRICT/RESERVED 要匹配 */
#define	GEM_RESTRICT	0x00000800	/* 限制級精華區，須 manager/owner 才能看 */
#define	GEM_RESERVED	0x00001000	/* 限制級精華區，須 sysop 才能更改 */
#define GEM_FULLPATH    0x00008000      //lexel 有 itoc 沒有/* 絕對路徑 */
#define	GEM_FOLDER	0x00010000	/* folder / article */
#define	GEM_BOARD	0x00020000	/* 看板精華區 */
#define GEM_19		0x00040000
#define GEM_LINE	0x00080000	/* 分隔線 */
#define GEM_GOPHER      0x10000000	//lexel 有 itoc 沒有/* gopher */
#define GEM_HTTP        0x20000000	//lexel 有 itoc 沒有/* http */
#define GEM_EXTEND      0x80000000	//lexel 有 itoc 沒有/* 延伸式 URL */
#define HDR_URL		(GEM_GOPHER | GEM_HTTP)	//lexel 有 itoc 沒有



/* ----------------------------------------------------- */
/* hdr_stamp() 的定義					 */
/* ----------------------------------------------------- */


#define	HDR_LINK	0x400		/* link() */
#define HDR_COPY	0x800		/* copy */

#endif	/* _HDR_H_ */
