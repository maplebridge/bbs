/*-------------------------------------------------------*/
/* util/snap.h						 */
/*-------------------------------------------------------*/
/* target : Maple 轉換					 */
/* create : 98/12/15					 */
/* update : 02/04/29					 */
/* author : itoc.bbs@bbs.tnfsh.tn.edu.tw		 */
/*-------------------------------------------------------*/


#if 0

  0. 保留原來 brd/ gem/ usr/ .USR，其餘換成新版的

  1. 利用 snap2brd 轉換 .BRD

  2. 利用 snap2usr 轉換 .ACCT

  3. 將新版的 gem/@/ 下的這些檔案複製過來
     @apply @e-mail @goodbye @index @justify @newuser @opening.0
     @opening.1 @opening.2 @post @re-reg @tryout @welcome

  4. 上 BBS 站，在 (A)nnounce 裡面，建以下二個卷宗的所有資料
     {話題} 熱門討論
     {排行} 統計資料

#endif


#include "bbs.h"

#define	MAK_DIRS	/* 建目錄 MF/ 及 gem/ */


/* ----------------------------------------------------- */
/* (舊的) 使用者帳號 .ACCT struct			 */
/* ----------------------------------------------------- */


typedef struct			/* 要和舊版程式 struct 一樣 */
{
  int userno;			/* unique positive code */
  char userid[13];
  char passwd[14];
  uschar signature;
  char realname[20];
  char username[24];
  usint userlevel;
  int numlogins;
  int numposts;
  usint ufo;
  time_t firstlogin;
  time_t lastlogin;
  time_t staytime;		/* 總共停留時間 */
  time_t tcheck;		/* time to check mbox/pal */
  char lasthost[32];
  int numemail;			/* 寄發 Inetrnet E-mail 次數 */
  time_t tvalid;		/* 通過認證、更改 mail address 的時間 */
  char email[60];
  char address[60];
  char justify[60];		/* FROM of replied justify mail */
  char vmail[60];		/* 通過認證之 email */
  char ident[140 -20];
/*  char cmode[20];                自訂動態 */
/*  char cfrom[34];                自訂來源 */
  time_t vtime;			/* validate time */
/*  int money;                     金錢 
  char pmail[60];                預設寄信信箱 
  char rmail[60];                預設轉記信箱 
  char pad[388];*/
}	userec;


/* ----------------------------------------------------- */
/* PAL : friend struct : 64 bytes                        */
/* ----------------------------------------------------- */


typedef struct
{ 
    char userid[IDLEN + 1];
    char ftype;
    char ship[46];
    int userno;
}      MAPLE_PAL;

#define PAL_BAD         0x02

/* ----------------------------------------------------- */
/* (舊的) 使用者習慣 ufo				 */
/* ----------------------------------------------------- */

/* old UFO */			/* 要和舊版程式 struct 一樣 */

#define HABIT_COLOR       BFLAG(0)        /* true if the ANSI color mode open */
#define HABIT_MOVIE       BFLAG(1)        /* true if show movie */
#define HABIT_BRDNEW      BFLAG(2)        /* 新文章模式 */
#define HABIT_BNOTE       BFLAG(3)        /* 顯示進板畫面 */
#define HABIT_VEDIT       BFLAG(4)        /* 簡化編輯器 */
#define HABIT_PAGER       BFLAG(5)        /* 關閉呼叫器 */
#define HABIT_QUIET       BFLAG(6)        /* 結廬在人境，而無車馬喧 */
#define HABIT_PAL         BFLAG(7)        /* true if show pals only */
#define HABIT_ALOHA       BFLAG(8)        /* 上站時主動通知好友 */
#define HABIT_MOTD        BFLAG(9)        /* 簡化進站畫面 */
#define HABIT_CLOAK       BFLAG(19)       /* true if cloak was ON */
#define HABIT_ACL         BFLAG(20)       /* true if ACL was ON */
/*#define HABIT_MPAGER      BFLAG(10)       /* lkchu.990428: 電子郵件傳呼 */
/*#define HABIT_NWLOG       BFLAG(11)       /* lkchu.990510: 不存對話紀錄 */
/*#define HABIT_NTLOG       BFLAG(12)       /* lkchu.990510: 不存聊天紀錄 */


