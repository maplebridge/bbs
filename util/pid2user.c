/*-------------------------------------------------------*/
/* util/pid2user.c      (MapleBBS Ver 3.10)              */
/*-------------------------------------------------------*/
/* target : 由pid找出當前站內使用者id和動態              */
/* author : hightman.bbs@bbs.hightman.net                */
/* create : 02/05/19                                     */
/* update :   /  /                                       */
/*-------------------------------------------------------*/


/* hightman.020519: 0
 * 近日發現個別進程占用cpu和memory資源過多，
 * 據此程序觀察該用戶在做什么
 */

#define _MODES_C_

#include "bbs.h"

static UCACHE *ushm;


static int      /* 0: 找不到使用者是此 pid */
pid2user(pid)
  pid_t pid;
{
  UTMP *utmp, *uceil;
  usint offset;

  ushm = shm_new(UTMPSHM_KEY, sizeof(UCACHE));

  offset = ushm->offset;
  if (offset > (MAXACTIVE - 1) * sizeof(UTMP))
    offset = (MAXACTIVE - 1) * sizeof(UTMP);
  utmp = ushm->uslot;
  uceil = (void *) utmp + offset;

  offset = 1;
  do
  {
    if (utmp->pid == pid)
    {
      printf("使用者代號: %s 狀態: %s [%d] 。 (來自 %s)\n",
        utmp->userid, ModeTypeTable[utmp->mode], utmp->mode, utmp->from);
      offset = 2;
      break;
    }
  } while (++utmp <= uceil);

  return (offset - 1);
}


int
main(argc, argv)
  int argc;
  char *argv[];
{
  pid_t pid;
  char c;
  extern int errno;

  if (argc < 2)
  {
    printf("Usage:\t%s <pid>\n", argv[0]);
    exit(-1);
  }

  setgid(BBSGID);
  setuid(BBSUID);
  chdir(BBSHOME);

  pid = atol(argv[1]);

  printf("您輸入的進程號為： [%ld]\n", (long) pid);
  if (!pid2user(pid))
  {
    printf("抱歉，根據 pid: [%ld] 找不到 BBS 使用者。\n", (long) pid);
    exit(0);
  }

  printf("是否殺死該進程(Y/N)？[N] ");
  c = getchar();
  if (c == 'y')
  {
    if (kill(pid, SIGKILL) == 0)
    {
      printf("\n成功\殺掉進程!\n");
    }
    else
    {
      if (errno == EINVAL)
      {
        printf("\nAn invalid signal was specified.\n");
      }
      else if (errno == ESRCH)
      {
        printf("\nThe  pid or process group does not exist.\n"
          "Note that an existing process might be a zombie,\n"
          "a process which  already  committed  termination,\n"
          "but has not yet been wait()ed for.\n");
      }
      else if (errno == EPERM)
      {
        printf("\nThe process does not have permission to \n"
          "send the signal to any of the receiving processes. \n");
      }
      else
      {
        printf("\nFail to kill the process for Unknown Reson.\n");
      }
    }
  }
  exit(0);
}
