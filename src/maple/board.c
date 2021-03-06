/*-------------------------------------------------------*/
/* board.c	( NTHU CS MapleBBS Ver 2.36 )		 */
/*-------------------------------------------------------*/
/* target : 看板、群組功能	 			 */
/* create : 95/03/29				 	 */
/* update : 95/12/15				 	 */
/*-------------------------------------------------------*/


#include "bbs.h"


extern BCACHE *bshm;
extern XZ xz[];
extern char xo_pool[];


char brd_bits[MAXBOARD];

#ifndef ENHANCED_VISIT
time4_t brd_visit[MAXBOARD];		/* 最近瀏覽時間 */
#endif


static char *class_img = NULL;
static XO board_xo;

inline int in_favor(char *brdname);	/* smiler.070724 */


#if 1
//********/* smiler.070602: for xsort 起始處 */********/

#define min(a, b)	(a) < (b) ? a : b
#undef	TEST


/* Qsort routine from Bentley & McIlroy's "Engineering a Sort Function". */


#define swapcode(TYPE, parmi, parmj, n) { 		\
	long i = (n) / sizeof (TYPE); 			\
	register TYPE *pi = (TYPE *) (parmi); 		\
	register TYPE *pj = (TYPE *) (parmj); 		\
	do { 						\
		register TYPE	t = *pi;		\
		*pi++ = *pj;				\
		*pj++ = t;				\
	} while (--i > 0);				\
}


#define SWAPINIT(a, es) \
	swaptype = (((char *)a - (char *)0) % sizeof(long) || \
	es % sizeof(long)) ? 2 : (es == sizeof(long)? 0 : 1);


static inline void
swapfunc(a, b, n, swaptype)
  char *a, *b;
  int n, swaptype;
{
  if (swaptype <= 1)
    swapcode(long, a, b, n)
  else
    swapcode(char, a, b, n)
}


#define swap(a, b)					\
	if (swaptype == 0) {				\
		long t = *(long *)(a);			\
		*(long *)(a) = *(long *)(b);		\
		*(long *)(b) = t;			\
	} else						\
		swapfunc(a, b, es, swaptype)


#define vecswap(a, b, n) 	if ((n) > 0) swapfunc(a, b, n, swaptype)


static inline char *
med3(a, b, c, cmp)
  char *a, *b, *c;
  int (*cmp) ();
{
  return cmp(a, b) < 0 ?
    (cmp(b, c) < 0 ? b : (cmp(a, c) < 0 ? c : a))
    : (cmp(b, c) > 0 ? b : (cmp(a, c) < 0 ? a : c));
}


void
xsort(a, n, es, cmp)
  void *a;
  size_t n, es;
  int (*cmp) ();
{
  char *pa, *pb, *pc, *pd, *pl, *pm, *pn;
  int d, r, swaptype, swap_cnt;

  SWAPINIT(a, es);

loop:

  swap_cnt = 0;
  if (n < 7)
  {
    for (pm = a + es; pm < (char *) a + n * es; pm += es)
      for (pl = pm; pl > (char *) a && cmp(pl - es, pl) > 0;
	pl -= es)
	swap(pl, pl - es);
    return;
  }

  pm = a + (n / 2) * es;

  if (n > 7)
  {
    pl = a;
    pn = a + (n - 1) * es;
    if (n > 40)
    {
      d = (n >> 3) * es;
      pl = med3(pl, pl + d, pl + d + d, cmp);
      pm = med3(pm - d, pm, pm + d, cmp);
      pn = med3(pn - 2 * d, pn - d, pn, cmp);
    }
    pm = med3(pl, pm, pn, cmp);
  }
  swap(a, pm);
  pa = pb = a + es;

  pc = pd = a + (n - 1) * es;
  for (;;)
  {
    while (pb <= pc && (r = cmp(pb, a)) <= 0)
    {
      if (r == 0)
      {
	swap_cnt = 1;
	swap(pa, pb);
	pa += es;
      }
      pb += es;
    }
    while (pb <= pc && (r = cmp(pc, a)) >= 0)
    {
      if (r == 0)
      {
	swap_cnt = 1;
	swap(pc, pd);
	pd -= es;
      }
      pc -= es;
    }
    if (pb > pc)
      break;
    swap(pb, pc);
    swap_cnt = 1;
    pb += es;
    pc -= es;
  }

  if (swap_cnt == 0)
  {				/* Switch to insertion sort */
    for (pm = a + es; pm < (char *) a + n * es; pm += es)
      for (pl = pm; pl > (char *) a && cmp(pl - es, pl) > 0; pl -= es)
	swap(pl, pl - es);
    return;
  }

  pn = a + n * es;
  r = min(pa - (char *) a, pb - pa);
  vecswap(a, pb - r, r);

  r = min(pd - pc, pn - pd - es);
  vecswap(pb, pn - r, r);

  if ((r = pb - pa) > es)
    xsort(a, r / es, es, cmp);

  if ((r = pd - pc) > es)
  {
    /* Iterate rather than recurse to save stack space */
    a = pn - r;
    n = r / es;
    goto loop;
  }
  /* xsort(pn - r, r / es, es, cmp); */
}

//*******/* smiler.070602: for xsort 結束處 */********/
#endif


/* ----------------------------------------------------- */
/* 看板閱讀記錄 .BRH (Board Reading History)		 */
/* ----------------------------------------------------- */


typedef struct BoardReadingHistory
{
  time4_t bstamp;		/* 建立看板的時間, unique */	/* Thor.brh_tail */
  time4_t bvisit;		/* 上次閱讀時間 */		/* Thor.980904: 沒在讀時放上次讀的時間, 正在讀時放 bhno */
  int bcount;			/* Thor.980902: 沒用到 */

  /* --------------------------------------------------- */
  /* time_t {final, begin} / {final | BRH_SIGN}		 */
  /* --------------------------------------------------- */
                           /* Thor.980904.註解: BRH_SIGN代表final begin 相同 */
                           /* Thor.980904.註解: 由大到小排列,存放已讀interval */
}                   BRH;


#define	BRH_EXPIRE	180		/* Thor.980902.註解: 保留多少天 */
#define BRH_MAX		200		/* Thor.980902.註解: 每板最多有幾個標籤 */
#define BRH_PAGE	2048		/* Thor.980902.註解: 每次多配量, 用不到了 */
#define	BRH_MASK	0x7fffffff	/* Thor.980902.註解: 最大量為2038年1月中*/
#define	BRH_SIGN	0x80000000	/* Thor.980902.註解: zap及壓final專用 */
#define	BRH_WINDOW	(sizeof(BRH) + sizeof(time4_t) * BRH_MAX * 2)


static int *brh_base;		/* allocated memory */
static int *brh_tail;		/* allocated memory */
static int brh_size;		/* allocated memory size */
static time4_t brh_expire;


static int *
brh_alloc(tail, size)
  int *tail;
  int size;
{
  int *base, n;

  base = brh_base;
  n = (char *) tail - (char *) base;
  size += n;
  if (size > brh_size)
  {
    /* size = (size & -BRH_PAGE) + BRH_PAGE; */
    size += n >> 4;		/* 多預約一些記憶體 */
    base = (int *) realloc((char *) base, size);

    if (base == NULL)
      abort_bbs();

    brh_base = base;
    brh_size = size;
    tail = (int *) ((char *) base + n);
  }

  return tail;
}


static void
brh_put()
{
  int *list;

  /* compact the history list */

  list = brh_tail;

  if (*list)
  {
    int *head, *tail, n, item, chrono;

    n = *++list;   /* Thor.980904: 正讀時是bhno */
    brd_bits[n] |= BRD_H_BIT;
    time4((time4_t *) list);    /* Thor.980904.註解: bvisit time */

    item = *++list;
    head = ++list;
    tail = head + item;

    while (head < tail)
    {
      chrono = *head++;
      n = *head++;
      if (n == chrono) /* Thor.980904.註解: 相同的時候壓起來 */
      {
	n |= BRH_SIGN;
	item--;
      }
      else
      {
	*list++ = chrono;
      }
      *list++ = n;
    }

    list[-item - 1] = item;
    *list = 0;
    brh_tail = list;  /* Thor.980904: 新的空brh */
  }
}


void
brh_get(bstamp, bhno)
  time4_t bstamp;		/* board stamp */
  int bhno;
{
  int *head, *tail;
  int size, bcnt, item;
  char buf[BRH_WINDOW];

  if (bstamp == *brh_tail) /* Thor.980904.註解: 該板已在 brh_tail上 */
    return;

  brh_put();

  bcnt = 0;
  tail = brh_tail;

  if (brd_bits[bhno] & BRD_H_BIT)
  {
    head = brh_base;
    while (head < tail)
    {
      item = head[2];
      size = item * sizeof(time4_t) + sizeof(BRH);

      if (bstamp == *head)
      {
	bcnt = item;
	memcpy(buf, head + 3, size);
	tail = (int *) ((char *) tail - size);
	if (item = (char *) tail - (char *) head)
	  memcpy(head, (char *) head + size, item);
	break;
      }
      head = (int *) ((char *) head + size);
    }
  }

  brh_tail = tail = brh_alloc(tail, BRH_WINDOW);

  *tail++ = bstamp;
  *tail++ = bhno;

  if (bcnt)			/* expand history list */
  {
    int *list;

    size = bcnt;
    list = tail;
    head = (int *) buf;

    do
    {
      item = *head++;
      if (item & BRH_SIGN)
      {
	item ^= BRH_SIGN;
	*++list = item;
	bcnt++;
      }
      *++list = item;
    } while (--size);
  }

  *tail = bcnt;
}


int
brh_unread(chrono)
  time4_t chrono;
{
  int *head, *tail, item;

  /* itoc.010407.註解: BRH_EXPIRE (180) 天前的文章都設為已讀 */
  if (chrono <= brh_expire)
    return 0;

  head = brh_tail + 2;
  if ((item = *head) > 0)
  {
    /* check {final, begin} history list */

    head++;
    tail = head + item;
    do
    {
      if (chrono > *head)
	return 1;

      head++;
      if (chrono >= *head)
	return 0;

    } while (++head < tail);
  }
  return 1;
}


void
brh_visit(mode)
  int mode;			/* 0 : visit, 1: un-visit */
{				/* itoc.010207: 或是傳入chrono, 代表讀至哪 */
  int *list;

  list = (int *) brh_tail + 2;
  *list++ = 2;
  if (mode)
  {
    *list = mode;
  }
  else
  {
    time4((time4_t *)list);
  }
  /* *++list = mode; */
  *++list = 0;	/* itoc.010207: 強定為 0, for 部分 visit */
}


