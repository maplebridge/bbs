/*-------------------------------------------------------*/
/* board.c	( NTHU CS MapleBBS Ver 2.36 )		 */
/*-------------------------------------------------------*/
/* target : �ݪO�B�s�ե\��	 			 */
/* create : 95/03/29				 	 */
/* update : 95/12/15				 	 */
/*-------------------------------------------------------*/


#include "bbs.h"


extern BCACHE *bshm;
extern XZ xz[];
extern char xo_pool[];


char brd_bits[MAXBOARD];

#ifndef ENHANCED_VISIT
time4_t brd_visit[MAXBOARD];		/* �̪��s���ɶ� */
#endif


static char *class_img = NULL;
static XO board_xo;

inline int in_favor(char *brdname);	/* smiler.070724 */


#if 1
//********/* smiler.070602: for xsort �_�l�B */********/

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

//*******/* smiler.070602: for xsort �����B */********/
#endif


/* ----------------------------------------------------- */
/* �ݪO�\Ū�O�� .BRH (Board Reading History)		 */
/* ----------------------------------------------------- */


typedef struct BoardReadingHistory
{
  time4_t bstamp;		/* �إ߬ݪO���ɶ�, unique */	/* Thor.brh_tail */
  time4_t bvisit;		/* �W���\Ū�ɶ� */		/* Thor.980904: �S�bŪ�ɩ�W��Ū���ɶ�, ���bŪ�ɩ� bhno */
  int bcount;			/* Thor.980902: �S�Ψ� */

  /* --------------------------------------------------- */
  /* time_t {final, begin} / {final | BRH_SIGN}		 */
  /* --------------------------------------------------- */
                           /* Thor.980904.����: BRH_SIGN�N��final begin �ۦP */
                           /* Thor.980904.����: �Ѥj��p�ƦC,�s��wŪinterval */
}                   BRH;


