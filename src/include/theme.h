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
/* °ò¥»ÃC¦â©w¸q¡A¥H§Q¤¶­±­×§ï				 */
/* ----------------------------------------------------- */

//#define COLOR1		"\033[34;46m"	/* footer/feeter ªº«e¬qÃC¦â */
#define COLOR1		"\033[;1;33;44m"	/* footer/feeter ªº«e¬qÃC¦â */
#define COLOR2		"\033[;30;47m"		/* footer/feeter ªº«á¬qÃC¦â */
#define COLOR3		"\033[;30;47m"	
#define COLOR4		"\033[1;44m"		/* ¥ú´Î ªºÃC¦â */
#define COLOR5		"\033[34;47m"		/* more ÀÉÀYªº¼ÐÃDÃC¦â */
#define COLOR6		"\033[37;44m"		/* more ÀÉÀYªº¤º®eÃC¦â */
#define COLOR7		"\033[1m"		/* §@ªÌ¦b½u¤WªºÃC¦â */
#define COLOR8		"\033[;1;34;47m"	/* feeter ¥[±j¼Ðµù(ÂÅ/¥Õ) */
#define COLOR9		"\033[;31;47m"		/* feeter ¥[±j¼Ðµù(¬õ/¥Õ) */
#define COLOR10		"\033[34;47m"		/* neck ªºÃC¦â */
#define COLOR11		"\033[35;47m"		/* menu feeter ªºÃC¦â */
#define COLOR_SITE	"\033[1;37;44m"		/* ryanlei.081017: ¯¸¥x¥D¦â */


/* ----------------------------------------------------- */
/* ¨Ï¥ÎªÌ¦W³æÃC¦â					 */
/* ----------------------------------------------------- */

#define COLOR_NORMAL	""		/* ¤@¯ë¨Ï¥ÎªÌ */
#define COLOR_MYBAD	"\033[1;31m"	/* Ãa¤H */
#define COLOR_MYGOOD	"\033[1;32m"	/* §Úªº¦n¤Í */
#define COLOR_OGOOD	"\033[1;33m"	/* »P§Ú¬°¤Í */
#define COLOR_CLOAK	"\033[1;35m"	/* Áô§Î */	/* itoc.µù¸Ñ: ¨S¥Î¨ì¡A­nªº¤H½Ð¦Û¦æ¥[¤J ulist_body() */
#define COLOR_SELF	"\033[1;36m"	/* ¦Û¤v */
#define COLOR_BOTHGOOD	"\033[1;37m"	/* ¤¬³]¦n¤Í */
#define COLOR_BRDMATE	"\033[36m"	/* ªO¦ñ */
#define COLOR_SUPER_BOTHGOOD	"\033[1;35m"	/* ¤¬³]¶W¯Å¦n¤Í */
#define COLOR_SUPER_MYGOOD	"\033[35m"	/* §Úªº¶W¯Å¦n¤Í */
#define COLOR_SUPER_OGOOD	"\033[33m"	/* »P§Ú¬°¶W¯Å¦n¤Í */


/* ----------------------------------------------------- */
/* ¦UÃþ¥ú´ÎÃC¦â						 */
/* ----------------------------------------------------- */

#define COLORBAR_MENU	"\033[1;37;44m"	/* menu.c ¿ï³æ¥ú´Î */
#define	COLORBAR_BRD	"\033[1;41m"	/* board.c, favor.c ¿ï³æ¥ú´Î */
#define COLORBAR_POST	"\033[1;43m"	/* post.c ¿ï³æ¥ú´Î */
#define COLORBAR_GEM	"\033[1;42m"	/* gem.c  ¿ï³æ¥ú´Î */
#define COLORBAR_PAL	"\033[1;45m"	/* pal.c  ¿ï³æ¥ú´Î */
#define COLORBAR_USR	"\033[1;45m"	/* ulist.c ¿ï³æ¥ú´Î */
#define COLORBAR_BMW	"\033[1;43m"	/* bmw.c ¿ï³æ¥ú´Î */
#define COLORBAR_MAIL	"\033[1;42m"	/* mail.c ¿ï³æ¥ú´Î */
#define COLORBAR_ALOHA	"\033[1;41m"	/* aloha.c ¿ï³æ¥ú´Î */
#define COLORBAR_VOTE	"\033[;30;43m"	/* vote.c ¿ï³æ¥ú´Î */
#define COLORBAR_NBRD	"\033[1;46m"	/* newbrd.c ¿ï³æ¥ú´Î */
#define COLORBAR_SONG	"\033[1;42m"	/* song.c ¿ï³æ¥ú´Î */
#define COLORBAR_RSS	"\033[1;46m"	/* rss.c ¿ï³æ¥ú´Î */

