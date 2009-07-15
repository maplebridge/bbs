/*-------------------------------------------------------*/
/* theme.h	( NTHU CS MapleBBS Ver 3.10 )		 */
/*-------------------------------------------------------*/
/* target : custom theme				 */
/* create : 02/08/17				 	 */
/* update :   /  /  				 	 */
/*-------------------------------------------------------*/


#ifndef	_THEME_H_
#define	_THEME_H_


/* ----------------------------------------------------- */
/* 基本顏色定義，以利介面修改				 */
/* ----------------------------------------------------- */

//#define COLOR1		"\033[34;46m"	/* footer/feeter 的前段顏色 */
#define COLOR1		"\033[;1;33;44m"	/* footer/feeter 的前段顏色 */
#define COLOR2		"\033[;30;47m"		/* footer/feeter 的後段顏色 */
#define COLOR3		"\033[;30;47m"	
#define COLOR4		"\033[1;44m"		/* 光棒 的顏色 */
#define COLOR5		"\033[34;47m"		/* more 檔頭的標題顏色 */
#define COLOR6		"\033[37;44m"		/* more 檔頭的內容顏色 */
#define COLOR7		"\033[1m"		/* 作者在線上的顏色 */
#define COLOR8		"\033[;1;34;47m"	/* feeter 加強標註(藍/白) */
#define COLOR9		"\033[;31;47m"		/* feeter 加強標註(紅/白) */
#define COLOR10		"\033[34;47m"		/* neck 的顏色 */
#define COLOR11		"\033[35;47m"		/* menu feeter 的顏色 */
#define COLOR_SITE	"\033[1;37;44m"		/* ryanlei.081017: 站台主色 */


/* ----------------------------------------------------- */
/* 使用者名單顏色					 */
/* ----------------------------------------------------- */

#define COLOR_NORMAL	""		/* 一般使用者 */
#define COLOR_MYBAD	"\033[1;31m"	/* 壞人 */
#define COLOR_MYGOOD	"\033[1;32m"	/* 我的好友 */
#define COLOR_OGOOD	"\033[1;33m"	/* 與我為友 */
#define COLOR_CLOAK	"\033[1;35m"	/* 隱形 */	/* itoc.註解: 沒用到，要的人請自行加入 ulist_body() */
#define COLOR_SELF	"\033[1;36m"	/* 自己 */
#define COLOR_BOTHGOOD	"\033[1;37m"	/* 互設好友 */
#define COLOR_BRDMATE	"\033[36m"	/* 板伴 */
#define COLOR_SUPER_BOTHGOOD	"\033[1;35m"	/* 互設超級好友 */
#define COLOR_SUPER_MYGOOD	"\033[32m"	/* 我的超級好友 */
#define COLOR_SUPER_OGOOD	"\033[33m"	/* 與我為超級好友 */


/* ----------------------------------------------------- */
/* 各類光棒顏色						 */
/* ----------------------------------------------------- */

#define COLORBAR_MENU	"\033[1;37;44m"	/* menu.c 選單光棒 */
#define	COLORBAR_BRD	"\033[1;41m"	/* board.c, favor.c 選單光棒 */
#define COLORBAR_POST	"\033[1;43m"	/* post.c 選單光棒 */
#define COLORBAR_GEM	"\033[1;42m"	/* gem.c  選單光棒 */
#define COLORBAR_PAL	"\033[1;45m"	/* pal.c  選單光棒 */
#define COLORBAR_USR	"\033[1;45m"	/* ulist.c 選單光棒 */
#define COLORBAR_BMW	"\033[1;43m"	/* bmw.c 選單光棒 */
#define COLORBAR_MAIL	"\033[1;42m"	/* mail.c 選單光棒 */
#define COLORBAR_ALOHA	"\033[1;41m"	/* aloha.c 選單光棒 */
#define COLORBAR_VOTE	"\033[;30;43m"	/* vote.c 選單光棒 */
#define COLORBAR_NBRD	"\033[1;46m"	/* newbrd.c 選單光棒 */
#define COLORBAR_SONG	"\033[1;42m"	/* song.c 選單光棒 */
#define COLORBAR_RSS	"\033[1;46m"	/* rss.c 選單光棒 */

/* ----------------------------------------------------- */
/* 選單位置						 */
/* ----------------------------------------------------- */