#define	BRH_EXPIRE	180		/* Thor.980902.����: �O�d�h�֤� */
#define BRH_MAX		200		/* Thor.980902.����: �C�O�̦h���X�Ӽ��� */
#define BRH_PAGE	2048		/* Thor.980902.����: �C���h�t�q, �Τ���F */
#define	BRH_MASK	0x7fffffff	/* Thor.980902.����: �̤j�q��2038�~1�뤤*/
#define	BRH_SIGN	0x80000000	/* Thor.980902.����: zap����final�M�� */
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
    size += n >> 4;		/* �h�w���@�ǰO���� */
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

    n = *++list;   /* Thor.980904: ��Ū�ɬObhno */
    brd_bits[n] |= BRD_H_BIT;
    time4((time4_t *) list);    /* Thor.980904.����: bvisit time */

    item = *++list;
    head = ++list;
    tail = head + item;

    while (head < tail)
    {
      chrono = *head++;
      n = *head++;
      if (n == chrono) /* Thor.980904.����: �ۦP���ɭ����_�� */
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
    brh_tail = list;  /* Thor.980904: �s����brh */
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

  if (bstamp == *brh_tail) /* Thor.980904.����: �ӪO�w�b brh_tail�W */
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

  /* itoc.010407.����: BRH_EXPIRE (180) �ѫe���峹���]���wŪ */
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
{				/* itoc.010207: �άO�ǤJchrono, �N��Ū�ܭ� */
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
  *++list = 0;	/* itoc.010207: �j�w�� 0, for ���� visit */
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


int			/* >=1:�ĴX�ӪO�D 0:���O�O�D */
is_bm(list, userid)
  char *list;		/* �O�D�GBM list */
  char *userid;
{
  return str_has(list, userid, strlen(userid));
}


#ifdef DO_POST_FILTER
int
IS_BIGGER_AGE(age)
  int age;
{
  time_t now;
  struct tm *ptime;

  if (age && !cuser.year)	/* �ͤ饼�� */
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
  /* �@�Ӥ�H 30 �ѭp�� */
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

    /* �|����o�̴N���ܹL���F */
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
  int ftype;	/* 0:�@��ID 1:�O�n 2:�O�a */

  /* songsongboy.060327: �ݪO�\Ū���Ż�����

  �z�w�w�w�w�s�w�w�w�w�s�w�w�w�w�s�w�w�w�w�s�w�w�w�w�{
  �x        �x�@��Τ�x�ݪO�n�͢x�n���a�H�x�ݪO�a�H�x
  �u�w�w�w�w�q�w�w�w�w�q�w�w�w�w�q�w�w�w�w�q�w�w�w�w�t
  �x�@��ݪO�x�v���M�w�x  ����  �x ��  �� �x �ݤ��� �x
  �u�w�w�w�w�q�w�w�w�w�q�w�w�w�w�q�w�w�w�w�q�w�w�w�w�t
  �x�n�ͬݪO�x �i���h �x  ����  �x ��  �� �x �ݤ��� �x
  �u�w�w�w�w�q�w�w�w�w�q�w�w�w�w�q�w�w�w�w�q�w�w�w�w�t
  �x���K�ݪO�x �ݤ��� �x  ����  �x ��  �� �x �ݤ��� �x
  �|�w�w�w�w�r�w�w�w�w�r�w�w�w�w�r�w�w�w�w�r�w�w�w�w�}
  �ݤ����G�b�ݪO�C�����L�k�ݨ�o�ӪO�A�]�i���h
  �i���h�G�b�ݪO�C�����i�H�ݨ�o�ӪO�A���O�i���h
  ��  ���G�b�ݪO�C�����i�H�ݨ�o�ӪO�A�]�i�o�h�A���O����o��
  ��  ��G�b�ݪO�C�����i�H�ݨ�o�ӪO�A�]�i�o�h�εo��

  */

  static int bit_data[12] =
  {                /* �@��Τ�   �ݪO�n��                            �S���n��(����)           �ݪO�a�H */
    /* ���}�ݪO */    0,         BRD_L_BIT | BRD_R_BIT | BRD_W_BIT, BRD_L_BIT | BRD_R_BIT, 0,
    /* �n�ͬݪO */    BRD_L_BIT, BRD_L_BIT | BRD_R_BIT | BRD_W_BIT, BRD_L_BIT | BRD_R_BIT, 0,
    /* ���K�ݪO */    0,         BRD_L_BIT | BRD_R_BIT | BRD_W_BIT, BRD_L_BIT | BRD_R_BIT, 0
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

  if (readlevel == PERM_SYSOP)		/* ���K�ݪO */
    bits = bit_data[8 + ftype];
  else if (readlevel == PERM_BOARD)	/* �n�ͬݪO */
    bits = bit_data[4 + ftype];
  else if (ftype)			/* ���}�ݪO�A�Y�b�O�n/�O�a�W�椤 */
    bits = bit_data[ftype];
  else					/* ���}�ݪO�A��L���v���P�w */
#endif

  if (!readlevel || (readlevel & ulevel))
  {
    bits = BRD_L_BIT | BRD_R_BIT;

    postlevel = brd->postlevel;
    if (!postlevel || (postlevel & ulevel))	/* �������u�� sysop �O�S�� postlevel */
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
  else					/* ���]�ݪOŪ���v�����O�A guest �N�L�k�ݨ� */
  {
    bits = 0;
  }

  /* Thor.980813.����: �S�O�� BM �Ҷq�A�O�D���ӪO���Ҧ��v�� */
  blist = brd->BM;
  if ((ulevel & PERM_BM) && blist[0] > ' ' && is_bm(blist, cuser.userid))
    bits = BRD_L_BIT | BRD_R_BIT | BRD_W_BIT | BRD_X_BIT | BRD_M_BIT;

  /* smiler.080811: ATOM ������ ATOM �ݪO��ƥ����ݩ� */
  else if ((ulevel & PERM_ATOM) && (brd->battr & BRD_ATOM))
    bits = BRD_L_BIT | BRD_R_BIT | BRD_W_BIT | BRD_X_BIT | BRD_M_BIT;

  else if (ulevel & PERM_ALLBOARD)
  {
    bits |= BRD_L_BIT;
    if (!(readlevel & (PERM_SYSOP | PERM_BOARD)))	/* ����/�ݪO�`�ީ󤽶}�O���޲z�v�� */
      bits |= BRD_R_BIT | BRD_W_BIT | BRD_X_BIT;
  }

  return bits;
}


/* ----------------------------------------------------- */
/* ���J currboard �i��Y�z�]�w				 */
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
  /* �N .BRH ���J memory				 */
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
  /* �h�O�d BRH_WINDOW ���B�@�Ŷ�			 */
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
	  /* itoc.001029: NOZAP��, ���|�X�{ */
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

      /* �o�ӬݪO�s�b�B�S���Q zap ���B�i�H list */

      if (bhno >= 0 && (bits[bhno] & BRD_L_BIT))
      {
	bits[bhno] |= BRD_H_BIT;/* �w���\Ū�O�� */

#ifndef ENHANCED_VISIT
	bstp[bhno] = head[1];	/* �W���\Ū�ɶ� */
#endif

	cbno = bhno;

	if (n > 0)
	{
	  list += n;	/* Thor.980904.����: �̫�@�� tag */

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

  /* Thor.980830: lkchu patch:  �٨S load �N���� save */
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
/* ���� zap �O�� .CZH (Class Zap History)		 */
/* ----------------------------------------------------- */


typedef struct ClassZapHistory
{
  char brdname[BNLEN + 1];	/* ������ brdname */
}                   CZH;


static char class_bits[CH_MAX + 3];


static int			/* >=0:pos  -1:���b .CZH */
czh_find(fpath, brdname)	/* �ˬd�O�_�w�g�b .CZH �� */
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


static int		/* >=0:�b .CZH  -1:���b .CZH */
czh_put(brdname)
  char *brdname;
{
  char fpath[64];
  int pos;

  usr_fpath(fpath, cuser.userid, FN_CZH);

  /* �Y�w�g�b .CZH �����ܡA�h�R��������ơF�Y���b .CZH �����ܡA�h�[�J������� */
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
      for (chn = CH_END - 2;; chn--)	/* �b�Ҧ�����������@�ӪO�W�ۦP�� */
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
	if (chn <= min_chn)	/* �p�G�䤣��A���ܸӤ����w�����A�q .CZH �R�� */
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

#ifdef LOG_BRD_USIES	/* lkchu.981201: �ݪO�\Ū�O�� */
static void
brd_usies()
{
  char buf[256];

  sprintf(buf, "%s %s (%s)\n", currboard, cuser.userid, Now());
  f_cat(FN_RUN_BRDUSIES, buf);
}
#endif


int
last_nobottom(folder)	/* ��ݪO�̫�@�g�D�m���媺��m */
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
  char BMstr[17];	/*by ryancid ����16*/
  char reason[64];

  brd = bshm->bcache + bno;
  if (!brd->brdname[0])	/* �w�R�����ݪO */
    return -1;

  bits = brd_bits[bno];

  if (currbno != bno)	/* �ݪO�S���q�`�O�]�� every_Z() �^��ݪO */
  {
#ifdef HAVE_UNANONYMOUS_BOARD
    /* 070403.songsongboy�G����ìݤϰΦW�� */
    if (!strcmp(brd->brdname, BN_UNANONYMOUS))
    {
      if (vans("�z�T�w�n�i�J�ϰΦW����(y/n)�H[N] ") != 'y')
	return -1;

      vmsg("�п�J�[�ݤϰΦW�����z��");
      sprintf(reason, "�z�ѡG");
      vget(b_lines, 0, "�п�J�]�w��ƪ��z�ѡG", reason + 6, 50, DOECHO);
      alog("�ϰΦW��", reason);
    }
#endif
#ifdef HAVE_MODERATED_BOARD
    if (!(bits & BRD_R_BIT))
    {
      if ((cuser.userlevel & PERM_SYSOP) && (brd->battr & BRD_PUBLIC))
      {
	vmsg("ĵ�i�G�ϥί��ȶi�J���ä����O�\\��");
	if (!adm_check())
	  return -1;
	sprintf(reason, "%-*s �O�G", BNLEN + 1, brd->brdname);
	vget(b_lines, 0, "�п�J�i�J���O���z�ѡG", reason + BNLEN + 6, 50, DOECHO);
	alog("�i�J���O", reason);
	bits |= BRD_R_BIT | BRD_X_BIT;
      }
      else
      {
	vmsg("�藍�_�A���O�u��O�Ͷi�J�A�ЦV�O�D�ӽФJ�ҳ\\�i");
	return -1;
      }
    }
#endif

    /* �B�z�v�� */
    bbstate = STAT_STARTED;
    if (bits & BRD_M_BIT)
      bbstate |= (STAT_BM | STAT_BOARD | STAT_POST);
    else if (bits & BRD_X_BIT)
      bbstate |= (STAT_BOARD | STAT_POST);
    else if (bits & BRD_W_BIT)
      bbstate |= STAT_POST;

#ifdef LOG_BRD_USIES
    /* lkchu.981201: �\Ū�ݪO�O�� */
    brd_usies();
#endif

    /* itoc.050613.����: �H�𪺴�֤��O�b���}�ݪO�ɡA�ӬO�b�i�J�s���ݪO�άO�����ɡA
       �o�O���F�קK switch ���ݪO�|����H�� */
    if (currbno >= 0)
      bshm->mantime[currbno]--;		/* �h�X�W�@�ӪO */
    bshm->mantime[bno]++;		/* �i�J�s���O */

    currbno = bno;
    currbattr = brd->battr;
    strcpy(currboard, brd->brdname);
    str = brd->BM;

    str_ncpy(BMstr, str, 17);
    if (strlen(str) > 16)
    {
      BMstr[14] = 0xA1;
      BMstr[15] = 0x4B;
    }	/* �L�X "�K" */

    sprintf(currBM, "�O�D�G%s", *str <= ' ' ? "�x�D��" : BMstr);
#ifdef HAVE_BRDMATE
    strcpy(cutmp->reading, currboard);
#endif

    brd_fpath(fpath, currboard, fn_dir);

#ifdef AUTO_JUMPPOST
    xz[XZ_POST - XO_ZONE].xo = xo = xo_get_post(fpath, brd);	/* itoc.010910: �� XoPost �q�����y�@�� xo_get() */
#else
    xz[XZ_POST - XO_ZONE].xo = xo = xo_get(fpath);
#endif
    xo->key = XZ_POST;
    xo->xyz = brd->title;
  }

  /* itoc.011113: �Ĥ@���i�O�@�w�n�ݶi�O�e���A�ĤG���H��h���M ufo �]�w */
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
brd_force()			/* itoc.010407: �j��\Ū���i�O�A�B�j��Ū�̫�@�g */
{
  if (cuser.userlevel)		/* guest ���L */
  {
    int bno;
    BRD *brd;

    if ((bno = brd_bno(BN_ANNOUNCE)) < 0)
      return;
    brd = bshm->bcache + bno;
    if (brd->btime < 0)		/* �|����s brd->blast �N���j��\Ū���i�O */
      return;

#ifdef ENHANCED_VISIT
    brh_get(brd->bstamp, bno);
    while (brh_unread(brd->blast))
#else
    if (brd->blast > brd_visit[bno])
#endif
    {
      vmsg("���s���i�I�Х��\\Ū���s���i��A���}");
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
/* Class [�����s��]					 */
/* ----------------------------------------------------- */


#ifdef MY_FAVORITE
int class_flag = 0;			/* favorite.c �n�� */
#else
static int class_flag = 0;
#endif


#ifdef AUTO_JUMPBRD
static int class_jumpnext = 0;	/* itoc.010910: �O�_���h�U�@�ӥ�Ū�O 1:�n 0:���n */
#endif


static int class_hot = 0;

static int
mantime_cmp(a, b)	/* smiler.070602: �ݪO�H��Ƨ� */
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
  int bnum = 0;			/* smiler.070602: for�����ݪO */
  BRD *brd;
  char *bits;

  /* lkchu.990106: ���� ���� account �y�X class.img �ΨS�� class �����p */
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
    if (chn >= 0)	/* �@��ݪO */
    {
      val = bits[chn];
      if (!(val & BRD_L_BIT) || (val & zap) || !(brd[chn].brdname[0]))
	continue;
      if (class_hot && bshm->mantime[chn] <= 2 && strcmp(brd[chn].brdname, "sysop"))
	continue;
    }
    else		/* �����s�� */
    {
      if (class_bits[-chn] & zap)
	continue;
    }

    max++;
    *cbase++ = chn;

    if (chn >= 0)	/* smiler.070602: for�����ݪO */
      bnum++;
  } while (chead < ctail);

  if (class_hot && bnum > 0)
  {
    cbase -= bnum;
#ifdef	DEBUG_ClassHeader_INT
    xsort(cbase, bnum, sizeof(int), mantime_cmp)
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

  /* Thor.980727: �ѨM XO xo�����T�w��,
                  class_load�����| initial xo.max, ��L���T�w */
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
    class_jumpnext = 1;	/* itoc.010910: �D�ʸ��h�U�@�ӥ�Ū�ݪO */
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
  /* itoc.020123: ���ަ��L UFO_BRDPOST�A�@����s�A�H�K��Ū�O���|�G */
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
	/* itoc.020829: ��̫�@�g���Q�[�K�B���O�m���� HDR */
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
class_item(num, bno, brdpost, infav, label)	/* smiler.070724: �ڪ��̷R�ݪO�W��*/
  int num, bno, brdpost, infav, label;
{
  BRD *brd;
  char *str1, *str2, *str3, token, buf[16];

  brd = bshm->bcache + bno;

  btime_refresh(brd);

  /* �B�z �s��/�g�� */
  if (brdpost)
    num = brd->bpost;

  /* �B�z zap/friend/secret �O���Ÿ� */
  if (brd_bits[bno] & BRD_Z_BIT)
    token = TOKEN_ZAP_BRD;
  else if (brd->readlevel == PERM_SYSOP)
    token = TOKEN_SECRET_BRD;
  else if (brd->readlevel == PERM_BOARD)
    token = TOKEN_FRIEND_BRD;
  else
    token = ' ';

  /* �B�z �wŪ/��Ū */
#ifdef ENHANCED_VISIT
  /* itoc.010407: ��γ̫�@�g�wŪ/��Ū�ӧP�_ */
  brh_get(brd->bstamp, bno);
  str1 = brh_unread(brd->blast) ? ICON_UNREAD_BRD : ICON_READ_BRD;
#else
  str1 = brd->blast > brd_visit[bno] ? ICON_UNREAD_BRD : ICON_READ_BRD;
#endif

  /* �B�z �벼/��H */
  if (brd->bvote)
    str2 = (brd->bvote > 0) ? ICON_VOTED_BRD : ICON_GAMBLED_BRD;
  else
    str2 = (brd->battr & BRD_NOTRAN) ? ICON_NOTRAN_BRD : ICON_TRAN_BRD;

  /* �B�z �H�� */
  brdpost = bno;	/* �O�� bno */
  bno = bshm->mantime[bno];
  if (bno > 730)
    str3 = "\033[1;36m�z�F\033[m";
  else if (bno > 400)
    str3 = "\033[1;35m�z�F\033[m";
  else if (bno > 180)
    str3 = "\033[1;34m�z�F\033[m";
  else if (bno > 90)
    str3 = "\033[1;32m�z�F\033[m";
  else if (bno > 50)
    str3 = "\033[1;31m����\033[m";
  else if (bno > 25)
    str3 = "\033[1;33m���l\033[m";
  else if (bno > 10)
    sprintf(str3 = buf, "\033[1;31m%4d\033[m", bno);
  else if (bno > 5)
    sprintf(str3 = buf, "\033[1;33m%4d\033[m", bno);
  else if (bno > 0)
    sprintf(str3 = buf, "%4d", bno);
  else
    str3 = "";

  /* smiler.070724: �ݪO�t�� */
  if (!infav)
  {
    prints("%6d%c%s", num, token, str1);

    /* smiler.080712: �B�z���O�i����� */
    if (((brd->readlevel == PERM_BOARD) || (brd->readlevel == PERM_SYSOP)) && !(brd_bits[brdpost] & BRD_R_BIT))
      prints("\033[m\033[1;30m%-13s\033[m", brd->brdname);
    else
      prints("%-13s", brd->brdname);
  }
  else	/* smiler.070724: �ڪ��̷R�ݪO�t�~�W��*/
    prints("%6d%c%s\033[1;36m%-13s\033[m", num, label ? 'T' : token, str1, brd->brdname);

  if (!strcmp(brd->class, "����") || !strcmp(brd->class, "�t��"))
    prints("\033[1;31m");
  else if (!strcmp(brd->class, "�a��") || !strcmp(brd->class, "��|"))
    prints("\033[1;32m");
  else if (!strcmp(brd->class, "�ӤH"))
    prints("\033[1;33m");
  else if (!strcmp(brd->class, "�M��") || !strcmp(brd->class, "��T") || !strcmp(brd->class, "�ն�"))
    prints("\033[1;34m");
  else if (!strcmp(brd->class, "Comp") || !strcmp(brd->class, "�q��"))
    prints("\033[1;36m");
  else if (!strcmp(brd->class, "����"))
    prints("\033[33m");
  else if (!strcmp(brd->class, "�ݦn"))
    prints("\033[35m");
  else
    prints("\033[1;3%dm", brd->class[3] & 7);

  prints("%-5s\033[m%s ", brd->class, str2);

  /* itoc.060530: �ɥ� str1�Bnum �ӳB�z�ݪO�ԭz��ܪ������_�r */
  str1 = brd->title;
  num = (d_cols >> 1) + 31;	/* smiler.070724: 33->31 for ����O�W��u */
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

  /* �B�z �s��/�g�� */
  num = brdpost ? brd->bpost : bno;

  /* �B�z zap/friend/secret �O���Ÿ� */
  if (brd_bits[chn] & BRD_Z_BIT)
    token = TOKEN_ZAP_BRD;
  else if (brd->readlevel == PERM_SYSOP)
    token = TOKEN_SECRET_BRD;
  else if (brd->readlevel == PERM_BOARD)
    token = TOKEN_FRIEND_BRD;
  else
    token = ' ';

  /* �B�z �wŪ/��Ū */
#ifdef ENHANCED_VISIT
  /* itoc.010407: ��γ̫�@�g�wŪ/��Ū�ӧP�_ */
  brh_get(brd->bstamp, chn);
  str1 = brh_unread(brd->blast) ? ICON_UNREAD_BRD : ICON_READ_BRD;
#else
  str1 = brd->blast > brd_visit[chn] ? ICON_UNREAD_BRD : ICON_READ_BRD;
#endif

  /* �B�z �벼/��H */
  if (brd->bvote)
    str2 = (brd->bvote > 0) ? ICON_VOTED_BRD : ICON_GAMBLED_BRD;
  else
    str2 = (brd->battr & BRD_NOTRAN) ? ICON_NOTRAN_BRD : ICON_TRAN_BRD;

  /* �B�z �H�� */
  bno = bshm->mantime[chn];
  if (bno > 730)
    str3 = "\033[1;36m�z�F\033[m";
  else if (bno > 400)
    str3 = "\033[1;35m�z�F\033[m";
  else if (bno > 180)
    str3 = "\033[1;34m�z�F\033[m";
  else if (bno > 90)
    str3 = "\033[1;32m�z�F\033[m";
  else if (bno > 50)
    str3 = "\033[1;31m����\033[m";
  else if (bno > 25)
    str3 = "\033[1;33m���l\033[m";
  else if (bno > 10)
    sprintf(str3 = buf, "\033[1;31m%4d\033[m", bno);
  else if (bno > 5)
    sprintf(str3 = buf, "\033[1;33m%4d\033[m", bno);
  else if (bno > 0)
    sprintf(str3 = buf, "%4d", bno);
  else
    str3 = "";

  /*smiler.070724: �ݪO�t�� */
  if (!infav)
  {
    prints("\033[m%s%6d%c%s%s", UCBAR[UCBAR_BRD], num, token, str1, UCBAR[UCBAR_BRD]);

    /* smiler.080712: �B�z���O�i����� */
    if (((brd->readlevel == PERM_BOARD) || (brd->readlevel == PERM_SYSOP)) && !(brd_bits[chn] & BRD_R_BIT))
      prints("\033[1;30m%-13s\033[m%s", brd->brdname, UCBAR[UCBAR_BRD]);
    else
      prints("%-13s", brd->brdname);
  }
  else	/* smiler.070724: �ڪ��̷R�ݪO�t�~�W��*/
    prints("\033[m%s%6d%c%s%s\033[1;36m%-13s\033[m%s",
      UCBAR[UCBAR_BRD], num, label ? 'T' : token, str1,
      UCBAR[UCBAR_BRD], brd->brdname, UCBAR[UCBAR_BRD]);

  if (!strcmp(brd->class, "����") || !strcmp(brd->class, "�t��"))
    prints("\033[1;31m");
  else if (!strcmp(brd->class, "�a��") || !strcmp(brd->class, "��|"))
    prints("\033[1;32m");
  else if (!strcmp(brd->class, "�ӤH"))
    prints("\033[1;33m");
  else if (!strcmp(brd->class, "�M��") || !strcmp(brd->class, "��T") || !strcmp(brd->class, "�ն�"))
    prints("\033[1;34m");
  else if (!strcmp(brd->class, "Comp") || !strcmp(brd->class, "�q��"))
    prints("\033[1;36m");
  else if (!strcmp(brd->class, "����"))
    prints("\033[33m");
  else if (!strcmp(brd->class, "�ݦn"))
    prints("\033[35m");
  else
    prints("\033[1;3%dm", brd->class[3] & 7);

  prints("%-5s\033[m%s%s%s ", brd->class, UCBAR[UCBAR_BRD], str2, UCBAR[UCBAR_BRD]);

  /* itoc.060530: �ɥ� str1�Bnum �ӳB�z�ݪO�ԭz��ܪ������_�r */
  str1 = brd->title;
  num = (d_cols >> 1) + 31; /* smiler.070724: 33->31 for ����O�W��u */
  prints("%-*.*s", num, IS_ZHC_LO(str1, num - 1) ? num - 2 : num - 1, str1);

  /* smiler.070724: �W�߳B�z�ݪO�H�� */
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

  if (chn >= 0)		/* �@��ݪO */
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

  /* itoc.010910: �j�M�U�@�ӥ�Ū�ݪO */
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
	/* itoc.010407: ��γ̫�@�g�wŪ/��Ū�ӧP�_ */
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

    /* �U�@�ӥ�Ū�O�b�O���A�n½�L�h */
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
      if (chn >= 0)		/* �@��ݪO */
      {
	 /*smiler.070724: �ڪ��̷R�ݪO�O�W�W�� */
	 BRD *bhdr;
	 bhdr = bshm->bcache + chn;
	 class_item(cnt, chn, brdpost, in_favor(bhdr->brdname), 0);
      }
      else			/* �����s�� */
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
  /* itoc.010910: �U�@�ӥ�Ū�O�b�����A�n���в��L�h */
  outf(FEETER_CLASS);
  return nextpos ? nextpos + XO_MOVE : XO_NONE;
#else
  /* return XO_NONE; */
  return XO_FOOT;	/* itoc.010403: �� b_lines ��W feeter */
#endif
}


static int
class_neck(xo)
  XO *xo;
{
  move(1, 0);
  prints(NECKER_CLASS,
    class_flag & UFO_BRDPOST ? "�`��" : "�s��",
    d_cols >> 1, "", d_cols - (d_cols >> 1), "");
  return class_body(xo);
}


static int
class_head(xo)
  XO *xo;
{
  vs_head("�ݪO�C��", str_site);
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
class_namemode(xo)		/* itoc.010413: �ݪO�̷Ӧr��/�����ƦC */
  XO *xo;
{
  static time_t last = 0;
  time_t now;

  if (time(&now) - last < 10)
  {
    vmsg("�C�Q�����u������@��");
    return XO_FOOT;
  }
  last = now;

  if (cuser.userlevel)
    brh_save();			/* itoc.010711: �x�s�\Ū�O���� */
  cuser.ufo ^= UFO_BRDNAME;
  cutmp->ufo = cuser.ufo;
  board_main();			/* ���s���J class_img */
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
	  outf(FEETER_CLASS);	/* itoc.010913: �� b_lines ��W feeter */
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

  if (vget(b_lines, 0, "�п�J�O�D�G", buf, IDLEN + 1, DOECHO))
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
	  outf(FEETER_CLASS);	/* itoc.010913: �� b_lines ��W feeter */
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
  /* itoc.001029: �Ҧ���class,board�C���U, key < 0, 1 �h����M�@�̼Ҧ�
                  �Ϩ䤣��] XO_INIT(�ح���class_load), �p class_yank,
                  ���F�����X���@�̬ݪO�C������, �]����H  */
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
  if (chn >= 0)		/* �@��ݪO */
  {
    brd = bshm->bcache + chn;
    if (!(brd->battr & BRD_NOZAP))
    {
      /* itoc.010909: �n�H class_item() ������ */
      move(3 + pos - xo->top, 6);
      num = brd_bits[chn] ^= BRD_Z_BIT;

      /* �B�z zap/friend/secret �O���Ÿ� */
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
  else			/* �����s�� */
  {
#ifdef	DEBUG_ClassHeader_INT
    int *chx;
#else
    short *chx;
#endif
    char *img, brdname[BNLEN + 1];

    /* itoc.010909: �n�H class_body() ������ */
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
  return XO_MOVE + pos + 1;	/* itoc.020219: ���ܤU�@�� */
}


static int
class_zapall(xo)
  XO *xo;
{
  BRD *brdp, *bend;
  int ans, bno;

  ans = vans("�]�w�Ҧ��ݪO (U)�q�\\ (Z)���q�\\ (Q)�����H [Q] ");
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

  class_flag |= BFO_YANK;	/* �j�� yank �_�Ӭݵ��G */
  return class_init(xo);
}


static int
class_visit(xo)		/* itoc.010128: �ݪO�C���]�w�ݪO�wŪ */
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
class_unvisit(xo)		/* itoc.010129: �ݪO�C���]�w�ݪO��Ū */
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
    brd_visit[chn] = 0;	/* itoc.010402: �̪��s���ɶ��k�s�A�ϬݪO�C������ܥ�Ū */
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
    {	/* ���L������ zap ���B�i���h���ݪO */
      brd = bcache + chn;

#ifdef ENHANCED_VISIT
      /* itoc.010407: ��γ̫�@�g�wŪ/��Ū�ӧP�_ */
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
	if (!brd_title(chn))		/* itoc.000312: �O�D�ק襤��ԭz */
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
    prints("   - �^��O�W: %-13s                   - �ݪO����: %s\n",
      bhdr->brdname, bhdr->class);
    prints("   - ����O�W: %s\n", bhdr->title);
    if (*bhdr->BM > ' ')
      prints("   - �O�D�W��: %s", bhdr->BM);
    else
      prints("   - ���O���x�O�D��");

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
  /* ���������A�A���O�W */
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

  /* �[�J�A�������� */

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

  /* itoc.010211: �s�ݪO�w�] postlevel = PERM_POST; battr = ����H */
  newboard.postlevel = PERM_POST;
  newboard.battr = BRD_NOTRAN;

  if (brd_new(&newboard) < 0)
    return class_head(xo);

  if (xo->key < CH_END)		/* �b�����s�ո̭� */
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

    /* �[�J�����s�� */
    add_class(&newboard, xname);

    vmsg("�s�O����");
  }
  else				/* �b�ݪO�C���̭� */
  {
    vmsg("�s�O���ߡA�O�ۥ[�J�����s��");
  }

  add_class(&newboard, "NewBoard");	/* smiler.080516: �s�}�ݪO�[�JNewBoard�s�դ� */

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
  if (chn < 0)		/* �i�J���� */
  {
#if 1
    /* smiler.070602: for�����ݪO */
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

    if (!strncmp(str, "HOT/", 4))	/* "HOT/" �W�٥i�۩w�A�Y��W�]�n���K��᭱������ 4 */
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
      class_hot = 0;	/* ���} HOT Class �A�M�� class_hot �аO */
  }
  else			/* �i�J�ݪO */
  {
    if (XoPost(chn))	/* �L�k�\Ū�ӪO */
      return XO_FOOT;
    xover(XZ_POST);
#ifndef ENHANCED_VISIT
    time4(&brd_visit[chn]);
#endif
  }

#ifdef AUTO_JUMPBRD
  if (cuser.ufo & UFO_JUMPBRD)
    class_jumpnext = 1;		/* itoc.010910: �u���b���}�ݪO�^��ݪO�C���ɤ~�ݭn���h�U�@�ӥ�Ū�ݪO */
#endif

  return class_head(xo);	/* Thor.980701: �L�k�M�֤@�I, �]�� XoPost */
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
/* MyFavorite [�ڪ��̷R]				 */
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

  if (chn >= 0)		/* �@��ݪO */
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
      vmsg("�w�N���ݪO�[�J�ڪ��̷R");
    }
    else
    {
      vmsg("���ݪO�w�b�̷R���C�Y�n���Х[�J�A�жi�ڪ��̷R�̷s�W");
    }
  }
  else			/* �����s�� */
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
      vmsg("���������ݪO���i�[�J�ڪ��̷R");  /* smiler.071111 */
      return XO_NONE;
    }

    mf_fpath(fpath, cuser.userid, FN_MF);
    rec_add(fpath, &mf, sizeof(MF));
    vmsg("�w�N�������[�J�ڪ��̷R");
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
/* Thor.980818: �Q�令�H�ثe���ݪO�C���Τ����ӧ�, ���n����� */


/* opus.1127 : �p�e���g, �i extract author/title */


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
    if ((chn = *chead++) >= 0)	/* Thor.980818: ���� group */
    {
      /* Thor.980701: �M����w�@�̤峹, ���h����m, �é�J */

      int fsize;
      char *fimage;

      char folder[80];
      HDR *head, *tail;

      sprintf(folder, "�m�M����w���D�@�̡n�ݪO�G%s \033[5m...\033[m�����N�䤤�_", brd[chn].brdname);
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

    /* �ϥΪ̥i�H���_�j�M */
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
    vmsg("�ŵL�@��");
    return XO_FOOT;
  }

  xo_a.pos = xo_a.top = 0;
  xo_a.max = tag;
  xo_a.key = 1;			/* all boards */
  /* Thor.990621: �Ҧ���class,board�C���U, key < 0, �H 1 �P���`�Ҧ��Ϥ�
                  �Ϩ䤣��] XO_INIT(�ح���class_load), �p class_yank,
                  ���F�����X���@�̬ݪO�C������, �]����H */
  xo_a.xyz = (char *) chp;

  xoTmp = xz[XZ_CLASS - XO_ZONE].xo;	/* Thor.980701: �O�U��Ӫ�class_xo */
  xz[XZ_CLASS - XO_ZONE].xo = &xo_a;

#ifdef AUTO_JUMPBRD
  if (cuser.ufo & UFO_JUMPBRD)
    class_jumpnext = 1;	/* itoc.010910: �D�ʸ��h�U�@�ӥ�Ū�ݪO */
#endif
  xover(XZ_CLASS);

  free(chp);
  xz[XZ_CLASS - XO_ZONE].xo = xoTmp;	/* Thor.980701: �٭� class_xo */

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

  'r', class_browse,    'r',    'z',    "�s��",                 "",
  '/', class_search,    '/',    'p',    "�̪O�W�j�M�ݪO",       "",
  '?', class_searchBM,  '?',    'p',    "�̪O�D�j�M�ݪO",       "",
  's', class_switch,    's',    'p',    "����ݪO",             "",
  'c', class_postmode,  'c',    'p',    "�����ݪO�s��/�g��",    "",
  'S', class_namemode,  'S',    'p',    "���Ӧr��/�����Ƨ�",    "",

  'y', class_yank,      'y',    'p',    "�C�X/���C�X�Ҧ��ݪO",  "",
  'z', class_zap,       'z',    'p',    "�q\�\\/���q\�\\���ݪO",    "",
  'Z', class_zapall,    'Z',    'p',    "�q\�\\���q\�\\�����ݪO",  "",
  'v', class_visit,     'v',    'p',    "�]�w�ݪO�wŪ",         "",
  'V', class_unvisit,   'V',    'p',    "�]�w�ݪO��Ū",         "",
  '`', class_nextunread,'`',    'p',    "���ܤU�@��Ū�ݪO",     "",
  'E', class_edit,      'E',    'b',    "�ק�ݪO",             "",
  'i', class_info,      'i',    'p',    "��ܬݪO��T",         "",

#ifdef AUTHOR_EXTRACTION
  'A', XoAuthor,        'A',    'p',    "�j�M�����@��/���D",    "",
#endif

#ifdef MY_FAVORITE
  'a', class_addMF,     'a',    'p',    "�[�J�ڪ��̷R",         "",
  'f', class_addMF,     'f',    'p',    "�[�J�ڪ��̷R",         "",
#endif

  Ctrl('P'), class_newbrd,      Ctrl('P'),    's',    "�}�P�ݪO",     "",

  'h', class_help,      'h',    'z',    "�\\�໡��",     ""
};
#endif


int
Class()
{
  /* XoClass(CH_END - 1); */
  /* Thor.980804: ���� ���� account �y�X class.img �ΨS�� class �����p */
  if (!class_img || !XoClass(CH_END - 1))
  {
    vmsg("���w�q���հQ�װ�");
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
  vmsg("�L�������s��");
  return XEASY;
}
#endif


void
board_main()
{
  int fsize;

  brh_load();

  if (class_img)	/* itoc.030416: �ĤG���i�J board_main �ɡA�n free �� class_img */
  {
    free(class_img);
  }
  else			/* itoc.040102: �Ĥ@���i�J board_main �ɡA�~�ݭn��l�� class_flag */
  {
    class_flag = cuser.ufo & UFO_BRDPOST;	/* �ݪO�C�� 1:�峹�� 0:�s�� */
    if (!cuser.userlevel)			/* guest yank all boards */
      class_flag |= BFO_YANK;

    /* �]�w default board */
    strcpy(currboard, BN_NULL);
    currbno = -1;
  }

  /* class_img = f_img(CLASS_IMGFILE, &fsize); */
  /* itoc.010413: �̷� ufo �Ӹ��J���P�� class image */
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
  /* class_xo = &board_xo; *//* Thor: �w�� default, ���ݧ@�� */

  class_hot = 0;
#ifdef AUTO_JUMPBRD
  if (cuser.ufo & UFO_JUMPBRD)
    class_jumpnext = 1;	/* itoc.010910: �D�ʸ��h�U�@�ӥ�Ū�ݪO */
#endif
  xover(XZ_CLASS);

  return 0;
}