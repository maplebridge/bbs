/*-------------------------------------------------------*/
/* util/snap2hdr.c	( NTHU CS MapleBBS Ver 楓橋驛站)	 */
/*-------------------------------------------------------*/
/* target : M3 楓橋開機 friend reload程式			 */
/* create : 07/11/04					 */
/* author : smiler.bbs@lexel.twbbs.org			 */
/*-------------------------------------------------------*/
/* syntax : reload_pal [board]				 */
/*-------------------------------------------------------*/

#include <stdlib.h>
#include "bbs.h"
#include <netinet/in.h>
#include "dao.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

int
rec_sync(fpath, size, fsync, fchk)
  char *fpath;
  int size;
  int (*fsync) ();
  int (*fchk) ();
{
  int fd, fsize;
  struct stat st;

  fsize = 0;

  if ((fd = open(fpath, O_RDWR, 0600)) < 0)
    return fsize;

  if (!fstat(fd, &st) && (fsize = st.st_size) > 0)
  {
    char *base;

    base = (char *) malloc(fsize);
    fsize = read(fd, base, fsize);

    if (fsize >= size)
    {
      if (fchk)		/* 檢查是否有不正確的資料 */
      {
	char *head, *tail;

	head = base;
	tail = base + fsize;
	while (head < tail)
	{
	  if (fchk(head))	/* 此筆資料正確 */
	  {
	    head += size;
	    continue;
	  }

	  /* 有問題的資料要刪除 */
	  tail -= size;
	  if (head >= tail)
	    break;
	  memcpy(head, tail, size);
	}
	fsize = tail - base;
      }

      if (fsize > 0)
      {
	if (fsize > size)
	  qsort(base, fsize / size, size, fsync);

	lseek(fd, 0, SEEK_SET);
	write(fd, base, fsize);
	ftruncate(fd, fsize);
      }
    }
    free(base);
  }
  close(fd);

  if (fsize <= 0)
    unlink(fpath);

  return fsize;
}

int
acct_userno(userid)
  char *userid;
{
  int fd;
  int userno;
  char fpath[64];
  char fpath2[64];
  char fn_acct[10];
  
  usr_fpath(fpath2, userid, FN_ACCT);
  sprintf(fpath,"../%s",fpath2);
  fd = open(fpath, O_RDONLY);
  if (fd >= 0)
  {
    read(fd, &userno, sizeof(userno));
    close(fd);
    return userno;
  }
  return 0;
}

static int
chkpal(pal)
  PAL *pal;
{
  int userno;

  userno = pal->userno;
  return (userno > 0 && userno == acct_userno(pal->userid));
}

void
pal_sync(fpath)
  char *fpath;
{
  int fsize;

  fsize = rec_sync(fpath, sizeof(PAL), str_cmp, chkpal);
  if (fsize > PAL_MAX * sizeof(PAL))
    printf("msg_list_over");
}

int boardnumber=0;

int
main(argc, argv)
  int argc;
  char *argv[];
{
  FILE *fp;                                          
  if (argc > 2)
  {
    printf("Usage: %s [board]\n", argv[0]);
    return -1;
  }

    char buf[64];
    char buf2[64];
	char make_bakup[64];

    struct dirent *de;
    DIR *dirp;

    sprintf(buf, BBSHOME"/brd");
    chdir(buf);
    
    dirp = opendir(".");

    while (de = readdir(dirp))
    {
      int fd;
      char *str;


      str = de->d_name;
      if (*str <= ' ' ||  *str == '.')
	continue;

      if ((argc == 2) && str_cmp(str, argv[1]))
	continue;

      sprintf(buf, "%s/" FN_PAL, str);
      sprintf(make_bakup, "cp %s %s.bak.071111",buf,buf);
      if ( fp = fopen(buf, "r") )
      {
		 system(make_bakup);           /* smiler.071111: 先備份 FN_PAL*/
         pal_sync(buf);
         fclose(fp);
         printf("success reload %s !!\n",buf);
      }
//      else
//        printf("fail to open %s\n",buf);
//      fclose(fp);

	 boardnumber++;
	 printf("%d . %s \n",boardnumber,buf);



	}

    closedir(dirp);


  return 0;
}