/* itoc.註解: 注意 MENU_XPOS 要 >= MENU_XNOTE + MOVIE_LINES */

#define MENU_XNOTE	2		/* 動態看板由 (2, 0) 開始 */
#define MOVIE_LINES	10		/* 動畫最多有 10 列 */

#define MENU_XPOS	13		/* 選單開始的 (x, y) 座標 */
#define MENU_YPOS	((d_cols >> 1) + 18)


/* ----------------------------------------------------- */
/* 訊息字串：*_neck() 時的 necker 都抓出來定義在這	 */
/* ----------------------------------------------------- */

/* necker 的行數都是二行，從 (1, 0) 到 (2, 80) */

/* 所有的 XZ_* 都有 necker，只是有些在 *_neck()，有些藏在 *_head() */

/* ulist_neck() 及 xpost_head() 的第一行比較特別，不在此定義 */

#define NECKER_CLASS	"[←]主選單 [→]閱\讀 [↑↓]選擇 [c]篇數 [y]載入 [/?]搜尋 [s]看板 [h]說明\n" \
			COLOR10 "  %s   看  板       類別轉信中   文   敘   述%*s              人氣 板    主%*s    \033[m"

#define NECKER_ULIST	"\n" \
			COLOR10 "  編號  代號         暱稱%*s                 %-*s               動態        閒置 \033[m"

#define NECKER_PAL	"[←]離開 [a]新增 [c]修改 [d]刪除 [m]寄信 [w]水球 [s]整理 [→]查詢 [h]說明\n" \
			COLOR10 "  編號    代 號         友       誼%*s                                           \033[m"

#define NECKER_ALOHA	"[←]離開 [a]新增 [d]刪除 [D]區段刪除 [m]寄信 [w]水球 [s]重整 [f]引入 [h]說明\n" \
			COLOR10 "  編號   上 站 通 知 名 單%*s                                                    \033[m"

#define NECKER_VOTE	"[←]離開 [R]結果 [^P]舉行 [E]修改 [V]預覽 [^Q]改期 [o]名單 [h]說明\n" \
			COLOR10 "  編號      開票日   主辦人       投  票  宗  旨%*s                              \033[m"

#define NECKER_BMW	"[←]離開 [d]刪除 [D]區段刪除 [m]寄信 [M]儲存 [w]水球 [s]更新 [→]查詢 [h]說明\n" \
			COLOR10 "  編號 代  號       內       容%*s                                          時間 \033[m"

#define NECKER_MF	"[←]離開 [→]進入 [^P]新增 [d]刪除 [c]切換 [C]複製 [^V]貼上 [m]移動 [h]說明\n" \
			COLOR10 "  %s   看  板       類別轉信中   文   敘   述%*s              人氣 板    主%*s    \033[m"

#define NECKER_COSIGN	"[←]離開 [→]閱\讀 [^P]申請 [d]刪除 [o]開板 [h]說明\n" \
			COLOR10 "  編號   日 期  舉辦人       看  板  標  題%*s                                   \033[m"

#define NECKER_SONG	"[←]離開 [→]瀏覽 [o]點歌到看板 [m]點歌到信箱 [Enter]瀏覽 [h]說明\n" \
			COLOR10 "  編號     主              題%*s                            [編      選] [日  期]\033[m"

#define NECKER_NEWS	"[←]離開 [→]閱\讀 [h]說明\n" \
			COLOR10 "  編號     日 期 作  者       新  聞  標  題%*s                                  \033[m"

#define NECKER_XPOST	"\n" \
			COLOR10 "  編號     日 期 作  者       文  章  標  題%*s                                  \033[m"

#define NECKER_MBOX	"[←]離開 [→,r]讀信 [d]刪除 [R,y](群組)回信 [s]寄信 [x]轉錄 [X]轉達 [h]說明\n" \
			COLOR10 "  編號   日 期 作  者       信  件  標  題%*s                                    \033[m"

#define NECKER_POST	"[←]離開 [→]閱\讀 [^P]發表 [b]進板畫面 [d]刪除 [V]投票 [TAB]精華區 [h]說明\n" \
			COLOR10 "  編號     日 期 作  者       文  章  標  題%*s                        人氣:%-4d \033[m"