/* ----------------------------------------------------- */
/* ¿ï³æ¦ì¸m						 */
/* ----------------------------------------------------- */

/* itoc.µù¸Ñ: ª`·N MENU_XPOS ­n >= MENU_XNOTE + MOVIE_LINES */

#define MENU_XNOTE	2		/* °ÊºA¬ÝªO¥Ñ (2, 0) ¶}©l */
#define MOVIE_LINES	10		/* °Êµe³Ì¦h¦³ 10 ¦C */

#define MENU_XPOS	13		/* ¿ï³æ¶}©lªº (x, y) ®y¼Ð */
#define MENU_YPOS	((d_cols >> 1) + 18)


/* ----------------------------------------------------- */
/* °T®§¦r¦ê¡G*_neck() ®Éªº necker ³£§ì¥X¨Ó©w¸q¦b³o	 */
/* ----------------------------------------------------- */

/* necker ªº¦æ¼Æ³£¬O¤G¦æ¡A±q (1, 0) ¨ì (2, 80) */

/* ©Ò¦³ªº XZ_* ³£¦³ necker¡A¥u¬O¦³¨Ç¦b *_neck()¡A¦³¨ÇÂÃ¦b *_head() */

/* ulist_neck() ¤Î xpost_head() ªº²Ä¤@¦æ¤ñ¸û¯S§O¡A¤£¦b¦¹©w¸q */

#define NECKER_CLASS	"[¡ö]¥D¿ï³æ [¡÷]¾\\Åª [¡ô¡õ]¿ï¾Ü [c]½g¼Æ [y]¸ü¤J [/?]·j´M [s]¬ÝªO [h]»¡©ú\n" \
			COLOR10 "  %s   ¬Ý  ªO       Ãþ§OÂà«H¤¤   ¤å   ±Ô   ­z%*s              ¤H®ð ªO    ¥D%*s    \033[m"

#define NECKER_ULIST	"\n" \
			COLOR10 "  ½s¸¹  ¥N¸¹         ¼ÊºÙ%*s                 %-*s               °ÊºA        ¶¢¸m \033[m"

#define NECKER_PAL	"[¡ö]Â÷¶} [a]·s¼W [c]­×§ï [d]§R°£ [m]±H«H [w]¤ô²y [s]¾ã²z [¡÷]¬d¸ß [h]»¡©ú\n" \
			COLOR10 "  ½s¸¹    ¥N ¸¹         ¤Í       ½Ë%*s                                           \033[m"

#define NECKER_ALOHA	"[¡ö]Â÷¶} [a]·s¼W [d]§R°£ [D]°Ï¬q§R°£ [m]±H«H [w]¤ô²y [s]­«¾ã [f]¤Þ¤J [h]»¡©ú\n" \
			COLOR10 "  ½s¸¹   ¤W ¯¸ ³q ª¾ ¦W ³æ%*s                                                    \033[m"

#define NECKER_VOTE	"[¡ö]Â÷¶} [R]µ²ªG [^P]Á|¦æ [E]­×§ï [V]¹wÄý [^Q]§ï´Á [o]¦W³æ [h]»¡©ú\n" \
			COLOR10 "  ½s¸¹      ¶}²¼¤é   ¥D¿ì¤H       §ë  ²¼  ©v  ¦®%*s                              \033[m"

#define NECKER_BMW	"[¡ö]Â÷¶} [d]§R°£ [D]°Ï¬q§R°£ [m]±H«H [M]Àx¦s [w]¤ô²y [s]§ó·s [¡÷]¬d¸ß [h]»¡©ú\n" \
			COLOR10 "  ½s¸¹ ¥N  ¸¹       ¤º       ®e%*s                                          ®É¶¡ \033[m"

#define NECKER_MF	"[¡ö]Â÷¶} [¡÷]¶i¤J [^P]·s¼W [d]§R°£ [c]¤Á´« [C]½Æ»s [^V]¶K¤W [m]²¾°Ê [h]»¡©ú\n" \
			COLOR10 "  %s   ¬Ý  ªO       Ãþ§OÂà«H¤¤   ¤å   ±Ô   ­z%*s              ¤H®ð ªO    ¥D%*s    \033[m"

#define NECKER_COSIGN	"[¡ö]Â÷¶} [¡÷]¾\\Åª [^P]¥Ó½Ð [d]§R°£ [o]¶}ªO [h]»¡©ú\n" \
			COLOR10 "  ½s¸¹   ¤é ´Á  Á|¿ì¤H       ¬Ý  ªO  ¼Ð  ÃD%*s                                   \033[m"

