/*-------------------------------------------------------*/
/* util/class-index.c	( NTHU CS MapleBBS Ver 3.20 )	 */
/*-------------------------------------------------------*/
/* target : 看板分類索引程式 (class index)	         */
/* create : 09/01/12				 	 */
/* update : //					 	 */
/*-------------------------------------------------------*/
/* syntax : class-index 				 */
/*-------------------------------------------------------*/


#include	"bbs.h"

#define	COLOR_INDEX	/* Thor.980307: 加上顏色試試是否比較易找 */


#define GINDEX_LOG      (BBSHOME "/run/Class_gindex.log")


#define	CHRONO_INDEX	1
#define	CHRONO_LOG	2


static char fn_index[] = "@Class.index";

static int gem_default;
static int ndir;
static int nfile;
static char pgem[128], pndx[128], pool[128];
static FILE *flog;

#define SHOW_FILE_NAME
#define SHOW_BRD_LIST

#undef SHOW_FILE_NAME

/* visit the hierarchy recursively */

#define	MAX_LEVEL	10

static char gem_title[MAX_LEVEL][73];

static void
gindex(level, toc, fpath, fndx)
  int level;
  char *toc;
  char *fpath;
  FILE *fndx;
{
  int count, xmode, i;
  char *fname, *ptr, buf[128];
  FILE *fgem;
  HDR hdr;

  if (level > MAX_LEVEL)		/* endless loop ? */
  {
    fprintf(flog, "level: %d [%s]\n", level, fpath);
    return;
  }

  if (!level)           /* 第一次進入 */
  {
    fprintf(flog, "%-14s", fpath);
    sprintf(pool, "%s/@Class", fpath);
    fpath = pool;
    strcpy(pgem, fpath);
  }

  if (!(fgem = fopen(fpath, "r")))
    return;

  fname = fpath;
  while (xmode = *fname++)
  {
    if (xmode == '/')
      ptr = fname;
  }
  fname = ptr;

  if (!fndx)
  {
    strcpy(fname, "/@Class.ing--");
    fndx = fopen(fpath, "w");
    if (!fndx)
    {
      fclose(fgem);
      return;
    }
#ifdef SHOW_FILE_NAME
    fprintf(fndx, "%-13.13s\t序號\t\t\t分類主題\n"
	"-------------------------------------------------------------\n", "檔名");
#else
    fprintf(fndx, "序號\t\t\t分類主題\n"
	"-------------------------------------------------------------\n");
#endif

    strcpy(pndx, fpath);
    gem_default = ndir = nfile = 0;
  }

  count = 0;
  while (fread(&hdr, sizeof(hdr), 1, fgem) == 1)
  {
    strcpy(gem_title[level], hdr.title);
    count++;
    xmode = hdr.xmode;

    /* 檢查是否為索引、異動 */
    if (!level && hdr.chrono <= CHRONO_LOG)
    {
      gem_default |= hdr.chrono;
      continue;
    }

    if (hdr.xname[0] == '@')
      ndir++;
    else
      nfile++;

    sprintf(buf, "%.*s%3d. ", level * 4, toc, count);

#ifdef COLOR_INDEX
    /* Thor.980307: 加上顏色試試是否比較易找 */
    if(hdr.xname[0] == '@')
#  ifdef SHOW_FILE_NAME
      fprintf(fndx, "%-13.13s\t%s\033[1;37;%dm%s\033[m\n", hdr.xname, buf, 41 + (level % 6) , hdr.title);
#  else
      fprintf(fndx, "%s\033[1;37;%dm%s\033[m\n", buf, 41 + (level % 6) , hdr.title);
#  endif
#else
    if(hdr.xname[0] == '@')
#  ifdef SHOW_FILE_NAME
      fprintf(fndx, "%-13.13s\t*%s%s\n", hdr.xname, buf + 1, hdr.title);
#  else
      fprintf(fndx, "*%s%s\n", buf + 1, hdr.title);
#  endif

#  ifdef SHOW_BRD_LIST
    else
#  endif
#endif

#ifdef SHOW_BRD_LIST
    if (strcmp(fname, "@People.1P"))      /* smiler.090112: 個人看板不印 */
    {
      for (i = 0; i <= level; i++)
      {
	fprintf(fndx, "*%-*.*s C:%-*.*s   ", BNLEN, BNLEN, hdr.xname, BNLEN + 1, BNLEN + 1, gem_title[i]);
	if (!strncmp(gem_title[i] + BNLEN + 1, "分類 □ ", 8))	/* "Classname/ 分類 □ " */
	  fprintf(fndx, "%s\n", gem_title[i] + BNLEN + 9);
	else
	{
	  ptr = gem_title[i] + BNLEN;
	  if (gem_title[i][BNLEN + 5] == ' ')
	    ptr += 6;
	  while (*ptr == ' ')
	    ptr++;
	  fprintf(fndx, "%s\n", ptr);
	}
/*	fprintf(fndx, "*%-*.*s C:%s\n", BNLEN, BNLEN, hdr.xname, ptr); */
      }
    }
#endif

    if (hdr.xname[0] == '@')
    {
      ptr = hdr.xname;		/* F1234567 */
      sprintf(fname, "%s", ptr);
      gindex(level + 1, buf, fpath, fndx);
    }
  }

  if (!level)
  {
    fclose(fndx);
    strcpy(fname, fn_index);
    rename(pndx, fpath);

    /* report */

    fprintf(flog, "==> d: %d\tf: %d\n", ndir, nfile);

    xmode = gem_default;
    if (xmode != (CHRONO_INDEX | CHRONO_LOG))	/* 已有索引及異動 */
    {
      sprintf(pool, "%s.o", pgem);
      sprintf(pndx, "%s.n", pgem);
      if (fndx = fopen(pndx, "w"))
      {
	memset(&hdr, 0, sizeof(HDR));
	hdr.xmode = GEM_RESERVED;
	strcpy(hdr.owner, SYSOPNICK);

#if 0
	if (!(xmode & CHRONO_INDEX))
	{
	  hdr.chrono = 0;
	  strcpy(hdr.xname, fn_index + 2);
	  strcpy(hdr.title, "精華區索引");
	  fwrite(&hdr, sizeof(hdr), 1, fndx);
	}

	if (!(xmode & CHRONO_LOG))
	{
	  //hdr.chrono = CHRONO_LOG;
	  hdr.chrono = 0;
	  strcpy(hdr.xname, fn_log + 2);
	  strcpy(hdr.title, "精華區異動");
	  fwrite(&hdr, sizeof(hdr), 1, fndx);
	}
#endif
	fseek(fgem, (off_t) 0, SEEK_SET);

	while (fread(&hdr, sizeof(hdr), 1, fgem) == 1)
	{
	  fwrite(&hdr, sizeof(hdr), 1, fndx);
	}

	fclose(fndx);
	fclose(fgem);
	rename(pgem, pool);
	rename(pndx, pgem);
	return;
      }
    }
  }

  fclose(fgem);
}


int
main(argc, argv)
  int argc;
  char *argv[];
{
  char fpath[80];

  umask(077);
  chdir(BBSHOME "/gem");

  flog = stderr;
  sprintf(fpath, "@");
  gindex(0, "", fpath, NULL);
  exit(0);
}