int
brh_add(prev, chrono, next)
  time4_t prev, chrono, next;
{
  int *base, *head, *tail, item, final, begin;

  head = base = brh_tail + 2;
  item = *head++;
  tail = head + item;

  begin = BRH_MASK;

  while (head < tail)
  {
    final = *head;
    if (chrono > final)
    {
      if (prev <= final)
      {
	if (next < begin)	/* increase */
	  *head = chrono;
	else
	{			/* merge */
	  *base = item - 2;
	  base = head - 1;
	  do
	  {
	    *base++ = *++head;
	  } while (head < tail);
	}
	return;
      }

      if (next >= begin)
      {
	head[-1] = chrono;
	return;
      }

      break;
    }

    begin = *++head;
    head++;
  }

  /* insert or append */

  /* [21, 22, 23] ==> [32, 30] [15, 10] */

  if (item < BRH_MAX)
  {
    /* [32, 30] [22, 22] [15, 10] */

    *base = item + 2;
    tail += 2;
  }
  else
  {
    /* [32, 30] [22, 10] */  /* Thor.980923: how about [6, 7, 8] ? [15, 7] ? */

    tail--;
  }

  prev = chrono;
  for (;;)
  {
    final = *head;
    *head++ = chrono;

    if (head >= tail)
      return;

    begin = *head;
    *head++ = prev;

    if (head >= tail)
      return;

    chrono = final;
    prev = begin;
  }
}


/* ----------------------------------------------------- */
/* board permission check				 */
/* ----------------------------------------------------- */


int			/* >=1:第幾個板主 0:不是板主 */
is_bm(list, userid)
  char *list;		/* 板主：BM list */
  char *userid;
{
  return str_has(list, userid, strlen(userid));
}


int
is_brd_public(brdname)
  char *brdname;
{
#ifdef SITE_LEXEL
  return 0;
#else
  return (strncmp(brdname, "P_", 2) && strncmp(brdname, "L_", 2) &&
	  strncmp(brdname, "R_", 2) && strncmp(brdname, "G_", 2) &&
	  strncmp(brdname, "La_", 3) && strncmp(brdname, "LAB_", 4) &&
	  strncmp(brdname, "IS_", 3) && strncmp(brdname, "IA_", 3) && strncmp(brdname, "IAS_", 4));
#endif
}


#ifdef DO_POST_FILTER
int
IS_BIGGER_AGE(age)
  int age;
{
  time_t now;
  struct tm *ptime;

  if (age && !cuser.year)	/* 生日未填 */
    return 0;

  time(&now);
  ptime = localtime(&now);

  return ((cuser.year + 11 + age) < ptime->tm_year) ? 1 :
	((cuser.year + 11 + age) > ptime->tm_year) ? 0 :
	(cuser.month < (ptime->tm_mon + 1))        ? 1 :
	(cuser.month > (ptime->tm_mon + 1))        ? 0 :
	(cuser.day > (ptime->tm_mday))             ? 0 : 1;
}


int
IS_BIGGER_1STLG(month)
  int month;
{
  /* 一個月以 30 天計算 */
  return (((time(0) - cuser.firstlogin) / (86400 * 30)) >= month);
}


static int
IS_WELCOME(perm)
  BPERM *perm;
{
  if (!perm->exist)
    return 1;

  while (1)
  {
    if (!IS_BIGGER_AGE(perm->age))
      break;

    if (perm->sex && (perm->sex != cuser.sex + 1))
      break;

    if (cuser.numlogins < perm->numlogins)
      break;

    if (cuser.numposts < perm->numposts)
      break;

    if (cuser.good_article < perm->good_article)
      break;

    if (perm->poor_article && (cuser.poor_article >= perm->poor_article))
      break;

    if (perm->violation && (cuser.violation >= perm->violation))
      break;

    if (cuser.money < perm->money)
      break;

    if (cuser.gold < perm->gold)
      break;

    if (cuser.numemails < perm->numemails)
      break;

    if (!IS_BIGGER_1STLG(perm->regmonth))
      break;

    /* 會走到這裡就表示過關了 */
    return 1;
  }

  return 0;
}
#endif


static inline int
Ben_Perm(bno, ulevel)
  int bno;
  usint ulevel;
{
  usint readlevel, postlevel, bits;
  char *blist, *bname;
  BRD *brd;
#ifdef HAVE_MODERATED_BOARD
  BPAL *bpal;
  int ftype;	/* 0:一般ID 1:板好 2:板壞 */

  /* songsongboy.060327: 看板閱讀等級說明表

  ┌────┬────┬────┬────┬────┐
  │        │一般用戶│看板好友│好友壞人│看板壞人│
  ├────┼────┼────┼────┼────┤
  │一般看板│權限決定│  完整  │ 水  桶 │ 看不見 │
  ├────┼────┼────┼────┼────┤
  │好友看板│ 進不去 │  完整  │ 水  桶 │ 看不見 │
  ├────┼────┼────┼────┼────┤
  │秘密看板│ 看不見 │  完整  │ 水  桶 │ 看不見 │
  └────┴────┴────┴────┴────┘
  看不見：在看板列表中無法看到這個板，也進不去
  進不去：在看板列表中可以看到這個板，但是進不去
  水  桶：在看板列表中可以看到這個板，也進得去，但是不能發文
  完  整：在看板列表中可以看到這個板，也進得去及發文

  */

  static int bit_data[12] =
  {                /* 一般用戶   看板好友                            特殊好友(水桶)           看板壞人 */
    /* 公開看板 */    0,         BRD_L_BIT | BRD_R_BIT | BRD_W_BIT, BRD_L_BIT | BRD_R_BIT, 0,
    /* 好友看板 */    BRD_L_BIT, BRD_L_BIT | BRD_R_BIT | BRD_W_BIT, BRD_L_BIT | BRD_R_BIT, 0,
    /* 秘密看板 */    0,         BRD_L_BIT | BRD_R_BIT | BRD_W_BIT, BRD_L_BIT | BRD_R_BIT, 0
  };
#endif

  brd = bshm->bcache + bno;
  bname = brd->brdname;
  if (!*bname)
    return 0;

  readlevel = brd->readlevel;

#ifdef HAVE_MODERATED_BOARD
  bpal = bshm->pcache + bno;
  ftype = is_bbad(bpal) ? 3 : is_bgood(bpal) ? 1 : is_bmate(bpal) ? 2 : 0;

  if (readlevel == PERM_SYSOP)		/* 秘密看板 */
    bits = bit_data[8 + ftype];
  else if (readlevel == PERM_BOARD)	/* 好友看板 */
    bits = bit_data[4 + ftype];
  else if (ftype)			/* 公開看板，若在板好/板壞名單中 */
    bits = bit_data[ftype];
  else					/* 公開看板，其他依權限判定 */
#endif

  if (!readlevel || (readlevel & ulevel))
  {
    bits = BRD_L_BIT | BRD_R_BIT;

    postlevel = brd->postlevel;
    if (!postlevel || (postlevel & ulevel))	/* 全站應只有 sysop 板沒有 postlevel */
      bits |= BRD_W_BIT;

#ifdef DO_POST_FILTER
    if (!IS_WELCOME(bshm->lperm + bno))
      bits &= ~(BRD_L_BIT | BRD_R_BIT | BRD_W_BIT);
    else if (!IS_WELCOME(bshm->rperm + bno))
      bits &= ~(BRD_R_BIT | BRD_W_BIT);
    else if (!IS_WELCOME(bshm->wperm + bno))
      bits &= ~BRD_W_BIT;
#endif
  }
  else					/* 有設看板讀取權限的板， guest 就無法看見 */
  {
    bits = 0;
  }

  /* Thor.980813.註解: 特別為 BM 考量，板主有該板的所有權限 */
  blist = brd->BM;
  if ((ulevel & PERM_BM) && blist[0] > ' ' && is_bm(blist, cuser.userid))
    bits = BRD_L_BIT | BRD_R_BIT | BRD_W_BIT | BRD_X_BIT | BRD_M_BIT;

  /* smiler.080811: ATOM 成員於 ATOM 看板具備全部屬性 */
  else if ((ulevel & PERM_ATOM) && (brd->battr & BRD_ATOM))
    bits = BRD_L_BIT | BRD_R_BIT | BRD_W_BIT | BRD_X_BIT | BRD_M_BIT;

  else if (ulevel & PERM_ALLBOARD)
  {
    bits |= BRD_L_BIT;
    if (!(readlevel & (PERM_SYSOP | PERM_BOARD)))	/* 站長/看板總管於公開板有管理權限 */
      bits |= BRD_R_BIT | BRD_W_BIT | BRD_X_BIT;
  }

  return bits;
}


/* ----------------------------------------------------- */
/* 載入 currboard 進行若干設定				 */
/* ----------------------------------------------------- */


int
bstamp2bno(stamp)
  time4_t stamp;
{
  BRD *brd;
  int bno, max;

  bno = 0;
  brd = bshm->bcache;
  max = bshm->number;
  for (;;)
  {
    if (stamp == brd->bstamp)
      return bno;
    if (++bno >= max)
      return -1;
    brd++;
  }
}