#define NECKER_GEM	"[←]離開 [→]瀏覽 [B]模式 [C]暫存 [F]轉寄 [d]刪除 [h]說明  %s\n" \
			COLOR10 "  編號     主              題%*s                            [編      選] [日  期]\033[m"

/* 以下這些則是一些類 XZ_* 結構的 necker */

#define NECKER_VOTEALL	"[↑/↓]上下 [PgUp/PgDn]上下頁 [Home/End]首尾 [→]投票 [←][q]離開\n" \
			COLOR10 "  編號   看  板       類別轉信中   文   敘   述%*s                  板    主%*s     \033[m"

#define NECKER_CREDIT	"[←]離開 [C]換頁 [1]新增 [2]刪除 [3]全刪 [4]總計\n" \
			COLOR10 "  編號   日  期   收支  金  額  分類     說  明%*s                               \033[m"

#define NECKER_HELP	"[←]離開 [→]閱\讀 [^P]新增 [d]刪除 [T]標題 [E]編輯 [m]移動\n" \
			COLOR10 "  編號    檔 案         標       題%*s                                           \033[m"

#define NECKER_INNBBS	"[←]離開 [^P]新增 [d]刪除 [E]編輯 [/]搜尋 [Enter]詳細\n" \
			COLOR10 "  編號            內         容%*s                                               \033[m"

#define NECKER_RSS	"[←]離開 [→]瀏覽 [o]隱藏 [u]UTF8編碼 [n]Html/Txt [s]啟動/暫停 [R]重送        \n" \
			COLOR10 "  編號  日 期%*s 標  簽       屬 性    網址                                      \033[m"

/* ----------------------------------------------------- */
/* 訊息字串：more() 時的 footer 都抓出來定義在這	 */
/* ----------------------------------------------------- */

/* itoc.010914.註解: 單一篇，所以叫 FOOTER，都是 78 char */

/* itoc.010821: 注意 \\ 是 \，最後別漏了一個空白鍵 :p */

#define FOOTER_POST	\
COLOR1 " 文章選讀 " COLOR9 " (ry)"COLOR2"回應 "COLOR9"(=\\[]<>-+;'`)"COLOR2"主題 "COLOR9"(|?QA)"COLOR2"搜尋標題作者 "COLOR9"(kj)"COLOR2"上下篇 "COLOR9"(C)"COLOR2"暫存   "

#define FOOTER_MAILER	\
COLOR1 " 魚雁往返 " COLOR9 " (ry)"COLOR2"回信/群組 "COLOR9"(X)"COLOR2"轉達 "COLOR9"(d)"COLOR2"刪除 "COLOR9"(m)"COLOR2"標記 "COLOR9"(C)"COLOR2"暫存 "COLOR9"(=\\[]<>-+;'`|?QAkj)"COLOR2"  "

#define FOOTER_GEM	\
COLOR1 " 精華選讀 " COLOR9 " (=\\[]<>-+;'`)"COLOR2"主題 "COLOR9"(|?QA)"COLOR2"搜尋標題作者 "COLOR9"(kj)"COLOR2"上下篇 "COLOR9"(↑↓←)"COLOR2"上下離開   "

#ifdef HAVE_GAME
#define FOOTER_TALK	\
COLOR1 " 交談模式 " COLOR9 " (^O)"COLOR2"對奕模式 "COLOR9"(^C,^D)"COLOR2"結束交談 "COLOR9"(^T)"COLOR2"切換呼叫器 "COLOR9"(^Z)"COLOR2"快捷列表 "COLOR9"(^G)"COLOR2"嗶嗶  "
#else
#define FOOTER_TALK	\
COLOR1 " 交談模式 " COLOR9 " (^C,^D)"COLOR2"結束交談 "COLOR9"(^T)"COLOR2"切換呼叫器 "COLOR9"(^Z)"COLOR2"快捷列表 "COLOR9"(^G)"COLOR2"嗶嗶 "COLOR9"(^Y)"COLOR2"清除      "
#endif

#define FOOTER_COSIGN	\
COLOR1 " 連署機制 " COLOR9 " (ry)"COLOR2"加入連署 "COLOR9"(kj)"COLOR2"上下篇 "COLOR9"(↑↓←)"COLOR2"上下離開 "COLOR9"(h)"COLOR2"說明                   " 

//#define FOOTER_MORE	\
//COLOR1 " 瀏覽 P.%d (%d%%) " COLOR2 " (h)說明 [PgUp][PgDn][0][$]移動 (/n)搜尋 (C)暫存 (←q)結束 "