#define NECKER_SONG	"[¡ö]Â÷¶} [¡÷]ÂsÄý [o]ÂIºq¨ì¬ÝªO [m]ÂIºq¨ì«H½c [Enter]ÂsÄý [h]»¡©ú\n" \
			COLOR10 "  ½s¸¹     ¥D              ÃD%*s                            [½s      ¿ï] [¤é  ´Á]\033[m"

#define NECKER_NEWS	"[¡ö]Â÷¶} [¡÷]¾\\Åª [h]»¡©ú\n" \
			COLOR10 "  ½s¸¹     ¤é ´Á §@  ªÌ       ·s  »D  ¼Ð  ÃD%*s                                  \033[m"

#define NECKER_XPOST	"\n" \
			COLOR10 "  ½s¸¹     ¤é ´Á §@  ªÌ       ¤å  ³¹  ¼Ð  ÃD%*s                                  \033[m"

#define NECKER_MBOX	"[¡ö]Â÷¶} [¡÷,r]Åª«H [d]§R°£ [R,y](¸s²Õ)¦^«H [s]±H«H [x]Âà¿ý [X]Âà¹F [h]»¡©ú\n" \
			COLOR10 "  ½s¸¹   ¤é ´Á §@  ªÌ       «H  ¥ó  ¼Ð  ÃD%*s                                    \033[m"

#define NECKER_POST	"[¡ö]Â÷¶} [¡÷]¾\\Åª [^P]µoªí [b]¶iªOµe­± [d]§R°£ [V]§ë²¼ [TAB]ºëµØ°Ï [h]»¡©ú\n" \
			COLOR10 "  ½s¸¹     ¤é ´Á §@  ªÌ       ¤å  ³¹  ¼Ð  ÃD%*s                        ¤H®ð:%-4d \033[m"

#define NECKER_GEM	"[¡ö]Â÷¶} [¡÷]ÂsÄý [B]¼Ò¦¡ [C]¼È¦s [F]Âà±H [d]§R°£ [h]»¡©ú  %s\n" \
			COLOR10 "  ½s¸¹     ¥D              ÃD%*s                            [½s      ¿ï] [¤é  ´Á]\033[m"

/* ¥H¤U³o¨Ç«h¬O¤@¨ÇÃþ XZ_* µ²ºcªº necker */

#define NECKER_VOTEALL	"[¡ô/¡õ]¤W¤U [PgUp/PgDn]¤W¤U­¶ [Home/End]­º§À [¡÷]§ë²¼ [¡ö][q]Â÷¶}\n" \
			COLOR10 "  ½s¸¹   ¬Ý  ªO       Ãþ§OÂà«H¤¤   ¤å   ±Ô   ­z%*s                  ªO    ¥D%*s     \033[m"

#define NECKER_CREDIT	"[¡ö]Â÷¶} [C]´«­¶ [1]·s¼W [2]§R°£ [3]¥þ§R [4]Á`­p\n" \
			COLOR10 "  ½s¸¹   ¤é  ´Á   ¦¬¤ä  ª÷  ÃB  ¤ÀÃþ     »¡  ©ú%*s                               \033[m"

#define NECKER_HELP	"[¡ö]Â÷¶} [¡÷]¾\\Åª [^P]·s¼W [d]§R°£ [T]¼ÐÃD [E]½s¿è [m]²¾°Ê\n" \
			COLOR10 "  ½s¸¹    ÀÉ ®×         ¼Ð       ÃD%*s                                           \033[m"

#define NECKER_INNBBS	"[¡ö]Â÷¶} [^P]·s¼W [d]§R°£ [E]½s¿è [/]·j´M [Enter]¸Ô²Ó\n" \
			COLOR10 "  ½s¸¹            ¤º         ®e%*s                                               \033[m"

#define NECKER_RSS	"[¡ö]Â÷¶} [¡÷]ÂsÄý [o]ÁôÂÃ [u]UTF8½s½X [n]Html/Txt [s]±Ò°Ê/¼È°± [R]­«°e        \n" \
			COLOR10 "  ½s¸¹  ¤é ´Á%*s ¼Ð  Ã±       ÄÝ ©Ê    ºô§}                                      \033[m"

/* ----------------------------------------------------- */
/* °T®§¦r¦ê¡Gmore() ®Éªº footer ³£§ì¥X¨Ó©w¸q¦b³o	 */
/* ----------------------------------------------------- */

/* itoc.010914.µù¸Ñ: ³æ¤@½g¡A©Ò¥H¥s FOOTER¡A³£¬O 78 char */

/* itoc.010821: ª`·N \\ ¬O \¡A³Ì«á§Oº|¤F¤@­ÓªÅ¥ÕÁä :p */