static inline void
brh_load()
{
  BRD *brdp;
  usint ulevel;
  int n, cbno;
  char *bits;

  int size, *base;
  time4_t expire;
  char fpath[64];

#ifndef ENHANCED_VISIT
  time4_t *bstp;
#endif

  memset(bits = brd_bits, 0, sizeof(brd_bits));
#ifndef ENHANCED_VISIT
  memset(bstp = brd_visit, 0, sizeof(brd_visit));
#endif

  ulevel = cuser.userlevel;
  n = 0;
  cbno = bshm->number;

  do
  {
    *bits++ = Ben_Perm(n, ulevel);
  } while (++n < cbno);

  /* --------------------------------------------------- */
  /* 將 .BRH 載入 memory				 */
  /* --------------------------------------------------- */

  size = 0;
  cbno = -1;
  brh_expire = expire = time(0) - BRH_EXPIRE * 86400;

  if (ulevel)
  {
    struct stat st;

    usr_fpath(fpath, cuser.userid, FN_BRH);
    if (!stat(fpath, &st))
      size = st.st_size;
  }

  /* --------------------------------------------------- */
  /* 多保留 BRH_WINDOW 的運作空間			 */
  /* --------------------------------------------------- */

  /* brh_size = n = ((size + BRH_WINDOW) & -BRH_PAGE) + BRH_PAGE; */
  brh_size = n = size + BRH_WINDOW;
  brh_base = base = (int *) malloc(n);

  if (size && ((n = open(fpath, O_RDONLY)) >= 0))
  {
    int *head, *tail, *list, bstamp, bhno;

    size = read(n, base, size);
    close(n);

    /* compact reading history : remove dummy/expired record */

    head = base;
    tail = (int *) ((char *) base + size);
    bits = brd_bits;

    while (head < tail && head >= brh_base)
    {
      bstamp = *head;

      if (bstamp & BRH_SIGN)	/* zap */
      {
	bstamp ^= BRH_SIGN;
	bhno = bstamp2bno(bstamp);
	if (bhno >= 0)
	{
	  /* itoc.001029: NOZAP時, 仍會出現 */
	  brdp = bshm->bcache + bhno;
	  if (!(brdp->battr & BRD_NOZAP))
	    bits[bhno] |= BRD_Z_BIT;
	}
	head++;
	continue;
      }

      bhno = bstamp2bno(bstamp);
      list = head + 2;

      if (list > tail)
	break;

      n = *list;
      size = n + 3;

      /* 這個看板存在、沒有被 zap 掉、可以 list */

      if (bhno >= 0 && (bits[bhno] & BRD_L_BIT))
      {
	bits[bhno] |= BRD_H_BIT;/* 已有閱讀記錄 */

#ifndef ENHANCED_VISIT
	bstp[bhno] = head[1];	/* 上次閱讀時間 */
#endif

	cbno = bhno;

	if (n > 0)
	{
	  list += n;	/* Thor.980904.註解: 最後一個 tag */

	  if (list > tail)
	    break;

	  do
	  {
	    bhno = *list;
	    if ((bhno & BRH_MASK) > expire)
	      break;

	    if (!(bhno & BRH_SIGN))
	    {
	      if (*--list > expire)
		break;
	      n--;
	    }

	    list--;
	    n--;
	  } while (n > 0);

	  head[2] = n;
	}

	n = n * sizeof(time4_t) + sizeof(BRH);
	if (base != head)
	  memcpy(base, head, n);
	base = (int *) ((char *) base + n);
      }
      head += size;
    }
  }

  *base = 0;
  brh_tail = base;
}


void
brh_save()
{
  int *base, *head, *tail, bhno, size;
  BRD *bhdr, *bend;
  char *bits;

  /* Thor.980830: lkchu patch:  還沒 load 就不用 save */
  if (!(base = brh_base))
    return;

  brh_put();

  /* save history of un-zapped boards */

  bits = brd_bits;
  head = base;
  tail = brh_tail;
  while (head < tail)
  {
    bhno = bstamp2bno(*head);
    size = head[2] * sizeof(time4_t) + sizeof(BRH);
    if (bhno >= 0 && !(bits[bhno] & BRD_Z_BIT))
    {
      if (base != head)
	memcpy(base, head, size);
      base = (int *) ((char *) base + size);
    }
    head = (int *) ((char *) head + size);
  }

  /* save zap record */

  tail = brh_alloc(base, sizeof(time4_t) * MAXBOARD);

  bhdr = bshm->bcache;
  bend = bhdr + bshm->number;
  do
  {
    if (*bits++ & BRD_Z_BIT)
    {
      *tail++ = bhdr->bstamp | BRH_SIGN;
    }
  } while (++bhdr < bend);

  /* OK, save it */

  base = brh_base;
  if ((size = (char *) tail - (char *) base) > 0)
  {
    char fpath[64];
    int fd;

    usr_fpath(fpath, cuser.userid, FN_BRH);
    if ((fd = open(fpath, O_WRONLY | O_CREAT | O_TRUNC, 0600)) >= 0)
    {
      write(fd, base, size);
      close(fd);
    }
  }
}


/* ----------------------------------------------------- */
/* 分類 zap 記錄 .CZH (Class Zap History)		 */
/* ----------------------------------------------------- */


typedef struct ClassZapHistory
{
  char brdname[BNLEN + 1];	/* 分類的 brdname */
}                   CZH;


static char class_bits[CH_MAX + 3];


static int			/* >=0:pos  -1:不在 .CZH */
czh_find(fpath, brdname)	/* 檢查是否已經在 .CZH 內 */
  char *fpath;
  char *brdname;
{
  CZH czh;
  int fd, pos;
  int rc = -1;

  if ((fd = open(fpath, O_RDONLY)) >= 0)
  {
    pos = 0;
    while (read(fd, &czh, sizeof(CZH)) == sizeof(CZH))
    {
      if (!strcmp(czh.brdname, brdname))
      {
	rc = pos;
	break;
      }
      pos++;
    }
    close(fd);
  }
  return rc;
}


static int		/* >=0:在 .CZH  -1:不在 .CZH */
czh_put(brdname)
  char *brdname;
{
  char fpath[64];
  int pos;

  usr_fpath(fpath, cuser.userid, FN_CZH);

  /* 若已經在 .CZH 內的話，則刪除此筆資料；若不在 .CZH 內的話，則加入此筆資料 */
  if ((pos = czh_find(fpath, brdname)) >= 0)
    rec_del(fpath, sizeof(CZH), pos, NULL);
  else
    rec_add(fpath, brdname, sizeof(CZH));

  return pos;
}


static void
czh_load()
{
  int fsize, chn, min_chn;
#ifdef	DEBUG_ClassHeader_INT
  int *chx;
#else
  short *chx;
#endif
  char *img, fpath[64];
  CZH *chead, *ctail, *czh;

  memset(class_bits, 0, sizeof(class_bits));

  usr_fpath(fpath, cuser.userid, FN_CZH);
  if (chead = (CZH *) f_img(fpath, &fsize))
  {
    min_chn = bshm->min_chn;

    czh = chead;
    ctail = chead + fsize / sizeof(CZH);
    img = class_img;
    do
    {
      for (chn = CH_END - 2;; chn--)	/* 在所有的分類中找一個板名相同的 */
      {
#ifdef	DEBUG_ClassHeader_INT
        chx = (int *) img + (CH_END - chn);
#else
	chx = (short *) img + (CH_END - chn);
#endif
	if (!strncmp(czh->brdname, img + *chx, BNLEN))
	{
	  class_bits[-chn] |= BRD_Z_BIT;
	  break;
	}
	if (chn <= min_chn)	/* 如果找不到，表示該分類已消失，從 .CZH 刪除 */
	{
	  czh_put(czh->brdname);
	  break;
	}
      }
    } while (++czh < ctail);
    free(chead);
  }
}


/*-------------------------------------------------------*/

#ifdef LOG_BRD_USIES	/* lkchu.981201: 看板閱讀記錄 */
static void
brd_usies()
{
  char buf[256];

  sprintf(buf, "%s %s (%s)\n", currboard, cuser.userid, Now());
  f_cat(FN_RUN_BRDUSIES, buf);
}
#endif


int
last_nobottom(folder)	/* 找看板最後一篇非置底文的位置 */
  char *folder;
{
  int fd, fsize;
  struct stat st;
  HDR hdr;

  if ((fd = open(folder, O_RDONLY)) >= 0)
  {
    if (!fstat(fd, &st) && (fsize = st.st_size) >= sizeof(HDR))
    {
      while ((fsize -= sizeof(HDR)) >= 0)
      {
	lseek(fd, fsize, SEEK_SET);
	read(fd, &hdr, sizeof(HDR));
	if (!(hdr.xmode & POST_BOTTOM))
	  break;
      }
    }
    close(fd);

    return fsize / sizeof(HDR);
  }

  return XO_TALL;
}


int
XoPost(bno)
  int bno;
{
  XO *xo;
  BRD *brd;
  int bits;
  char *str, fpath[64];
  char BMstr[17];	/*by ryancid 砍到剩16*/
  char reason[64];

  brd = bshm->bcache + bno;
  if (!brd->brdname[0])	/* 已刪除的看板 */
    return -1;

  bits = brd_bits[bno];

  if (currbno != bno)	/* 看板沒換通常是因為 every_Z() 回原看板 */
  {
#ifdef HAVE_UNANONYMOUS_BOARD
    /* 070403.songsongboy：不能亂看反匿名版 */
    if (!strcmp(brd->brdname, BN_UNANONYMOUS))
    {
      if (vans("您確定要進入反匿名版嗎(y/n)？[N] ") != 'y')
	return -1;

      vmsg("請輸入觀看反匿名版之理由");
      sprintf(reason, "理由：");
      vget(b_lines, 0, "請輸入設定資料的理由：", reason + 6, 50, DOECHO);
      alog("反匿名版", reason);
    }
#endif
#ifdef HAVE_MODERATED_BOARD
    if (!(bits & BRD_R_BIT))
    {
      if ((cuser.userlevel & PERM_SYSOP) && (is_brd_public(brd->brdname)))
      {
	vmsg("警告：使用站務進入隱藏公眾板功\能");
	if (!adm_check())
	  return -1;
	sprintf(reason, "%-*s 板：", BNLEN + 1, brd->brdname);
	vget(b_lines, 0, "請輸入進入隱板的理由：", reason + BNLEN + 6, 50, DOECHO);
	alog("進入隱板", reason);
	bits |= BRD_R_BIT | BRD_X_BIT;
      }
      else
      {
	vmsg("對不起，此板只准板友進入，請向板主申請入境許\可");
	return -1;
      }
    }
#endif

    /* 處理權限 */
    bbstate = STAT_STARTED;
    if (bits & BRD_M_BIT)
      bbstate |= (STAT_BM | STAT_BOARD | STAT_POST);
    else if (bits & BRD_X_BIT)
      bbstate |= (STAT_BOARD | STAT_POST);
    else if (bits & BRD_W_BIT)
      bbstate |= STAT_POST;

#ifdef LOG_BRD_USIES
    /* lkchu.981201: 閱讀看板記錄 */
    brd_usies();
#endif

    /* itoc.050613.註解: 人氣的減少不是在離開看板時，而是在進入新的看板或是離站時，
       這是為了避免 switch 跳看板會算錯人氣 */
    if (currbno >= 0)
      bshm->mantime[currbno]--;		/* 退出上一個板 */
    bshm->mantime[bno]++;		/* 進入新的板 */

    currbno = bno;
    currbattr = brd->battr;
    strcpy(currboard, brd->brdname);
    str = brd->BM;

    str_ncpy(BMstr, str, 17);
    if (strlen(str) > 16)
    {
      BMstr[14] = 0xA1;
      BMstr[15] = 0x4B;
    }	/* 印出 "…" */

    sprintf(currBM, "板主：%s", *str <= ' ' ? "徵求中" : BMstr);
#ifdef HAVE_BRDMATE
    strcpy(cutmp->reading, currboard);
#endif

    brd_fpath(fpath, currboard, fn_dir);

#ifdef AUTO_JUMPPOST
    xz[XZ_POST - XO_ZONE].xo = xo = xo_get_post(fpath, brd);	/* itoc.010910: 為 XoPost 量身打造一支 xo_get() */
#else
    xz[XZ_POST - XO_ZONE].xo = xo = xo_get(fpath);
#endif
    xo->key = XZ_POST;
    xo->xyz = brd->title;
  }

  /* itoc.011113: 第一次進板一定要看進板畫面，第二次以後則取決 ufo 設定 */
  if (!(bits & BRD_V_BIT) || (cuser.ufo & UFO_BRDNOTE))
  {
    brd_bits[bno] = bits | BRD_V_BIT;
    brd_fpath(fpath, currboard, fn_note);
    more(fpath, NULL);
  }

  brh_get(brd->bstamp, bno);

  return 0;
}


