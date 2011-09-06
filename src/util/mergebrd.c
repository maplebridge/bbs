/*-------------------------------------------------------*/
/* util/mergebrd.c	( NTHU CS MapleBBS Ver 楓橋驛站) */
/*-------------------------------------------------------*/
/* target : M3 楓橋 看板合併程式			 */
/* create : 08/05/09					 */
/* author : smiler.bbs@lexel.twbbs.org			 */
/* update : 11/09/05					 */
/*-------------------------------------------------------*/
/* syntax : mergebrd bsrc bdst				 */
/*-------------------------------------------------------*/


#include "bbs.h"

#define FN_DIR_TMP	".DIR.tmp"

/* ----------------------------------------------------- */
/* 轉換主程式						 */
/* ----------------------------------------------------- */


static int
transfer_hdr(old, src, dst)
  HDR *old;
  char *src, *dst;
{
  char fsrc[64], fdst[64];
  HDR new;

  hdr_fpath(fsrc, src, old);
  hdr_fpath(fdst, dst, old);

  char *family, *fname;
  int chrono = old->chrono;

  family = fdst;
  fname = strrchr(fdst, '/') + 2;
  family = fname - 3;

  for (;;)
  {
    if (f_cp(fsrc, fdst, O_EXCL) >= 0)
      break;
  
    if (errno == EEXIST)	/* path already taken, make another new path */
    {
      chrono++;
      *family = radix32[chrono & 31];
      archiv32(chrono, fname);
    }
    else	/* unhandle error */
      return -1;
  }
  
  memcpy(&new, old, sizeof(HDR));
  new.chrono = chrono;
  strcpy(new.xname, --fname);

  rec_add(dst, &new, sizeof(HDR));

  return 0;
}


static int
hdr_cmp(a, b)
  HDR *a;
  HDR *b;
{
  return ((a->xmode & POST_BOTTOM) && !(b->xmode & POST_BOTTOM)) ? 1 :
	(!(a->xmode & POST_BOTTOM) && (b->xmode & POST_BOTTOM)) ? -1 :
	(a->chrono - b->chrono);
}


int
main(argc, argv)
  int argc;
  char *argv[];
{
  FILE *fp;
  char src[64], tmp[64], dst[64];

  char start;
  HDR hdr;

  if (argc != 3)
  {
    printf("Usage: %s <SRC_BRD> <DST_BRD>\n", argv[0]);
    exit(1);
  }

  if (!strcmp(argv[1], argv[2]))
  {
    printf("來源看板需與目的看板相異 !!\n");
    exit(1);
  }

  printf("press key '\e[1;32mY\e[m' to merge \e[1;33m%s\e[m into \e[1;33m%s\e[m, or any other key to exit. ", argv[1], argv[2]);
  scanf("%c", &start);
  if (start != 'Y')
    exit(0);

  chdir(BBSHOME);

  brd_fpath(src, argv[1], FN_DIR);
  brd_fpath(dst, argv[2], NULL);
  brd_fpath(tmp, argv[2], FN_DIR_TMP);

  if (!dashd(dst))
  {
    printf("Error: board <%s> does not exist.\n", argv[2]);
    exit(1);
  }
  else
  {
    brd_fpath(dst, argv[2], FN_DIR);
    unlink(tmp);	/* delete any possible temp index */
    f_cp(dst, tmp, O_EXCL);
    printf("Old <%s> index file has been backed up to: %s\n", argv[2], tmp);
  }

  if (fp = fopen(src, "r"))
  {
    while (fread(&hdr, sizeof(HDR), 1, fp) == 1)
      transfer_hdr(&hdr, src, dst);
    fclose(fp);
  }
  else
  {
    printf("Error: open index file failed: %s\n", src);
    exit(1);
  }

  rec_sync(dst, sizeof(HDR), hdr_cmp, NULL);

  printf("Remember to remove backup index <%s> if the merge work seems well done.\n", tmp);
  exit(0);
}