#define FOOTER_POST	\
COLOR1 " ¤å³¹¿ïÅª " COLOR9 " (ry)"COLOR2"¦^À³ "COLOR9"(=\\[]<>-+;'`)"COLOR2"¥DÃD "COLOR9"(|?QA)"COLOR2"·j´M¼ÐÃD§@ªÌ "COLOR9"(kj)"COLOR2"¤W¤U½g "COLOR9"(C)"COLOR2"¼È¦s   "

#define FOOTER_MAILER	\
COLOR1 " ³½¶­©¹ªð " COLOR9 " (ry)"COLOR2"¦^«H/¸s²Õ "COLOR9"(X)"COLOR2"Âà¹F "COLOR9"(d)"COLOR2"§R°£ "COLOR9"(m)"COLOR2"¼Ð°O "COLOR9"(C)"COLOR2"¼È¦s "COLOR9"(=\\[]<>-+;'`|?QAkj)"COLOR2"  "

#define FOOTER_GEM	\
COLOR1 " ºëµØ¿ïÅª " COLOR9 " (=\\[]<>-+;'`)"COLOR2"¥DÃD "COLOR9"(|?QA)"COLOR2"·j´M¼ÐÃD§@ªÌ "COLOR9"(kj)"COLOR2"¤W¤U½g "COLOR9"(¡ô¡õ¡ö)"COLOR2"¤W¤UÂ÷¶}   "

#ifdef HAVE_GAME
#define FOOTER_TALK	\
COLOR1 " ¥æ½Í¼Ò¦¡ " COLOR9 " (^O)"COLOR2"¹ï«³¼Ò¦¡ "COLOR9"(^C,^D)"COLOR2"µ²§ô¥æ½Í "COLOR9"(^T)"COLOR2"¤Á´«©I¥s¾¹ "COLOR9"(^Z)"COLOR2"§Ö±¶¦Cªí "COLOR9"(^G)"COLOR2"¹Í¹Í  "
#else
#define FOOTER_TALK	\
COLOR1 " ¥æ½Í¼Ò¦¡ " COLOR9 " (^C,^D)"COLOR2"µ²§ô¥æ½Í "COLOR9"(^T)"COLOR2"¤Á´«©I¥s¾¹ "COLOR9"(^Z)"COLOR2"§Ö±¶¦Cªí "COLOR9"(^G)"COLOR2"¹Í¹Í "COLOR9"(^Y)"COLOR2"²M°£      "
#endif

#define FOOTER_COSIGN	\
COLOR1 " ³s¸p¾÷¨î " COLOR9 " (ry)"COLOR2"¥[¤J³s¸p "COLOR9"(kj)"COLOR2"¤W¤U½g "COLOR9"(¡ô¡õ¡ö)"COLOR2"¤W¤UÂ÷¶} "COLOR9"(h)"COLOR2"»¡©ú                   " 

//#define FOOTER_MORE	\
//COLOR1 " ÂsÄý P.%d (%d%%) " COLOR2 " (h)»¡©ú [PgUp][PgDn][0][$]²¾°Ê (/n)·j´M (C)¼È¦s (¡öq)µ²§ô "

#define FOOTER_MORE	\
COLOR1 " ÂsÄý P.%d (%d%%) " COLOR9 " (h)"COLOR2"»¡©ú " COLOR9 "(@)"COLOR2"°Êµe¼½©ñ "COLOR9"(¡ô¡õ0$)"COLOR2"²¾°Ê "COLOR9"(/n)"COLOR2"·j´M "COLOR9"(C)"COLOR2"¼È¦s "COLOR9"(¡öq)"COLOR2"µ²§ô  "

#define FOOTER_VEDIT	\
COLOR1 " %s " COLOR9 " (^Z)"COLOR2"»¡©ú "COLOR9"(^W)"COLOR2"²Å¸¹ "COLOR9"(^L)"COLOR2"­«Ã¸ "COLOR9"(^X)"COLOR2"ÀÉ®×³B²z ùø"COLOR9"%s"COLOR2"¢x"COLOR9"%s"COLOR2"ùø"COLOR9"%5d:%3d"COLOR2"    \033[m"

#define FOOTER_VEDIT_RONLY	\
"%.0s\033[1;5m             °ßÅª¼Ò¦¡                 \033[m"COLOR9"(^X)"COLOR2"ÀÉ®×³B²z ùø"COLOR9"%s"COLOR2"¢x"COLOR9"%s"COLOR2"ùø"COLOR9"%5d:%3d"COLOR2"    \033[m"