#ifdef HAVE_FORCE_BOARD
void
brd_force()			/* itoc.010407: 強制閱讀公告板，且強迫讀最後一篇 */
{
  if (cuser.userlevel)		/* guest 跳過 */
  {
    int bno;
    BRD *brd;

    if ((bno = brd_bno(BN_ANNOUNCE)) < 0)
      return;
    brd = bshm->bcache + bno;
    if (brd->btime < 0)		/* 尚未更新 brd->blast 就不強制閱讀公告板 */
      return;

#ifdef ENHANCED_VISIT
    brh_get(brd->bstamp, bno);
    while (brh_unread(brd->blast))
#else
    if (brd->blast > brd_visit[bno])
#endif
    {
      vmsg("有新公告！請先閱\讀完新公告後再離開");
      XoPost(bno);
      xover(XZ_POST);

#ifndef ENHANCED_VISIT
      time4(&brd_visit[bno]);
#endif
    }
  }
}
#endif	/* HAVE_FORCE_BOARD */


/* ----------------------------------------------------- */
/* Class [分類群組]					 */
/* ----------------------------------------------------- */


#ifdef MY_FAVORITE
int class_flag = 0;			/* favorite.c 要用 */
#else
static int class_flag = 0;
#endif


#ifdef AUTO_JUMPBRD
static int class_jumpnext = 0;	/* itoc.010910: 是否跳去下一個未讀板 1:要 0:不要 */
#endif


static int class_hot = 0;

static int
mantime_cmp(a, b)	/* smiler.070602: 看板人氣排序 */
#ifdef	DEBUG_ClassHeader_INT
int *a;
int *b;
#else
short *a;
short *b;
#endif
{
  return bshm->mantime[*b] - bshm->mantime[*a];
}


#define	BFO_YANK	0x01


static int
class_load(xo)
  XO *xo;
{
#ifdef	DEBUG_ClassHeader_INT
  int *cbase, *chead, *ctail;
#else
  short *cbase, *chead, *ctail;
#endif
  int chn;			/* ClassHeader number */
  int pos, max, val, zap;
  int bnum = 0;			/* smiler.070602: for熱門看板 */
  BRD *brd;
  char *bits;

  /* lkchu.990106: 防止 未用 account 造出 class.img 或沒有 class 的情況 */
  if (!class_img)
    return 0;

  chn = CH_END - xo->key;

#ifdef	DEBUG_ClassHeader_INT
  cbase = (int *) class_img;
#else
  cbase = (short *) class_img;
#endif
  chead = cbase + chn;

  pos = chead[0] + CH_TTLEN;
  max = chead[1];

#ifdef	DEBUG_ClassHeader_INT
  chead = (int *) ((char *) cbase + pos);
  ctail = (int *) ((char *) cbase + max);
#else
  chead = (short *) ((char *) cbase + pos);
  ctail = (short *) ((char *) cbase + max);
#endif

  max -= pos;

#ifdef	DEBUG_ClassHeader_INT
  if (cbase = (int *) xo->xyz)
    cbase = (int *) realloc(cbase, max);
  else
    cbase = (int *) malloc(max);
#else
  if (cbase = (short *) xo->xyz)
    cbase = (short *) realloc(cbase, max);
  else
    cbase = (short *) malloc(max);
#endif

  xo->xyz = (char *) cbase;

  max = 0;
  brd = bshm->bcache;
  bits = brd_bits;
  zap = (class_flag & BFO_YANK) ? 0 : BRD_Z_BIT;

  do
  {
    chn = *chead++;
    if (chn >= 0)	/* 一般看板 */
    {
      val = bits[chn];
      if (!(val & BRD_L_BIT) || (val & zap) || !(brd[chn].brdname[0]))
	continue;
      if (class_hot && bshm->mantime[chn] <= 2 && strcmp(brd[chn].brdname, "sysop"))
	continue;
    }
    else		/* 分類群組 */
    {
      if (class_bits[-chn] & zap)
	continue;
    }

    max++;
    *cbase++ = chn;

    if (chn >= 0)	/* smiler.070602: for熱門看板 */
      bnum++;
  } while (chead < ctail);

  if (class_hot && bnum > 0)
  {
    cbase -= bnum;
#ifdef	DEBUG_ClassHeader_INT
    xsort(cbase, bnum, sizeof(int), mantime_cmp);
#else
    xsort(cbase, bnum, sizeof(short), mantime_cmp);
#endif
  }

  xo->max = max;
  if (xo->pos >= max)
    xo->pos = xo->top = 0;

  return max;
}


static int
XoClass(chn)
  int chn;
{
  XO xo, *xt;

  /* Thor.980727: 解決 XO xo的不確定性,
                  class_load內部會 initial xo.max, 其他不確定 */
  xo.pos = xo.top = 0;

  xo.key = chn;
  xo.xyz = NULL;
  if (!class_load(&xo))
  {
    if (xo.xyz)
      free(xo.xyz);
    return 0;
  }

  xt = xz[XZ_CLASS - XO_ZONE].xo;
  xz[XZ_CLASS - XO_ZONE].xo = &xo;

#ifdef AUTO_JUMPBRD
  if (cuser.ufo & UFO_JUMPBRD)
    class_jumpnext = 1;	/* itoc.010910: 主動跳去下一個未讀看板 */
#endif
  xover(XZ_CLASS);

  free(xo.xyz);
  xz[XZ_CLASS - XO_ZONE].xo = xt;

  return 1;
}


//static inline void
inline void
btime_refresh(brd)
  BRD *brd;
{
  /* itoc.020123: 不管有無 UFO_BRDPOST，一概更新，以免未讀燈不會亮 */
  if (brd->btime < 0)
  {
    int fd, fsize;
    char folder[64];
    struct stat st;
    time4_t maxchrono;

    brd->btime = 1;
    brd_fpath(folder, brd->brdname, fn_dir);
    if ((fd = open(folder, O_RDONLY)) >= 0)
    {
      if (!fstat(fd, &st) && (fsize = st.st_size) >= sizeof(HDR))
      {
#ifdef ENHANCED_BSHM_UPDATE
	HDR hdr;

	brd->bpost = fsize / sizeof(HDR);
	/* itoc.020829: 找最後一篇未被加密、不是置底的 HDR */
	maxchrono = 0;
	while (read(fd, &hdr, sizeof(HDR)) == sizeof(HDR))
	{
	  if (!(hdr.xmode & (POST_RESTRICT | POST_FRIEND | POST_BOTTOM)))
	  {
	    maxchrono = BMAX(maxchrono, hdr.chrono);
	    maxchrono = BMAX(maxchrono, hdr.stamp);
	  }
	}
	brd->blast = maxchrono;
#else
	brd->bpost = fsize / sizeof(HDR);
	lseek(fd, fsize - sizeof(HDR), SEEK_SET);
	read(fd, &brd->blast, sizeof(time4_t));
#endif
      }
      else
      {
	brd->blast = brd->bpost = 0;
      }
      close(fd);
    }
  }
}


void
class_item(num, bno, brdpost, infav, label)	/* smiler.070724: 我的最愛看板上色*/
  int num, bno, brdpost, infav, label;
{
  BRD *brd;
  char *str1, *str2, *str3, token, buf[16];

  brd = bshm->bcache + bno;

  btime_refresh(brd);

  /* 處理 編號/篇數 */
  if (brdpost)
    num = brd->bpost;

  /* 處理 zap/friend/secret 板的符號 */
  if (brd_bits[bno] & BRD_Z_BIT)
    token = TOKEN_ZAP_BRD;
  else if (brd->readlevel == PERM_SYSOP)
    token = TOKEN_SECRET_BRD;
  else if (brd->readlevel == PERM_BOARD)
    token = TOKEN_FRIEND_BRD;
  else
    token = ' ';

  /* 處理 已讀/未讀 */
#ifdef ENHANCED_VISIT
  /* itoc.010407: 改用最後一篇已讀/未讀來判斷 */
  brh_get(brd->bstamp, bno);
  str1 = brh_unread(brd->blast) ? ICON_UNREAD_BRD : ICON_READ_BRD;
#else
  str1 = brd->blast > brd_visit[bno] ? ICON_UNREAD_BRD : ICON_READ_BRD;
#endif

  /* 處理 投票/轉信 */
  if (brd->bvote)
    str2 = (brd->bvote > 0) ? ICON_VOTED_BRD : ICON_GAMBLED_BRD;
  else
    str2 = (brd->battr & BRD_NOTRAN) ? ICON_NOTRAN_BRD : ICON_TRAN_BRD;

  /* 處理 人氣 */
  brdpost = bno;	/* 記住 bno */
  bno = bshm->mantime[bno];
  if (bno > 730)
    str3 = "\033[1;36m爆了\033[m";
  else if (bno > 400)
    str3 = "\033[1;35m爆了\033[m";
  else if (bno > 180)
    str3 = "\033[1;34m爆了\033[m";
  else if (bno > 90)
    str3 = "\033[1;32m爆了\033[m";
  else if (bno > 50)
    str3 = "\033[1;31m熱門\033[m";
  else if (bno > 25)
    str3 = "\033[1;33m有勁\033[m";
  else if (bno > 10)
    sprintf(str3 = buf, "\033[1;31m%4d\033[m", bno);
  else if (bno > 5)
    sprintf(str3 = buf, "\033[1;33m%4d\033[m", bno);
  else if (bno > 0)
    sprintf(str3 = buf, "%4d", bno);
  else
    str3 = "";

  /* smiler.070724: 看板配色 */
  if (!infav)
  {
    prints("%6d%c%s", num, token, str1);

    /* smiler.080712: 處理隱板可見顯色 */
    if (((brd->readlevel == PERM_BOARD) || (brd->readlevel == PERM_SYSOP)) && !(brd_bits[brdpost] & BRD_R_BIT))
      prints("\033[m\033[1;30m%-13s\033[m", brd->brdname);
    else
      prints("%-13s", brd->brdname);
  }
  else	/* smiler.070724: 我的最愛看板另外上色*/
    prints("%6d%c%s\033[1;36m%-13s\033[m", num, label ? 'T' : token, str1, brd->brdname);

  if (!strcmp(brd->class, "楓橋") || !strcmp(brd->class, "系統"))
    prints("\033[1;31m");
  else if (!strcmp(brd->class, "地方") || !strcmp(brd->class, "體育"))
    prints("\033[1;32m");
  else if (!strcmp(brd->class, "個人"))
    prints("\033[1;33m");
  else if (!strcmp(brd->class, "清華") || !strcmp(brd->class, "資訊") || !strcmp(brd->class, "校隊"))
    prints("\033[1;34m");
  else if (!strcmp(brd->class, "Comp") || !strcmp(brd->class, "電腦"))
    prints("\033[1;36m");
  else if (!strcmp(brd->class, "社團"))
    prints("\033[33m");
  else if (!strcmp(brd->class, "嗜好"))
    prints("\033[35m");
  else
    prints("\033[1;3%dm", brd->class[3] & 7);

  prints("%-5s\033[m%s ", brd->class, str2);

  /* itoc.060530: 借用 str1、num 來處理看板敘述顯示的中文斷字 */
  str1 = brd->title;
  num = (d_cols >> 1) + 31;	/* smiler.070724: 33->31 for 中文板名減短 */
  prints("%-*.*s", num, IS_ZHC_LO(str1, num - 1) ? num - 2 : num - 1, str1);

  prints("%-4s %.*s\n", str3, d_cols - (d_cols >> 1) + 12, brd->BM);
}