#define FOOTER_MORE	\
COLOR1 " 瀏覽 P.%d (%d%%) " COLOR9 " (h)"COLOR2"說明 " COLOR9 "(@)"COLOR2"動畫播放 "COLOR9"(↑↓0$)"COLOR2"移動 "COLOR9"(/n)"COLOR2"搜尋 "COLOR9"(C)"COLOR2"暫存 "COLOR9"(←q)"COLOR2"結束  "

#define FOOTER_VEDIT	\
COLOR1 " %s " COLOR9 " (^Z)"COLOR2"說明 "COLOR9"(^W)"COLOR2"符號 "COLOR9"(^L)"COLOR2"重繪 "COLOR9"(^X)"COLOR2"檔案處理 ║"COLOR9"%s"COLOR2"│"COLOR9"%s"COLOR2"║"COLOR9"%5d:%3d"COLOR2"    \033[m"

#define FOOTER_VEDIT_RONLY	\
"%.0s\033[1;5m             唯讀模式                 \033[m"COLOR9"(^X)"COLOR2"檔案處理 ║"COLOR9"%s"COLOR2"│"COLOR9"%s"COLOR2"║"COLOR9"%5d:%3d"COLOR2"    \033[m"

/* ----------------------------------------------------- */
/* 訊息字串：xo_foot() 時的 feeter 都抓出來定義在這      */
/* ----------------------------------------------------- */


/* itoc.010914.註解: 列表多篇，所以叫 FEETER，都是 78 char */

#define FEETER_CLASS	\
COLOR1 " 看板選擇 " COLOR9 " (c)"COLOR2"新文章 "COLOR9"(vV)"COLOR2"標記已讀未讀 "COLOR9"(y)"COLOR2"全部列出 "COLOR9"(z)"COLOR2"選訂 "COLOR9"(A)"COLOR2"全域搜尋 "COLOR9"(S)"COLOR2"排序 "

#define FEETER_ULIST	\
COLOR1 " 網友列表 " COLOR9 " (f)"COLOR2"好友 "COLOR9"(w/t/m)"COLOR2"水球/聊天/寄信 "COLOR9"(q)"COLOR2"查詢 "COLOR9"(ad)"COLOR2"交友 "COLOR9"(s)"COLOR2"更新 "COLOR9"(TAB)"COLOR2"切換   "

#define FEETER_PAL	\
COLOR1 " 呼朋引伴 " COLOR9 " (a/d/m)"COLOR2"新增/刪除/寄信 "COLOR9"(c)"COLOR2"友誼 "COLOR9"(f)"COLOR2"引入好友 "COLOR9"(r^Q)"COLOR2"查詢 "COLOR9"(s)"COLOR2"更新        "

#define FEETER_ALOHA	\
COLOR1 " 上站通知 " COLOR9 " (a)"COLOR2"新增 "COLOR9"(d)"COLOR2"刪除 "COLOR9"(D)"COLOR2"區段刪除 "COLOR9"(f)"COLOR2"引入好友 "COLOR9"(r^Q)"COLOR2"查詢 "COLOR9"(s)"COLOR2"更新          "

#define FEETER_VOTE	\
COLOR1 " 看板投票 " COLOR9 " (→/r/v)"COLOR2"投票 "COLOR9"(R)"COLOR2"結果 "COLOR9"(^P)"COLOR2"新增投票 "COLOR9"(E)"COLOR2"修改 "COLOR9"(V/b/o)"COLOR2"預覽/開票/名單    "

#define FEETER_BMW	\
COLOR1 " 水球回顧 " COLOR9 " (d/D)"COLOR2"刪除/區段刪除 "COLOR9"(m)"COLOR2"寄信 "COLOR9"(w)"COLOR2"水球 "COLOR9"(^R)"COLOR2"回訊 "COLOR9"(^Q)"COLOR2"查詢 "COLOR9"(s)"COLOR2"更新       "

#define FEETER_MF	\
COLOR1 " 最愛看板 " COLOR9 " (^P)"COLOR2"新增 "COLOR9"(Cg)"COLOR2"複製 "COLOR9"(p^V)"COLOR2"貼上 "COLOR9"(d)"COLOR2"刪除 "COLOR9"(c)"COLOR2"新文章 "COLOR9"(vV)"COLOR2"標記已讀/未讀    "