/* ----------------------------------------------------- */
/* °T®§¦r¦ê¡Gxo_foot() ®Éªº feeter ³£§ì¥X¨Ó©w¸q¦b³o      */
/* ----------------------------------------------------- */


/* itoc.010914.µù¸Ñ: ¦Cªí¦h½g¡A©Ò¥H¥s FEETER¡A³£¬O 78 char */

#define FEETER_CLASS	\
COLOR1 " ¬ÝªO¿ï¾Ü " COLOR9 " (c)"COLOR2"·s¤å³¹ "COLOR9"(vV)"COLOR2"¼Ð°O¤wÅª¥¼Åª "COLOR9"(y)"COLOR2"¥þ³¡¦C¥X "COLOR9"(z)"COLOR2"¿ï­q "COLOR9"(A)"COLOR2"¥þ°ì·j´M "COLOR9"(S)"COLOR2"±Æ§Ç "

#define FEETER_ULIST	\
COLOR1 " ºô¤Í¦Cªí " COLOR9 " (f)"COLOR2"¦n¤Í "COLOR9"(w/t/m)"COLOR2"¤ô²y/²á¤Ñ/±H«H "COLOR9"(q)"COLOR2"¬d¸ß "COLOR9"(ad)"COLOR2"¥æ¤Í "COLOR9"(s)"COLOR2"§ó·s "COLOR9"(TAB)"COLOR2"¤Á´«   "

#define FEETER_PAL	\
COLOR1 " ©IªB¤Þ¦ñ " COLOR9 " (a/d/m)"COLOR2"·s¼W/§R°£/±H«H "COLOR9"(c)"COLOR2"¤Í½Ë "COLOR9"(f)"COLOR2"¤Þ¤J¦n¤Í "COLOR9"(r^Q)"COLOR2"¬d¸ß "COLOR9"(s)"COLOR2"§ó·s        "

#define FEETER_ALOHA	\
COLOR1 " ¤W¯¸³qª¾ " COLOR9 " (a)"COLOR2"·s¼W "COLOR9"(d)"COLOR2"§R°£ "COLOR9"(D)"COLOR2"°Ï¬q§R°£ "COLOR9"(f)"COLOR2"¤Þ¤J¦n¤Í "COLOR9"(r^Q)"COLOR2"¬d¸ß "COLOR9"(s)"COLOR2"§ó·s          "

#define FEETER_VOTE	\
COLOR1 " ¬ÝªO§ë²¼ " COLOR9 " (¡÷/r/v)"COLOR2"§ë²¼ "COLOR9"(R)"COLOR2"µ²ªG "COLOR9"(^P)"COLOR2"·s¼W§ë²¼ "COLOR9"(E)"COLOR2"­×§ï "COLOR9"(V/b/o)"COLOR2"¹wÄý/¶}²¼/¦W³æ    "

#define FEETER_BMW	\
COLOR1 " ¤ô²y¦^ÅU " COLOR9 " (d/D)"COLOR2"§R°£/°Ï¬q§R°£ "COLOR9"(m)"COLOR2"±H«H "COLOR9"(w)"COLOR2"¤ô²y "COLOR9"(^R)"COLOR2"¦^°T "COLOR9"(^Q)"COLOR2"¬d¸ß "COLOR9"(s)"COLOR2"§ó·s       "

#define FEETER_MF	\
COLOR1 " ³Ì·R¬ÝªO " COLOR9 " (^P)"COLOR2"·s¼W "COLOR9"(Cg)"COLOR2"½Æ»s "COLOR9"(p^V)"COLOR2"¶K¤W "COLOR9"(d)"COLOR2"§R°£ "COLOR9"(c)"COLOR2"·s¤å³¹ "COLOR9"(vV)"COLOR2"¼Ð°O¤wÅª/¥¼Åª    "

#define FEETER_COSIGN	\
COLOR1 " ³s¸p¤p¯¸ " COLOR9 " (r/y)"COLOR2"Åª¨ú/¦^À³ "COLOR9"(^P)"COLOR2"µoªí "COLOR9"(d)"COLOR2"§R°£ "COLOR9"(o)"COLOR2"¶}ªO "COLOR9"(c)"COLOR2"Ãö³¬ "COLOR9"(E/B)"COLOR2"½s¿è/³]©w     "

#define FEETER_SONG	\
COLOR1 " ÂIºq¨t²Î " COLOR9 " (r)"COLOR2"Åª¨ú "COLOR9"(o)"COLOR2"ÂIºq¨ì¬ÝªO "COLOR9"(m)"COLOR2"ÂIºq¨ì«H½c "COLOR9"(E)"COLOR2"½s¿èÀÉ®× "COLOR9"(T)"COLOR2"½s¿è¼ÐÃD        "