#ifdef HAVE_LIGHTBAR
#ifdef MY_FAVORITE
void
#else
static void
#endif
class_item_bar(brd, bno, chn, brdpost, infav, label)
  BRD *brd;
  int bno, chn, brdpost, infav, label;
{
  int num;
  char *str1, *str2, *str3, token, buf[16];

  btime_refresh(brd);

  /* 處理 編號/篇數 */
  num = brdpost ? brd->bpost : bno;

  /* 處理 zap/friend/secret 板的符號 */
  if (brd_bits[chn] & BRD_Z_BIT)
    token = TOKEN_ZAP_BRD;
  else if (brd->readlevel == PERM_SYSOP)
    token = TOKEN_SECRET_BRD;
  else if (brd->readlevel == PERM_BOARD)
    token = TOKEN_FRIEND_BRD;
  else
    token = ' ';

  /* 處理 已讀/未讀 */
#ifdef ENHANCED_VISIT
  /* itoc.010407: 改用最後一篇已讀/未讀來判斷 */
  brh_get(brd->bstamp, chn);
  str1 = brh_unread(brd->blast) ? ICON_UNREAD_BRD : ICON_READ_BRD;
#else
  str1 = brd->blast > brd_visit[chn] ? ICON_UNREAD_BRD : ICON_READ_BRD;
#endif

  /* 處理 投票/轉信 */
  if (brd->bvote)
    str2 = (brd->bvote > 0) ? ICON_VOTED_BRD : ICON_GAMBLED_BRD;
  else
    str2 = (brd->battr & BRD_NOTRAN) ? ICON_NOTRAN_BRD : ICON_TRAN_BRD;

  /* 處理 人氣 */
  bno = bshm->mantime[chn];
  if (bno > 730)
    str3 = "\033[1;36m爆了\033[m";
  else if (bno > 400)
    str3 = "\033[1;35m爆了\033[m";
  else if (bno > 180)
    str3 = "\033[1;34m爆了\033[m";
  else if (bno > 90)
    str3 = "\033[1;32m爆了\033[m";
  else if (bno > 50)
    str3 = "\033[1;31m熱門\033[m";
  else if (bno > 25)
    str3 = "\033[1;33m有勁\033[m";
  else if (bno > 10)
    sprintf(str3 = buf, "\033[1;31m%4d\033[m", bno);
  else if (bno > 5)
    sprintf(str3 = buf, "\033[1;33m%4d\033[m", bno);
  else if (bno > 0)
    sprintf(str3 = buf, "%4d", bno);
  else
    str3 = "";

  /*smiler.070724: 看板配色 */
  if (!infav)
  {
    prints("\033[m%s%6d%c%s%s", UCBAR[UCBAR_BRD], num, token, str1, UCBAR[UCBAR_BRD]);

    /* smiler.080712: 處理隱板可見顯色 */
    if (((brd->readlevel == PERM_BOARD) || (brd->readlevel == PERM_SYSOP)) && !(brd_bits[chn] & BRD_R_BIT))
      prints("\033[1;30m%-13s\033[m%s", brd->brdname, UCBAR[UCBAR_BRD]);
    else
      prints("%-13s", brd->brdname);
  }
  else	/* smiler.070724: 我的最愛看板另外上色*/
    prints("\033[m%s%6d%c%s%s\033[1;36m%-13s\033[m%s",
      UCBAR[UCBAR_BRD], num, label ? 'T' : token, str1,
      UCBAR[UCBAR_BRD], brd->brdname, UCBAR[UCBAR_BRD]);

  if (!strcmp(brd->class, "楓橋") || !strcmp(brd->class, "系統"))
    prints("\033[1;31m");
  else if (!strcmp(brd->class, "地方") || !strcmp(brd->class, "體育"))
    prints("\033[1;32m");
  else if (!strcmp(brd->class, "個人"))
    prints("\033[1;33m");
  else if (!strcmp(brd->class, "清華") || !strcmp(brd->class, "資訊") || !strcmp(brd->class, "校隊"))
    prints("\033[1;34m");
  else if (!strcmp(brd->class, "Comp") || !strcmp(brd->class, "電腦"))
    prints("\033[1;36m");
  else if (!strcmp(brd->class, "社團"))
    prints("\033[33m");
  else if (!strcmp(brd->class, "嗜好"))
    prints("\033[35m");
  else
    prints("\033[1;3%dm", brd->class[3] & 7);

  prints("%-5s\033[m%s%s%s ", brd->class, UCBAR[UCBAR_BRD], str2, UCBAR[UCBAR_BRD]);

  /* itoc.060530: 借用 str1、num 來處理看板敘述顯示的中文斷字 */
  str1 = brd->title;
  num = (d_cols >> 1) + 31; /* smiler.070724: 33->31 for 中文板名減短 */
  prints("%-*.*s", num, IS_ZHC_LO(str1, num - 1) ? num - 2 : num - 1, str1);

  /* smiler.070724: 獨立處理看板人氣 */
  prints("%s%-4s%s ", UCBAR[UCBAR_BRD], str3, UCBAR[UCBAR_BRD]);

  prints("%-*.*s\033[m", d_cols - (d_cols >> 1) + 12, d_cols - (d_cols >> 1) + 12, brd->BM);
}


static int
class_bar(xo, mode)
  XO *xo;
  int mode;
{
#ifdef	DEBUG_ClassHeader_INT
  int *chp;
#else
  short *chp;
#endif
  BRD *brd;
  int chn, cnt, brdpost;

  cnt = xo->pos + 1;
#ifdef	DEBUG_ClassHeader_INT
  chp = (int *) xo->xyz + xo->pos;
#else
  chp = (short *) xo->xyz + xo->pos;
#endif
  chn = *chp;
  brd = bshm->bcache + chn;
  brdpost = class_flag & UFO_BRDPOST;

  if (chn >= 0)		/* 一般看板 */
  {
    if (mode)
      class_item_bar(brd, cnt, chn, brdpost, in_favor(brd->brdname), 0);
    else
      class_item(cnt, chn, brdpost, in_favor(brd->brdname), 0);
  }
  else
  {
#ifdef	DEBUG_ClassHeader_INT
    int *chx;
#else
    short *chx;
#endif
    char *img, *str;

    img = class_img;
#ifdef	DEBUG_ClassHeader_INT
    chx = (int *) img + (CH_END - chn);
#else
    chx = (short *) img + (CH_END - chn);
#endif
    str = img + *chx;
    prints("%s%6d%c  %-13.13s\033[1;3%dm%5.5s\033[m%s%-*.*s%s",
      mode ? UCBAR[UCBAR_BRD] : "",
      cnt, class_bits[-chn] & BRD_Z_BIT ? TOKEN_ZAP_BRD : ' ',
      str, str[BNLEN + 4] & 7,str + BNLEN + 1,
      mode ? UCBAR[UCBAR_BRD] : "",
      d_cols + 51, d_cols + 50, str + BNLEN + 1 + BCLEN + 1,
      mode ? "\033[m" : "");
  }

  move(xo->pos - xo->top + 3, 0);
  return XO_NONE;
}
#endif


