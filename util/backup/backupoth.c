/*-------------------------------------------------------*/
/* util/backupoth.c     ( YZU WindTopBBS Ver 3.00 )      */
/*-------------------------------------------------------*/
/* target : 備份所有使用者資料                           */
/* create : 95/03/29                                     */
/* update : 97/03/29                                     */
/* author : itoc.bbs@bbs.tnfsh.tn.edu.tw		 */
/*-------------------------------------------------------*/


#include "bbs.h"


int
main()
{
  int i;
  char *str;
  char bakpath[128], cmd[256];
  time_t now;
  struct tm *ptime;
  char *folders[] = {"bin", "etc", "innd", "run", "src", NULL};

  time(&now);
  ptime = localtime(&now);

  sprintf(bakpath, "%s/oth%02d%02d%02d", BAKPATH, ptime->tm_year % 100, ptime->tm_mon + 1, ptime->tm_mday);
  mkdir(bakpath, 0755);

  for (i = 0; str = folders[i]; i++)
  {
    if (*str)
    {
      sprintf(cmd, "tar cfz %s/%s.tgz %s/%s", bakpath, str, BBSHOME, str);
      system(cmd);
    }
  }

  exit(0);
}