#define FEETER_NEWS	\
COLOR1 " ·s»DÂI¿ï " COLOR9 " (¡ô/¡õ)"COLOR2"¤W¤U "COLOR9"(PgUp/PgDn)"COLOR2"¤W¤U­¶ "COLOR9"(Home/End)"COLOR2"­º§À "COLOR9"(¡÷r)"COLOR2"¿ï¨ú "COLOR9"(¡ö)(q)"COLOR2"Â÷¶} "

#define FEETER_XPOST	\
COLOR1 " ¦ê¦C·j´M " COLOR9 " (y/x)"COLOR2"¦^À³/Âà¿ý  "COLOR9"(m/t)"COLOR2"¼Ð°O/¼ÐÅÒ "COLOR9"(d)"COLOR2"§R°£ "COLOR9"(^P)"COLOR2"µoªí "COLOR9"(^Q)"COLOR2"¬d¸ß§@ªÌ       "

#define FEETER_MBOX	\
COLOR1 " «H«H¬Û±¤ " COLOR9 " (y)"COLOR2"¦^«H "COLOR9"(F/X/x)"COLOR2"Âà±H/Âà¹F/Âà¿ý "COLOR9"(d)"COLOR2"§R°£ "COLOR9"(D)"COLOR2"°Ï¬q§R°£ "COLOR9"(m)"COLOR2"¼Ð°O "COLOR9"(E)"COLOR2"½s¿è  "

#define FEETER_POST	\
COLOR1 " ¤å³¹¦Cªí \033[m" COLOR9 " (f)"COLOR8"½s¸¹/ÀÉ¦W¤Á´«\033[m" COLOR9 " (ry)"COLOR2"¦^«H " COLOR9 "(/)"COLOR8"·j´M¥\\¯à¾ã¦XÁä\033[m" COLOR9 " (x/V/u)"COLOR2"Âà¿ý/§ë²¼/·s»D  "

#define FEETER_GEM	\
COLOR1 " ¬ÝªOºëµØ " COLOR9 " (^P/a/f)"COLOR2"·s¼W/¤å³¹/¥Ø¿ý "COLOR9"(E)"COLOR2"½s¿è "COLOR9"(T)"COLOR2"¼ÐÃD "COLOR9"(m)"COLOR2"²¾°Ê "COLOR9"(c)"COLOR2"½Æ»s "COLOR9"(p^V)"COLOR2"¶K¤W   "

#define FEETER_VOTEALL	\
COLOR1 " §ë²¼¤¤¤ß " COLOR9 " (¡ô/¡õ)"COLOR2"¤W¤U "COLOR9"(PgUp/PgDn)"COLOR2"¤W¤U­¶ "COLOR9"(Home/End)"COLOR2"­º§À "COLOR9"(¡÷)"COLOR2"§ë²¼ "COLOR9"(¡ö)(q)"COLOR2"Â÷¶}  "

#define FEETER_HELP	\
COLOR1 " »¡©ú¤å¥ó " COLOR9 " (¡ô/¡õ)"COLOR2"¤W¤U "COLOR9"(PgUp/PgDn)"COLOR2"¤W¤U­¶ "COLOR9"(Home/End)"COLOR2"­º§À "COLOR9"(¡÷r)"COLOR2"ÂsÄý "COLOR9"(¡ö)(q)"COLOR2"Â÷¶} "

#define FEETER_INNBBS	\
COLOR1 " Âà«H³]©w " COLOR9 " (¡ô/¡õ)"COLOR2"¤W¤U "COLOR9"(PgUp/PgDn)"COLOR2"¤W¤U­¶ "COLOR9"(Home/End)"COLOR2"­º§À "COLOR9"(¡ö)(q)"COLOR2"Â÷¶}           "

#define FEETER_BITLBEE \
COLOR1 "  §Y®É³q  "COLOR9" (w)"COLOR2"¶Ç°e°T®§ " COLOR9 "(^r)"COLOR2"¦^ÂÐ°T®§ " COLOR9 "(a/d)"COLOR2"¼W/§RÁpµ¸¤H " COLOR9 "(b/B)"COLOR2"«ÊÂê/¸Ñ°£ "COLOR9"(^k)"COLOR2"Â_½u  " 

#define FEETER_FAKE_PAL	\
COLOR1 " ÂsÄý¦W³æ " COLOR9 " ¥»¦W³æ¶È¨ÑÂsÄý¡AªO¥D­×§ïªO¤Í¦W³æ½Ð°h¥X«á«ö o ¿ï¾Ü­×§ï              "