static int
class_body(xo)
  XO *xo;
{
#ifdef	DEBUG_ClassHeader_INT
  int *chp;
#else
  short *chp;
#endif
  BRD *bcache;
  int n, cnt, max, chn, brdpost;
#ifdef AUTO_JUMPBRD
  int nextpos;
#endif

  bcache = bshm->bcache;
  max = xo->max;
  cnt = xo->top;

#ifdef AUTO_JUMPBRD
  nextpos = 0;

  /* itoc.010910: 搜尋下一個未讀看板 */
  if (class_jumpnext)
  {
    class_jumpnext = 0;
    n = xo->pos;
#ifdef	DEBUG_ClassHeader_INT
    chp = (int *) xo->xyz + n;
#else
    chp = (short *) xo->xyz + n;
#endif

    while (n < max)
    {
      chn = *chp++;
      if (chn >= 0)
      {
	BRD *brd;

	brd = bcache + chn;

#ifdef ENHANCED_VISIT
	/* itoc.010407: 改用最後一篇已讀/未讀來判斷 */
	brh_get(brd->bstamp, chn);
	if (brh_unread(brd->blast))
#else
	if (brd->blast > brd_visit[chn])
#endif
	{
	  nextpos = n;
	  break;
	}
      }
      n++;
    }

    /* 下一個未讀板在別頁，要翻過去 */
    if (nextpos >= cnt + XO_TALL)
      return nextpos + XO_MOVE;
  }
#endif

  brdpost = class_flag & UFO_BRDPOST;
#ifdef	DEBUG_ClassHeader_INT
  chp = (int *) xo->xyz + cnt;
#else
  chp = (short *) xo->xyz + cnt;
#endif

  n = 3;
  move(3, 0);
  do
  {
    chn = *chp;
    if (cnt < max)
    {
      clrtoeol();
      cnt++;
      if (chn >= 0)		/* 一般看板 */
      {
	 /*smiler.070724: 我的最愛看板板名上色 */
	 BRD *bhdr;
	 bhdr = bshm->bcache + chn;
	 class_item(cnt, chn, brdpost, in_favor(bhdr->brdname), 0);
      }
      else			/* 分類群組 */
      {
#ifdef	DEBUG_ClassHeader_INT
        int *chx;
#else
	short *chx;
#endif
	char *img, *str;

	img = class_img;
#ifdef	DEBUG_ClassHeader_INT
        chx = (int *) img + (CH_END - chn);
#else
	chx = (short *) img + (CH_END - chn);
#endif
	str = img + *chx;
	prints("%6d%c  %-13.13s\033[1;3%dm%-5.5s\033[m%s\n",
	  cnt, class_bits[-chn] & BRD_Z_BIT ? TOKEN_ZAP_BRD : ' ',
	  str, str[BNLEN + 4] & 7, str + BNLEN + 1, str + BNLEN + 1 + BCLEN + 1);
      }
      chp++;
    }
    else
    {
      clrtobot();
      break;
    }
  } while (++n < b_lines);

#ifdef AUTO_JUMPBRD
  /* itoc.010910: 下一個未讀板在本頁，要把游標移過去 */
  outf(FEETER_CLASS);
  return nextpos ? nextpos + XO_MOVE : XO_NONE;
#else
  /* return XO_NONE; */
  return XO_FOOT;	/* itoc.010403: 把 b_lines 填上 feeter */
#endif
}


static int
class_neck(xo)
  XO *xo;
{
  move(1, 0);
  prints(NECKER_CLASS,
    class_flag & UFO_BRDPOST ? "總數" : "編號",
    d_cols >> 1, "", d_cols - (d_cols >> 1), "");
  return class_body(xo);
}


static int
class_head(xo)
  XO *xo;
{
  vs_head("看板列表", str_site);
  return class_neck(xo);
}


static int
class_init(xo)			/* re-init */
  XO *xo;
{
  class_load(xo);
  return class_head(xo);
}


static int
class_postmode(xo)
  XO *xo;
{
  cuser.ufo ^= UFO_BRDPOST;
  cutmp->ufo = cuser.ufo;
  class_flag ^= UFO_BRDPOST;
  return class_neck(xo);
}


static int
class_namemode(xo)		/* itoc.010413: 看板依照字母/分類排列 */
  XO *xo;
{
  static time_t last = 0;
  time_t now;

  if (time(&now) - last < 10)
  {
    vmsg("每十秒鐘只能切換一次");
    return XO_FOOT;
  }
  last = now;

  if (cuser.userlevel)
    brh_save();			/* itoc.010711: 儲存閱讀記錄檔 */
  cuser.ufo ^= UFO_BRDNAME;
  cutmp->ufo = cuser.ufo;
  board_main();			/* 重新載入 class_img */
  return class_neck(xo);
}


static int
class_help(xo)
  XO *xo;
{
  xo_help("class");
  return class_head(xo);
}


static int
class_search(xo)
  XO *xo;
{
  int num, pos, max;
  char buf[BNLEN + 1];

  if (vget(b_lines, 0, MSG_BID, buf, BNLEN + 1, DOECHO))
  {
#ifdef	DEBUG_ClassHeader_INT
    int *chp, chn;
#else
    short *chp, chn;
#endif
    BRD *bcache, *brd;

    str_lowest(buf, buf);

    bcache = bshm->bcache;
    pos = num = xo->pos;
    max = xo->max;
#ifdef	DEBUG_ClassHeader_INT
    chp = (int *) xo->xyz;
#else
    chp = (short *) xo->xyz;
#endif

    do
    {
      if (++pos >= max)
	pos = 0;
      chn = chp[pos];
      if (chn >= 0)
      {
	brd = bcache + chn;
	if (str_str(brd->brdname, buf) || str_sub(brd->title, buf))
	{
	  outf(FEETER_CLASS);	/* itoc.010913: 把 b_lines 填上 feeter */
	  return pos + XO_MOVE;
	}
      }
    } while (pos != num);
  }

  return XO_FOOT;
}


static int
class_searchBM(xo)
  XO *xo;
{
  int num, pos, max;
  char buf[IDLEN + 1];

  if (vget(b_lines, 0, "請輸入板主：", buf, IDLEN + 1, DOECHO))
  {
#ifdef	DEBUG_ClassHeader_INT
    int *chp, chn;
#else
    short *chp, chn;
#endif
    BRD *bcache, *brd;

    str_lower(buf, buf);

    bcache = bshm->bcache;
    pos = num = xo->pos;
    max = xo->max;
#ifdef	DEBUG_ClassHeader_INT
    chp = (int *) xo->xyz;
#else
    chp = (short *) xo->xyz;
#endif

    do
    {
      if (++pos >= max)
	pos = 0;
      chn = chp[pos];
      if (chn >= 0)
      {
	brd = bcache + chn;
	if (str_str(brd->BM, buf))
	{
	  outf(FEETER_CLASS);	/* itoc.010913: 把 b_lines 填上 feeter */
	  return pos + XO_MOVE;
	}
      }
    } while (pos != num);
  }

  return XO_FOOT;
}


static int
class_yank(xo)
  XO *xo;
{
  /* itoc.001029: 所有的class,board列表下, key < 0, 1 則為找尋作者模式
                  使其不能跑 XO_INIT(�堶悸構lass_load), 如 class_yank,
                  除了防止找出的作者看板列表消失, 也防踢人  */
  if (xo->key >= 0)
    return XO_NONE;

  class_flag ^= BFO_YANK;
  return class_init(xo);
}


static int
class_zap(xo)
  XO *xo;
{
  BRD *brd;
#ifdef	DEBUG_ClassHeader_INT
  int *chp;
#else
  short *chp;
#endif
  int pos, num, chn;
  char token;

  pos = xo->pos;
#ifdef	DEBUG_ClassHeader_INT
  chp = (int *) xo->xyz + pos;
#else
  chp = (short *) xo->xyz + pos;
#endif
  chn = *chp;
  if (chn >= 0)		/* 一般看板 */
  {
    brd = bshm->bcache + chn;
    if (!(brd->battr & BRD_NOZAP))
    {
      /* itoc.010909: 要隨 class_item() 版面變 */
      move(3 + pos - xo->top, 6);
      num = brd_bits[chn] ^= BRD_Z_BIT;

      /* 處理 zap/friend/secret 板的符號 */
      if (num & BRD_Z_BIT)
	token = TOKEN_ZAP_BRD;
      else if (brd->readlevel == PERM_SYSOP)
	token = TOKEN_SECRET_BRD;
      else if (brd->readlevel == PERM_BOARD)
	token = TOKEN_FRIEND_BRD;
      else
	token = ' ';

      outc(token);
    }
  }
  else			/* 分類群組 */
  {
#ifdef	DEBUG_ClassHeader_INT
    int *chx;
#else
    short *chx;
#endif
    char *img, brdname[BNLEN + 1];

    /* itoc.010909: 要隨 class_body() 版面變 */
    move(3 + pos - xo->top, 6);
    num = class_bits[-chn] ^= BRD_Z_BIT;
    outc(num & BRD_Z_BIT ? TOKEN_ZAP_BRD : ' ');

    img = class_img;
#ifdef	DEBUG_ClassHeader_INT
    chx = (int *) img + (CH_END - chn);
#else
    chx = (short *) img + (CH_END - chn);
#endif
    str_ncpy(brdname, img + *chx, BNLEN + 1);
    czh_put(brdname);
  }

  /* return XO_NONE; */
  return XO_MOVE + pos + 1;	/* itoc.020219: 跳至下一項 */
}


static int
class_zapall(xo)
  XO *xo;
{
  BRD *brdp, *bend;
  int ans, bno;

  ans = vans("設定所有看板 (U)訂閱\ (Z)不訂閱\ (Q)取消？ [Q] ");
  if (ans != 'z' && ans != 'u')
    return XO_FOOT;

  brdp = bshm->bcache;
  bend = brdp + bshm->number;
  bno = 0;
  do
  {
    if (ans == 'z')
    {
      if (!(brdp->battr & BRD_NOZAP))
	brd_bits[bno] |= BRD_Z_BIT;
    }
    else
    {
      brd_bits[bno] &= ~BRD_Z_BIT;
    }

    bno++;
  } while (++brdp < bend);

  class_flag |= BFO_YANK;	/* 強迫 yank 起來看結果 */
  return class_init(xo);
}


static int
class_visit(xo)		/* itoc.010128: 看板列表設定看板已讀 */
  XO *xo;
{
#ifdef	DEBUG_ClassHeader_INT
  int *chp;
#else
  short *chp;
#endif
  int chn;

#ifdef	DEBUG_ClassHeader_INT
  chp = (int *) xo->xyz + xo->pos;
#else
  chp = (short *) xo->xyz + xo->pos;
#endif
  chn = *chp;
  if (chn >= 0)
  {
    BRD *brd;
    brd = bshm->bcache + chn;
    brh_get(brd->bstamp, chn);
    brh_visit(0);
#ifndef ENHANCED_VISIT
    time4(&brd_visit[chn]);
#endif
  }
  return class_body(xo);
}


static int
class_unvisit(xo)		/* itoc.010129: 看板列表設定看板未讀 */
  XO *xo;
{
#ifdef	DEBUG_ClassHeader_INT
  int *chp;
#else
  short *chp;
#endif
  int chn;

#ifdef	DEBUG_ClassHeader_INT
  chp = (int *) xo->xyz + xo->pos;
#else
  chp = (short *) xo->xyz + xo->pos;
#endif
  chn = *chp;
  if (chn >= 0)
  {
    BRD *brd;
    brd = bshm->bcache + chn;
    brh_get(brd->bstamp, chn);
    brh_visit(1);
#ifndef ENHANCED_VISIT
    brd_visit[chn] = 0;	/* itoc.010402: 最近瀏覽時間歸零，使看板列表中顯示未讀 */
#endif
  }
  return class_body(xo);
}