#define FEETER_COSIGN	\
COLOR1 " 連署小站 " COLOR9 " (r/y)"COLOR2"讀取/回應 "COLOR9"(^P)"COLOR2"發表 "COLOR9"(d)"COLOR2"刪除 "COLOR9"(o)"COLOR2"開板 "COLOR9"(c)"COLOR2"關閉 "COLOR9"(E/B)"COLOR2"編輯/設定     "

#define FEETER_SONG	\
COLOR1 " 點歌系統 " COLOR9 " (r)"COLOR2"讀取 "COLOR9"(o)"COLOR2"點歌到看板 "COLOR9"(m)"COLOR2"點歌到信箱 "COLOR9"(E)"COLOR2"編輯檔案 "COLOR9"(T)"COLOR2"編輯標題        "

#define FEETER_NEWS	\
COLOR1 " 新聞點選 " COLOR9 " (↑/↓)"COLOR2"上下 "COLOR9"(PgUp/PgDn)"COLOR2"上下頁 "COLOR9"(Home/End)"COLOR2"首尾 "COLOR9"(→r)"COLOR2"選取 "COLOR9"(←)(q)"COLOR2"離開 "

#define FEETER_XPOST	\
COLOR1 " 串列搜尋 " COLOR9 " (y/x)"COLOR2"回應/轉錄  "COLOR9"(m/t)"COLOR2"標記/標籤 "COLOR9"(d)"COLOR2"刪除 "COLOR9"(^P)"COLOR2"發表 "COLOR9"(^Q)"COLOR2"查詢作者       "

#define FEETER_MBOX	\
COLOR1 " 信信相惜 " COLOR9 " (y)"COLOR2"回信 "COLOR9"(F/X/x)"COLOR2"轉寄/轉達/轉錄 "COLOR9"(d)"COLOR2"刪除 "COLOR9"(D)"COLOR2"區段刪除 "COLOR9"(m)"COLOR2"標記 "COLOR9"(E)"COLOR2"編輯  "

#define FEETER_POST	\
COLOR1 " 文章列表 \033[m" COLOR9 " (f)"COLOR8"編號/檔名切換\033[m" COLOR9 " (ry)"COLOR2"回信 " COLOR9 "(/)"COLOR8"搜尋功\能整合鍵\033[m" COLOR9 " (x/V/u)"COLOR2"轉錄/投票/新聞  "

#define FEETER_GEM	\
COLOR1 " 看板精華 " COLOR9 " (^P/a/f)"COLOR2"新增/文章/目錄 "COLOR9"(E)"COLOR2"編輯 "COLOR9"(T)"COLOR2"標題 "COLOR9"(m)"COLOR2"移動 "COLOR9"(c)"COLOR2"複製 "COLOR9"(p^V)"COLOR2"貼上   "

#define FEETER_VOTEALL	\
COLOR1 " 投票中心 " COLOR9 " (↑/↓)"COLOR2"上下 "COLOR9"(PgUp/PgDn)"COLOR2"上下頁 "COLOR9"(Home/End)"COLOR2"首尾 "COLOR9"(→)"COLOR2"投票 "COLOR9"(←)(q)"COLOR2"離開  "

#define FEETER_HELP	\
COLOR1 " 說明文件 " COLOR9 " (↑/↓)"COLOR2"上下 "COLOR9"(PgUp/PgDn)"COLOR2"上下頁 "COLOR9"(Home/End)"COLOR2"首尾 "COLOR9"(→r)"COLOR2"瀏覽 "COLOR9"(←)(q)"COLOR2"離開 "

#define FEETER_INNBBS	\
COLOR1 " 轉信設定 " COLOR9 " (↑/↓)"COLOR2"上下 "COLOR9"(PgUp/PgDn)"COLOR2"上下頁 "COLOR9"(Home/End)"COLOR2"首尾 "COLOR9"(←)(q)"COLOR2"離開           "

#define FEETER_BITLBEE \
COLOR1 "  即時通  "COLOR9" (w)"COLOR2"傳送訊息 " COLOR9 "(^r)"COLOR2"回覆訊息 " COLOR9 "(a/d)"COLOR2"增/刪聯絡人 " COLOR9 "(b/B)"COLOR2"封鎖/解除 "COLOR9"(^k)"COLOR2"斷線  " 