#define FEETER_RSS	\
COLOR1 " RSS ³]©w " COLOR9 " (^P/a)"COLOR2"·s¼W "COLOR9"(d)"COLOR2"§R°£ "COLOR9"(E)"COLOR2"½s¿è "COLOR9"(T)"COLOR2"¼ÐÃ±¦WºÙ "COLOR9"(H)"COLOR2"RSSºô§} "COLOR9"(m)"COLOR2"²¾°Ê "COLOR9"(h)"COLOR2"»¡©ú  "

/* ----------------------------------------------------- */
/* ¯¸¥x¨Ó·½Ã±¦W						 */
/* ----------------------------------------------------- */

/* itoc: «ØÄ³ banner ¤£­n¶W¹L¤T¦æ¡A¹Lªøªº¯¸Ã±¥i¯à·|³y¦¨¬Y¨Ç¨Ï¥ÎªÌªº¤Ï·P */

/*dexter  §âorigin,From§ï¦¨¨â¦æ*/
#define EDIT_BANNER	"--\n" \
"\e[;32m¡° Origin: ·¬¾ôÅæ¯¸<bbs.cs.nthu.edu.tw>\n\e[;32m¡» From: %-*.*s\e[m\n"

#define MODIFY_BANNER	"\e[;36m%s ©ó %s ±q %s ­×§ï\e[m\n"

#define EDIT_BANNER_1	"--\n" \
"\e[m\e[1m¡@\e[;32;40m¢z¢w¢x¢z¢z¢w¢z¢w¢¡¢w¢s¢z¢w¢¡¢x¡@¢z¢w \e[1;37mtelnet://imaple.tw \e[31m·[;30;41m¬[m\n" \
"\e[1m¡@\e[32m¢u¢w¢x¢x¢u¢w¢u¢s¢£  ¢x¢u¢w¢x¢x  ¢u¢w\e[37m¡@ \e[32mIAS_EverTale   \e[37m¡@\e[31m¾[;30;41mô[m\n" \
"\e[1m¢z\e[42m¢|¢w\e[;30;42m¢[1;37m|¢£¢|¢w¢x¢¢    ¢x¢x¡@¢£¢|¢w¢|¢w¬[;30;42mG\e[1;37m¡@¨[;30;42mÆ[1;37m¡@±[;30;42mµ[1;37m¡@À[;30;42ms\e[1;37m¡@ª[;30;42mO\e[1;37;40m¢{\e[;30;41mÅ[1;31;40mæ[;30;40mNAYURI\e[m\n" \
" \e[1m¡Ã¡Ã¡Ã¡Ã¡Ã¡Ã¡Ã¡Ã¡Ã¡Ã¡Ã¡Ã¡Ã¡Ã¡Ã¡Ã¡Ã¡Ã¡Ã¡Ã¡Ã¡Ã¡Ã¡Ã¡Ã¡Ã¡Ã¡Ã \e[;30;41m¯[1;31;40m¸[m\n" \
"\e[;32m¡» From: %-*.*s\e[m\n"


#define EDIT_BANNER_2	"--\n" \
"  ¢e       ¢¨ ¢e¢e¢e ¢e¢e¢e ¢e     ¢e¢e¢e \e[1;37m ²M¤j¸ê¤u\e[m\n" \
"\e[1;37;44m  ¢i ¢i¢©¢¨¢i ¢i¢e¢i ¢i¢e¢i ¢i     ¢i¢e¢e  \e[33m%-*.*s\e[m\n" \
"  ¢i ¢i¢ª¢«¢i ¢i  ¢i ¢i     ¢i¢e¢e ¢i¢e¢e \e[1;37m¡i·¬¾ôÅæ¯¸¡j telnet://imaple.tw\e[m\n"