static int
class_nextunread(xo)
  XO *xo;
{
  int max, pos, chn;
#ifdef	DEBUG_ClassHeader_INT
  int *chp;
#else
  short *chp;
#endif
  BRD *bcache, *brd;

  bcache = bshm->bcache;
  max = xo->max;
  pos = xo->pos;
#ifdef	DEBUG_ClassHeader_INT
  chp = (int *) xo->xyz + pos;
#else
  chp = (short *) xo->xyz + pos;
#endif

  while (++pos < max)
  {
    chn = *(++chp);
    if (chn >= 0 && !(brd_bits[chn] & BRD_Z_BIT) && (brd_bits[chn] & BRD_R_BIT))
    {	/* 跳過分類及 zap 掉、進不去的看板 */
      brd = bcache + chn;

#ifdef ENHANCED_VISIT
      /* itoc.010407: 改用最後一篇已讀/未讀來判斷 */
      brh_get(brd->bstamp, chn);
      if (brh_unread(brd->blast))
#else
      if (brd->blast > brd_visit[chn])
#endif
	return pos + XO_MOVE;
    }
  }

  return XO_NONE;
}


static int
class_edit(xo)
  XO *xo;
{
  if (HAS_PERM(PERM_ALLBOARD | PERM_BM))
  {
#ifdef	DEBUG_ClassHeader_INT
    int *chp;
#else
    short *chp;
#endif
    int chn;

#ifdef	DEBUG_ClassHeader_INT
    chp = (int *) xo->xyz + xo->pos;
#else
    chp = (short *) xo->xyz + xo->pos;
#endif
    chn = *chp;
    if (chn >= 0)
    {
      if (!HAS_PERM(PERM_ALLBOARD))
      {
	if (!brd_title(chn))		/* itoc.000312: 板主修改中文敘述 */
	  return XO_NONE;
      }
      else
	brd_edit(chn);
      return class_init(xo);
    }
  }
  return XO_NONE;
}


static int
class_info(xo)
  XO *xo;
{
  BRD *bhdr;
#ifdef	DEBUG_ClassHeader_INT
  int *chp;
#else
  short *chp;
#endif
  int chn;

#ifdef	DEBUG_ClassHeader_INT
  chp = (int *) xo->xyz + xo->pos;
#else
  chp = (short *) xo->xyz + xo->pos;
#endif
  chn = *chp;

  if (chn >= 0)
  {
    bhdr = bshm->bcache + chn;

    move(0, 0);
    clrtoeol();
    prints("   - 英文板名: %-13s                   - 看板分類: %s\n",
      bhdr->brdname, bhdr->class);
    prints("   - 中文板名: %s\n", bhdr->title);
    if (*bhdr->BM > ' ')
      prints("   - 板主名單: %s", bhdr->BM);
    else
      prints("   - 本板應徵板主中");

    clrtoeol();
    move(xo->pos - xo->top + 3, 0);
    vkey();
    return class_head(xo);
  }
  return XO_NONE;
}


static int
hdr_cmp(a, b)
  HDR *a;
  HDR *b;
{
  /* 先比對分類，再比對板名 */
  int k = strncmp(a->title + BNLEN + 1, b->title + BNLEN + 1, BCLEN);
  return k ? k : str_cmp(a->xname, b->xname);
}


static void
add_class(brd, class_name)
  BRD *brd;
  char *class_name;
{
  HDR hdr;
  char fpath[64];

  sprintf(fpath, "gem/@/@%s", class_name);

  /* 加入適當的分類 */

  brd2gem(brd, &hdr);
  rec_add(fpath, &hdr, sizeof(HDR));
  rec_sync(fpath, sizeof(HDR), hdr_cmp, NULL);
}


static int
class_newbrd(xo)
  XO *xo;
{
  BRD newboard;

  if (!HAS_PERM(PERM_ALLBOARD))
    return XO_NONE;

  if (!adm_check())
    return;

  memset(&newboard, 0, sizeof(BRD));

  /* itoc.010211: 新看板預設 postlevel = PERM_POST; battr = 不轉信 */
  newboard.postlevel = PERM_POST;
  newboard.battr = BRD_NOTRAN;

  if (brd_new(&newboard) < 0)
    return class_head(xo);

  if (xo->key < CH_END)		/* 在分類群組裡面 */
  {
#ifdef	DEBUG_ClassHeader_INT
    int *chx;
#else
    short *chx;
#endif
    char *img, *str;
    char xname[BNLEN + 1];

    img = class_img;
#ifdef	DEBUG_ClassHeader_INT
    chx = (int *) img + (CH_END - xo->key);
#else
    chx = (short *) img + (CH_END - xo->key);
#endif
    str = img + *chx;

    str_ncpy(xname, str, sizeof(xname));
    if (str = strchr(xname, '/'))
      *str = '\0';

    /* 加入分類群組 */
    add_class(&newboard, xname);

    vmsg("新板成立");
  }
  else				/* 在看板列表裡面 */
  {
    vmsg("新板成立，記著加入分類群組");
  }

  add_class(&newboard, "NewBoard");	/* smiler.080516: 新開看板加入NewBoard群組內 */

  return class_init(xo);
}


static int
class_browse(xo)
  XO *xo;
{
#ifdef	DEBUG_ClassHeader_INT
  int *chp;
#else
  short *chp;
#endif
  int chn;

#ifdef	DEBUG_ClassHeader_INT
  chp = (int *) xo->xyz + xo->pos;
#else
  chp = (short *) xo->xyz + xo->pos;
#endif
  chn = *chp;
  if (chn < 0)		/* 進入分類 */
  {
#if 1
    /* smiler.070602: for熱門看板 */
#ifdef	DEBUG_ClassHeader_INT
    int *chx;
#else
    short *chx;
#endif
    char *img, *str;
    img = class_img;
#ifdef	DEBUG_ClassHeader_INT
    chx = (int *) img + (CH_END - chn);
#else
    chx = (short *) img + (CH_END - chn);
#endif
    str = img + *chx;

    if (!strncmp(str, "HOT/", 4))	/* "HOT/" 名稱可自定，若改名也要順便改後面的長度 4 */
    {
      class_hot = 1;
      chn = CH_END;
    }
#endif

    if (!strncmp(str, "IM_CREATE/", 10))
      more("brd/IAS_Announce/note", NULL);

    if (!XoClass(chn))
      return XO_NONE;

    if (class_hot)
      class_hot = 0;	/* 離開 HOT Class 再清除 class_hot 標記 */
  }
  else			/* 進入看板 */
  {
    if (XoPost(chn))	/* 無法閱讀該板 */
      return XO_FOOT;
    xover(XZ_POST);
#ifndef ENHANCED_VISIT
    time4(&brd_visit[chn]);
#endif
  }

#ifdef AUTO_JUMPBRD
  if (cuser.ufo & UFO_JUMPBRD)
    class_jumpnext = 1;		/* itoc.010910: 只有在離開看板回到看板列表時才需要跳去下一個未讀看板 */
#endif

  return class_head(xo);	/* Thor.980701: 無法清少一點, 因為 XoPost */
}


int
Select()
{
  int bno;
  BRD *brd;
  char bname[BNLEN + 1];

  if (brd = ask_board(bname, BRD_R_BIT, NULL))
  {
    bno = brd - bshm->bcache;
    XoPost(bno);
    xover(XZ_POST);
#ifndef ENHANCED_VISIT
    time4(&brd_visit[bno]);
#endif
  }
  else
  {
    vmsg(err_bid);
  }

  return 0;
}


static int
class_switch(xo)
  XO *xo;
{
  Select();
  return class_head(xo);
}


#ifdef MY_FAVORITE
/* ----------------------------------------------------- */
/* MyFavorite [我的最愛]				 */
/* ----------------------------------------------------- */


inline int
in_favor(brdname)
  char *brdname;
{
  MF mf;
  int fd;
  int rc = 0;
  char fpath[64];

  if (brdname[0])
  {
    mf_fpath(fpath, cuser.userid, FN_MF);

    if ((fd = open(fpath, O_RDONLY)) >= 0)
    {
      while (read(fd, &mf, sizeof(MF)) == sizeof(MF))
      {
	if (!strcmp(brdname, mf.xname))
	{
	  rc = 1;
	  break;
	}
      }
    }
    close(fd);
  }
  return rc;
}


static int
class_addMF(xo)
  XO *xo;
{
#ifdef	DEBUG_ClassHeader_INT
  int *chp;
#else
  short *chp;
#endif
  int chn;
  MF mf;
  char fpath[64];

  if (!cuser.userlevel)
    return XO_NONE;

#ifdef	DEBUG_ClassHeader_INT
  chp = (int *) xo->xyz + xo->pos;
#else
  chp = (short *) xo->xyz + xo->pos;
#endif
  chn = *chp;

  if (chn >= 0)		/* 一般看板 */
  {
    BRD *bhdr;

    bhdr = bshm->bcache + chn;

    if (!in_favor(bhdr->brdname))
    {
      memset(&mf, 0, sizeof(MF));
      time4(&mf.chrono);
      mf.mftype = MF_BOARD;
      strcpy(mf.xname, bhdr->brdname);

      mf_fpath(fpath, cuser.userid, FN_MF);
      rec_add(fpath, &mf, sizeof(MF));
      vmsg("已將此看板加入我的最愛");
    }
    else
    {
      vmsg("此看板已在最愛中。若要重覆加入，請進我的最愛裡新增");
    }
  }
  else			/* 分類群組 */
  {
#ifdef	DEBUG_ClassHeader_INT
    int *chx;
#else
    short *chx;
#endif
    char *img, *str, *ptr;

    img = class_img;
#ifdef	DEBUG_ClassHeader_INT
    chx = (int *) img + (CH_END - chn);
#else
    chx = (short *) img + (CH_END - chn);
#endif
    str = img + *chx;

    memset(&mf, 0, sizeof(MF));
    time4(&mf.chrono);
    mf.mftype = MF_CLASS;
    ptr = strchr(str, '/');
    strncpy(mf.xname, str, ptr - str);
    strncpy(mf.class, str + BNLEN + 1, BCLEN);
    strcpy(mf.title, str + BNLEN + 1 + BCLEN + 1);

    if (!strcmp(mf.xname,"HOT"))
    {
      vmsg("熱門分類看板不可加入我的最愛");  /* smiler.071111 */
      return XO_NONE;
    }

    mf_fpath(fpath, cuser.userid, FN_MF);
    rec_add(fpath, &mf, sizeof(MF));
    vmsg("已將此分類加入我的最愛");
  }

  return XO_FOOT;
}