/* ----------------------------------------------------- */
/* (舊的) BOARDS struct					 */
/* ----------------------------------------------------- */

typedef struct//Maple BBS
{
  char brdname[12 + 1];      /* board ID */
  char title[47 + 1];
  char bottom;
  char BM[36 + 1];
  char bclass[5];               /* 看板分類 */
  /*char bholiday[5];             看板日 */
  /*char BM[36 + 1];           BMs' uid, token '/' */
          
  uschar bvote;                 /* 共有幾項投票舉行中 */
             
  time_t bstamp;                /* 建立看板的時間, unique */
  usint readlevel;              /* 閱讀文章的權限 */
  usint postlevel;              /* 發表文章的權限 */
  usint battr;                  /* 看板屬性 */
  time_t btime;                 /* .DIR 的 st_mtime */
  int bpost;                    /* 共有幾篇 post */
  time_t blast;                 /* 最後一篇 post 的時間 */
  char pad[120];
} 	boardheader;

/* ----------------------------------------------------- */
/* (舊的) HDR struct					 */
/* ----------------------------------------------------- */
typedef struct
{
  time_t chrono;		/* timestamp */
  int xmode;

  int xid;			/* reserved */

  char xname[28];		/* 檔案名稱 */
  
  time_t ochrono;		/* 最原始的 timestamp */
  
  char owner[80];		/* 作者 (E-mail address) */
  char nick[50];		/* 暱稱 */

  char date[9];			/* [96/12/01] */
  /* Thor.990329:特別注意, date只供顯示用, 不作比較, 以避免 y2k 問題,
                 定義 2000為 00, 2001為01 */

  char title[73];		/* 主題 (TTLEN + 1) */

  int score;
  
}          LEXEL_HDR;

typedef struct
{
  time_t chrono;		/* timestamp */
  int xmode;

  int xid;			/* reserved */

  char xname[32];		/* 檔案名稱 */
  char owner[80];		/* 作者 (E-mail address) */
  char nick[50];		/* 暱稱 */

  char date[9];			/* [96/12/01] */
  /* Thor.990329:特別注意, date只供顯示用, 不作比較, 以避免 y2k 問題,
                 定義 2000為 00, 2001為01 */

  char title[73];		/* 主題 (TTLEN + 1) */
}         MAPLECS_HDR;


typedef struct
{
  time_t chrono;                /* timestamp */
  char buf1[4];
  int xmode;
                                                                                
  int xid;                      /* reserved */
                                                                                
  char xname[32];
  char owner[76];
  time_t stamp;
  char nick[49];
  char score;
                                                                                
  char date[9];                 /* 96/12/31 */
                                                                                
  char title[73];
  char buf2[4];
}     WRETCH_HDR;


/* ----------------------------------------------------- */
/* (舊的) GEM struct                                        */
/* ----------------------------------------------------- */
typedef struct
{
  time_t chrono;		/* timestamp */
  int xmode;
  int xid;			/* reserved */
  char xname[28];		/* 檔案名稱 */
  time_t ochrono;		/* 最原始的 timestamp */
  char owner[80];		/* 作者 (E-mail address) */
  char nick[50];		/* 暱稱 */
  char date[9];			/* [96/12/01] */
  /* Thor.990329:特別注意, date只供顯示用, 不作比較, 以避免 y2k 問題,
     定義 2000為 00, 2001為01 */
  char title[73];		/* 主題 (TTLEN + 1) */
  int score;
  }          LEXEL_GEM;



/*----------舊的MF----------*/
typedef struct
{
  time_t chrono;/*建立時間*/
  usint mftype;/*type*/
  int bno;/*board #*/
/*  char title[38+1];/*標題*/  
  char xname[13];/*檔名*/
  char title[38+1];/*標題*/
} MyF;
#define OLDMF_LINE	0x00000001/*分隔線*/
#define OLDMF_BOARD	0x00000002/*看板捷徑*/
#define OLDMF_FOLDER	0x00000004/*卷宗*/
#define OLDMF_GEM	0x00000008/*精華區捷徑*/
#define OLDMF_DELETED	0x10000000/*被移除的連結*/