#define EDIT_BANNER_3	"--\n" \
"\e[;30m\242\e[44m\143_\241\e[1;30m\304¡Å\e[33m¡]_\e[;34m¢h¢g\e[1;31;44m' * \e[;34;44m\242\e[31m\251_ \e[34;41m\242\e[31;44m\251\242\e[41m\250\e[34;40m¢g¢h\e[30;44m \e[37m¢b_\e[30m ¢e¢g¢h\e[m\241\e[33m\103\e[31m.\e[1;34m·¬¾ôÅæ¯¸\241\e[37m\104\e[30mtelnet://imaple.tw\e[;30m\241\e[41m\273\e[31;40m\242\e[41m\251\e[40m}\e[m\n" \
" \e[34m=\e[37m\242\e[44m\142  ¢v¡Ç\e[30m\241\e[34m\303   \e[31m*\e[33m.   \e[34;41m¢g\e[1;30;44m\244\e[;30;41m\164@\e[31;44m¢n \e[30m¢d\e[m¢v   \e[1;35m¢c~\e[5m+\e[;33m                                  \e[30m\242\e[31m\253\e[1;31m\244\e[;30m\241\e[m\n" \
"   ¡Ã\e[30;44m¢d¢c¢b¢c\241\e[31m\103\e[30m¢b¢c¡Å\e[31m_\242\e[41m\250\e[30m¢Ä\e[44m_¢h\e[33;40m.\e[31m*   \e[1;35m¢u=rom¡G\e[37m%-*.*s\e[m\n" \
"   \e[31m¡Ç¡Ã¡Ç¡Ã¡Ã¡Ç¡Ç¡Ã¡Ã¡Ç¡Ã¡Ç¡Ã¡Ã¡Ç\e[35m@\244\e[30m\110\e[31m¡Ã¡Ã¡Ç¡Ã¡Ç¡Ã¡Ã¡Ç¡Ç¡Ã¡Ã¡Ç¡Ç¡Ã¡Ã¡Ç¡Ç¡Ã¡Ç¡Ã\e[m\n"

#define EDIT_BANNER_MEICHU_WIN	"--\n" \
"\e[42m   \e[0;30;42m¢¨\e[0;42m¢j\e[0;30;42m¢¨\e[0;42m¢j \e[0;31;43m  §Ú­n  \e[0;42m                                                  \e[0;30;42m(((°k  \e[m\n" \
"\e[42m \e[0;32;47m¢p\e[1;37;40m¢h¢i¢h\e[42m¢© \e[0;31;43m ¦Y¦Ë¤l \e[42m  \e[0;33m   ¤v¤¡±ö¦ËÁÉ±N©ó 3/6 ~ 3/8 ®i¶}    \e[0;31;42m          ¢©  ¢©   \e[m\n" \
"\e[42m \e[0;32;47m¢o\e[1;37;40m¢i¢i¢i\e[1;37m -\e[1;37;42m¢j   \e[0;33;42m¢k                                                  \e[0;30;41m @) @)\e[0;31;40m\e[42m   \e[m\n" \
"\e[42m \e[1;37;42m ¢i¢i¢i¢i¢i\e[42m    \e[0;33;42m¢k    \e[0;33m ·¬¾ôÅæ¯¸ÁÜ½Ð±z  ¤@¦P¬°²MµØ¤j¾Ç¥[ªo \e[0;31;42m    @     \e[0;31;42m¢ª\e[0;30;41m¤H\e[0;31;42m¢«   \e[m\n" \
"\e[0;32;47m¢n\e[1;37;42m¢i¢i¢i¢i¢«  \e[1;37;42m,,\e[0;33;42m¢k                                             \e[0;31;42m\\¢¨¢i¢i\e[42m\e[1;30m¢i\e[0;31;42m¢k   \e[m\n" \
"\e[0;32;47m¢m\e[1;37;40m¢i¢«    \e[0;32m¢ª\e[0;30;42m¢¨¢i\e[m\e[42m     \e[1;34m From:%-*.*s      \e[0;31;42m/\\     /\\     \e[m\n"

/* ----------------------------------------------------- */
/* ¨ä¥L°T®§¦r¦ê						 */
/* ----------------------------------------------------- */

#define VMSG_NULL	"                           " COLOR1 " ¡´ ½Ð«ö¥ô·NÁäÄ~Äò ¡´ \033[m"

#define ICON_UNREAD_BRD		"\033[1;33m¡E\033[m"	/* ¥¼Åª¬ÝªO */
#define ICON_READ_BRD		"  "			/* ¤wÅª¬ÝªO */

#define ICON_GAMBLED_BRD	"\033[1;31m½ä\033[m"	/* Á|¦æ½ä½L¤¤ªº¬ÝªO */
#define ICON_VOTED_BRD		"\033[1;33m§ë\033[m"	/* Á|¦æ§ë²¼¤¤ªº¬ÝªO */
#define ICON_NOTRAN_BRD		"¡³"			/* ¤£Âà«HªO */
#define ICON_TRAN_BRD		"¡´"			/* Âà«HªO */

#define TOKEN_ZAP_BRD		'-'			/* zap ªO */
#define TOKEN_FRIEND_BRD	'.'			/* ¦n¤ÍªO */
#define TOKEN_SECRET_BRD	')'			/* ¯µ±KªO */

#define GOODBYE_MSG		"G)ÀH­·¦Ó³u M)³ø§i¯¸ªø N)¯d¨¥ªO Q)¨ú®ø¡H[Q] "

#ifdef SITE_LEXEL
#include "lexel.h"
#endif

#endif				/* _THEME_H_ */