int
MFclass_browse(name)
  char *name;
{
  int chn, min_chn, len;
#ifdef	DEBUG_ClassHeader_INT
  int *chx;
#else
  short *chx;
#endif
  char *img, cname[BNLEN + 2];

  min_chn = bshm->min_chn;
  img = class_img;

  sprintf(cname, "%s/", name);
  len = strlen(cname);

  for (chn = CH_END - 2; chn >= min_chn; chn--)
  {
#ifdef	DEBUG_ClassHeader_INT
    chx = (int *) img + (CH_END - chn);
#else
    chx = (short *) img + (CH_END - chn);
#endif
    if (!strncmp(img + *chx, cname, len))
    {
      if (XoClass(chn))
	return 1;
      break;
    }
  }
  return 0;
}
#endif	/* MY_FAVORITE */


#ifdef AUTHOR_EXTRACTION
/* Thor.980818: 想改成以目前的看板列表或分類來找, 不要找全部 */


/* opus.1127 : 計畫重寫, 可 extract author/title */


static int
XoAuthor(xo)
  XO *xo;
{
  int chn, len, max, tag;
  long value;
#ifdef	DEBUG_ClassHeader_INT
  int *chp, *chead, *ctail;
#else
  short *chp, *chead, *ctail;
#endif
  BRD *brd;
  char key[30], author[IDLEN + 1];
  XO xo_a, *xoTmp;
  struct timeval tv = {0, 10};

  vget(b_lines, 0, MSG_XYPOST1, key, 30, DOECHO);
  vget(b_lines, 0, MSG_XYPOST2, author, IDLEN + 1, DOECHO);

  if (!*key && !*author)
    return XO_FOOT;

  str_lowest(key, key);
  str_lower(author, author);
  len = strlen(author);

#ifdef	DEBUG_ClassHeader_INT
  chead = (int *) xo->xyz;
#else
  chead = (short *) xo->xyz;
#endif
  max = xo->max;
  ctail = chead + max;

  tag = 0;
#ifdef	DEBUG_ClassHeader_INT
  chp = (int *) malloc(max * sizeof(int));
#else
  chp = (short *) malloc(max * sizeof(short));
#endif
  brd = bshm->bcache;

  do
  {
    if ((chn = *chead++) >= 0)	/* Thor.980818: 不為 group */
    {
      /* Thor.980701: 尋找指定作者文章, 有則移位置, 並放入 */

      int fsize;
      char *fimage;

      char folder[80];
      HDR *head, *tail;

      sprintf(folder, "《尋找指定標題作者》看板：%s \033[5m...\033[m按任意鍵中斷", brd[chn].brdname);
      outz(folder);
      refresh();

      brd_fpath(folder, brd[chn].brdname, fn_dir);
      fimage = f_map(folder, &fsize);

      if (fimage == (char *) -1)
	continue;

      head = (HDR *) fimage;
      tail = (HDR *) (fimage + fsize);

      while (head <= --tail)
      {
	if ((!*key || str_sub(tail->title, key)) &&
	  (!len || !str_ncmp(tail->owner, author, len)))
	{
	  xo_get(folder)->pos = tail - head;
	  chp[tag++] = chn;
	  break;
	}
      }

      munmap(fimage, fsize);
    }

    /* 使用者可以中斷搜尋 */
    value = 1;
    if (select(1, (fd_set *) &value, NULL, NULL, &tv) > 0)
    {
      vkey();
      break;
    }
  } while (chead < ctail);

  if (!tag)
  {
    free(chp);
    vmsg("空無一物");
    return XO_FOOT;
  }

  xo_a.pos = xo_a.top = 0;
  xo_a.max = tag;
  xo_a.key = 1;			/* all boards */
  /* Thor.990621: 所有的class,board列表下, key < 0, 以 1 與正常模式區分
                  使其不能跑 XO_INIT(�堶悸構lass_load), 如 class_yank,
                  除了防止找出的作者看板列表消失, 也防踢人 */
  xo_a.xyz = (char *) chp;

  xoTmp = xz[XZ_CLASS - XO_ZONE].xo;	/* Thor.980701: 記下原來的class_xo */
  xz[XZ_CLASS - XO_ZONE].xo = &xo_a;

#ifdef AUTO_JUMPBRD
  if (cuser.ufo & UFO_JUMPBRD)
    class_jumpnext = 1;	/* itoc.010910: 主動跳去下一個未讀看板 */
#endif
  xover(XZ_CLASS);

  free(chp);
  xz[XZ_CLASS - XO_ZONE].xo = xoTmp;	/* Thor.980701: 還原 class_xo */

  return class_body(xo);
}
#endif


#ifndef NEW_KeyFunc
static KeyFunc class_cb[] =
{
#ifdef  HAVE_LIGHTBAR
  XO_ITEM, class_bar,
#endif
  XO_INIT, class_head,
  XO_LOAD, class_body,
  XO_HEAD, class_head,
  XO_BODY, class_body,

  'r', class_browse,
  '/', class_search,
  '?', class_searchBM,
  's', class_switch,
  'c', class_postmode,
  'S', class_namemode,

  'y', class_yank,
  'z', class_zap,
  'Z', class_zapall,
  'v', class_visit,
  'V', class_unvisit,
  '`', class_nextunread,
  'E', class_edit,
  'i', class_info,

#ifdef AUTHOR_EXTRACTION
  'A', XoAuthor,
#endif

#ifdef MY_FAVORITE
  'a', class_addMF,
  'f', class_addMF,
#endif

  Ctrl('P'), class_newbrd,

  'h', class_help
};
#else
NewKeyFunc class_cb[] =
{
#ifdef  HAVE_LIGHTBAR
  XO_ITEM, class_bar,   XO_ITEM,        'n',    "XO_ITEM",      "",
#endif
  XO_INIT, class_head,  XO_INIT,        'n',    "XO_INIT",      "",
  XO_LOAD, class_body,  XO_LOAD,        'n',    "XO_LOAD",      "",
  XO_HEAD, class_head,  XO_HEAD,        'n',    "XO_HEAD",      "",
  XO_BODY, class_body,  XO_BODY,        'n',    "XO_BODY",      "",

  'r', class_browse,    'r',    'z',    "瀏覽",                 "",
  '/', class_search,    '/',    'p',    "依板名搜尋看板",       "",
  '?', class_searchBM,  '?',    'p',    "依板主搜尋看板",       "",
  's', class_switch,    's',    'p',    "選取看板",             "",
  'c', class_postmode,  'c',    'p',    "切換看板編號/篇數",    "",
  'S', class_namemode,  'S',    'p',    "按照字母/分類排序",    "",

  'y', class_yank,      'y',    'p',    "列出/不列出所有看板",  "",
  'z', class_zap,       'z',    'p',    "訂\閱\/不訂\閱\此看板",    "",
  'Z', class_zapall,    'Z',    'p',    "訂\閱\不訂\閱\全部看板",  "",
  'v', class_visit,     'v',    'p',    "設定看板已讀",         "",
  'V', class_unvisit,   'V',    'p',    "設定看板未讀",         "",
  '`', class_nextunread,'`',    'p',    "跳至下一未讀看板",     "",
  'E', class_edit,      'E',    'b',    "修改看板",             "",
  'i', class_info,      'i',    'p',    "顯示看板資訊",         "",

#ifdef AUTHOR_EXTRACTION
  'A', XoAuthor,        'A',    'p',    "搜尋全站作者/標題",    "",
#endif

#ifdef MY_FAVORITE
  'a', class_addMF,     'a',    'p',    "加入我的最愛",         "",
  'f', class_addMF,     'f',    'p',    "加入我的最愛",         "",
#endif

  Ctrl('P'), class_newbrd,      Ctrl('P'),    's',    "開闢看板",     "",

  'h', class_help,      'h',    'z',    "功\能說明",     ""
};
#endif


int
Class()
{
  /* XoClass(CH_END - 1); */
  /* Thor.980804: 防止 未用 account 造出 class.img 或沒有 class 的情況 */
  if (!class_img || !XoClass(CH_END - 1))
  {
    vmsg("未定義分組討論區");
    return XEASY;
  }
  return 0;
}


#ifdef MEICHU_WIN
int
Class2()
{
  int chn, min_chn;
#ifdef DEBUG_ClassHeader_INT
  int *chx;
#else
  short *chx;
#endif
  char *img, *str;
  const char *name = "NthuMeichu/";

  class_flag |= BFO_YANK;

  min_chn = bshm->min_chn;
  img = class_img;

  for (chn = CH_END - 2; chn >= min_chn; chn--)
  {
#ifdef DEBUG_ClassHeader_INT
    chx = (int *) img + (CH_END - chn);
#else
    chx = (short *) img + (CH_END - chn);
#endif
    if (!strncmp(img + *chx, name, strlen(name)))
    {
      more("brd/MeichuWin/note", NULL);
      if (XoClass(chn))
	return 0;
    }
  }
  vmsg("無此分類群組");
  return XEASY;
}
#endif


void
board_main()
{
  int fsize;

  brh_load();

  if (class_img)	/* itoc.030416: 第二次進入 board_main 時，要 free 掉 class_img */
  {
    free(class_img);
  }
  else			/* itoc.040102: 第一次進入 board_main 時，才需要初始化 class_flag */
  {
    class_flag = cuser.ufo & UFO_BRDPOST;	/* 看板列表 1:文章數 0:編號 */
    if (!cuser.userlevel)			/* guest yank all boards */
      class_flag |= BFO_YANK;

    /* 設定 default board */
    strcpy(currboard, BN_NULL);
    currbno = -1;
  }

  /* class_img = f_img(CLASS_IMGFILE, &fsize); */
  /* itoc.010413: 依照 ufo 來載入不同的 class image */
  class_img = f_img(cuser.ufo & UFO_BRDNAME ? CLASS_IMGFILE_NAME : CLASS_IMGFILE_TITLE, &fsize);

  if (class_img == NULL)
    blog("CACHE", "class.img");
  else
    czh_load();

  board_xo.key = CH_END;
  class_load(&board_xo);

  xz[XZ_CLASS - XO_ZONE].xo = &board_xo;	/* Thor: default class_xo */
  xz[XZ_CLASS - XO_ZONE].cb = class_cb;		/* Thor: default class_xo */
}


int
Boards()
{
  /* class_xo = &board_xo; *//* Thor: 已有 default, 不需作此 */

  class_hot = 0;
#ifdef AUTO_JUMPBRD
  if (cuser.ufo & UFO_JUMPBRD)
    class_jumpnext = 1;	/* itoc.010910: 主動跳去下一個未讀看板 */
#endif
  xover(XZ_CLASS);

  return 0;
}