#define FEETER_FAKE_PAL	\
COLOR1 " 瀏覽名單 " COLOR9 " 本名單僅供瀏覽，板主修改板友名單請退出後按 o 選擇修改              "

#define FEETER_RSS	\
COLOR1 " RSS 設定 " COLOR9 " (^P/a)"COLOR2"新增 "COLOR9"(d)"COLOR2"刪除 "COLOR9"(E)"COLOR2"編輯 "COLOR9"(T)"COLOR2"標簽名稱 "COLOR9"(H)"COLOR2"RSS網址 "COLOR9"(m)"COLOR2"移動 "COLOR9"(h)"COLOR2"說明  "

/* ----------------------------------------------------- */
/* 站台來源簽名						 */
/* ----------------------------------------------------- */

/* itoc: 建議 banner 不要超過三行，過長的站簽可能會造成某些使用者的反感 */

/*dexter  把origin,From改成兩行*/
#define EDIT_BANNER	"\n--\n" \
"\033[;32m※ Origin: 楓橋驛站<bbs.cs.nthu.edu.tw>\n\033[;32m◆ From: %-*.*s\033[m\n"

#define MODIFY_BANNER	"\033[;36m%s 於 %s 從 %s 修改\033[m\n"

#define EDIT_BANNER_1	"\n--\n" \
"              \033[;30m\242\033[47m\253 \033[m◣  \033[31m─\033[1;37m 楓橋藝文站正式開張！\033[;31m─\033[1;31m\243\033[;31m\273─\033[1;31m\243\033[;31m\273─\033[30;41m▍\033[m \033[1;36m\267\033[;36m\254\033[1;36m\276\033[;36m\364\033[1;36m\305\033[;36m\346\033[1;36m\257\033[;36m\270\033[1;36m\243\033[;36m\273\033[m\n" \
"   \033[1;37m竟然不是 ‵\033[m \033[30;45m◣\033[47m︹\033[m▉ \033[1;36m　　　　　　　　　　\033[m            \033[30;41m▋ │\033[31;40m▉▄▅▆\033[m\n" \
"   \033[1;37m紅蘿蔔！\033[m▄▅▇\033[1;30;47m \033[m;\033[47m \033[;30;47m \033[m \033[1;36m快來尋找你愛的作家。\033[m       \033[1;31m▃▆\033[41m◤\033[;30;41m ／＼\033[31m     \033[36m◢\033[46m    \033[30;47m◥\033[m\n" \
"          \033[30m \033[47m◣▃\033[45m\242\033[37;47m\146 \033[1;31m\241\033[41m\100\033[;30;47m\242\033[40m\250\033[36m為你嘔心瀝血的作品找個窩。\033[37m \033[30;41m◣       \033[31m\242\033[36m\251\033[30m   \033[31m◢\033[30;46m＝＝＝\033[47m \033[37m\241\033[33;40m\273\033[m\n" \
"                 \033[30;47m ◥\033[m  \033[1;36m文學版＋美工板，強烈邀請您。\033[m \033[30;41m◣▁▂  \033[36;46m \033[41m▆\033[46m         \033[30;47m◢\033[m\n" \
"      \033[30;42m\242\033[32m\251     \033[37m   \241\033[47m\264\033[30m.\033[37m\241\033[42m\264 %-*.*s\033[30m \033[37m \033[31m◢\033[32;41m◢\033[42m          \242\033[30m\252\033[m\n"

#define EDIT_BANNER_2	"\n--\n" \
"  ▄       ◢ ▄▄▄ ▄▄▄ ▄     ▄▄▄ \033[1;37m 清大資工\033[m\n" \
"\033[1;37;44m  █ █◣◢█ █▄█ █▄█ █     █▄▄  \033[33m%-*.*s\033[m\n" \
"  █ █◥◤█ █  █ █     █▄▄ █▄▄ \033[1;37m【楓橋驛站】 telnet://imaple.tw\033[m\n"

#define EDIT_BANNER_3	"\n--\n" \
"\033[;30m\242\033[44m\143_\241\033[1;30m\304ˍ\033[33m（_\033[;34m▇▆\033[1;31;44m' * \033[;34;44m\242\033[31m\251_ \033[34;41m\242\033[31;44m\251\242\033[41m\250\033[34;40m▆▇\033[30;44m \033[37m▁_\033[30m ▄▆▇\033[m\241\033[33m\103\033[31m.\033[1;34m楓橋驛站\241\033[37m\104\033[30mtelnet://imaple.tw\033[;30m\241\033[41m\273\033[31;40m\242\033[41m\251\033[40m}\033[m\n" \
" \033[34m=\033[37m\242\033[44m\142  ▔﹊\033[30m\241\033[34m\303   \033[31m*\033[33m.   \033[34;41m▆\033[1;30;44m\244\033[;30;41m\164@\033[31;44m▋ \033[30m▃\033[m▔   \033[1;35m▂~\033[5m+\033[;33m                                  \033[30m\242\033[31m\253\033[1;31m\244\033[;30m\241\033[m\n" \
"   ￣\033[30;44m▃▂▁▂\241\033[31m\103\033[30m▁▂ˍ\033[31m_\242\033[41m\250\033[30m〢\033[44m_▇\033[33;40m.\033[31m*   \033[1;35m├=rom：\033[37m%-*.*s\033[m\n" \
"   \033[31m﹊￣﹊￣￣﹊﹊￣￣﹊￣﹊￣￣﹊\033[35m@\244\033[30m\110\033[31m￣￣﹊￣﹊￣￣﹊﹊￣￣﹊﹊￣￣﹊﹊￣﹊￣\033[m\n"

#define EDIT_BANNER_MEICHU_WIN	"\n--\n" \
"\033[42m   \033[0;30;42m◢\033[0;42m▏\033[0;30;42m◢\033[0;42m▏ \033[0;31;43m  我要  \033[0;42m                                                  \033[0;30;42m(((逃  \033[m\n" \
"\033[42m \033[0;32;47m▉\033[1;37;40m▇█▇\033[42m◣ \033[0;31;43m 吃竹子 \033[42m  \033[0;33m   己丑梅竹賽將於 3/6 ~ 3/8 展開    \033[0;31;42m          ◣  ◣   \033[m\n" \
"\033[42m \033[0;32;47m▊\033[1;37;40m███\033[1;37m -\033[1;37;42m▏   \033[0;33;42m▎                                                  \033[0;30;41m @) @)\033[0;31;40m\033[42m   \033[m\n" \
"\033[42m \033[1;37;42m █████\033[42m    \033[0;33;42m▎    \033[0;33m 楓橋驛站邀請您  一同為清華大學加油 \033[0;31;42m    @     \033[0;31;42m◥\033[0;30;41m人\033[0;31;42m◤   \033[m\n" \
"\033[0;32;47m▋\033[1;37;42m████◤  \033[1;37;42m,,\033[0;33;42m▎                                             \033[0;31;42m\\◢██\033[42m\033[1;30m█\033[0;31;42m▎   \033[m\n" \
"\033[0;32;47m▌\033[1;37;40m█◤    \033[0;32m◥\033[0;30;42m◢█\033[m\033[42m     \033[1;34m From:%-*.*s      \033[0;31;42m/\\     /\\     \033[m\n"

/* ----------------------------------------------------- */
/* 其他訊息字串						 */
/* ----------------------------------------------------- */

#define VMSG_NULL	"                           " COLOR1 " ● 請按任意鍵繼續 ● \033[m"

#define ICON_UNREAD_BRD		"\033[1;33m‧\033[m"	/* 未讀看板 */
#define ICON_READ_BRD		"  "			/* 已讀看板 */

#define ICON_GAMBLED_BRD	"\033[1;31m賭\033[m"	/* 舉行賭盤中的看板 */
#define ICON_VOTED_BRD		"\033[1;33m投\033[m"	/* 舉行投票中的看板 */
#define ICON_NOTRAN_BRD		"○"			/* 不轉信板 */
#define ICON_TRAN_BRD		"●"			/* 轉信板 */

#define TOKEN_ZAP_BRD		'-'			/* zap 板 */
#define TOKEN_FRIEND_BRD	'.'			/* 好友板 */
#define TOKEN_SECRET_BRD	')'			/* 秘密板 */

#define GOODBYE_MSG		"G)隨風而逝 M)報告站長 N)留言板 Q)取消？[Q] "

#ifdef SITE_LEXEL
#include "lexel.h"
#endif

#endif				/* _THEME_H_ */
