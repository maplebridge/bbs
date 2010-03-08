/*-------------------------------------------------------*/
/* util/backupacct.c	( NTHU MapleBBS Ver 3.10 )       */
/*-------------------------------------------------------*/
/* target : 備份所有使用者 .ACCT                         */
/* create : 01/10/19                                     */
/* update :   /  /                                       */
/* author : itoc.bbs@bbs.tnfsh.tn.edu.tw		 */
/*-------------------------------------------------------*/


#include "bbs.h"


int
main()
{
  struct dirent *de;
  DIR *dirp;
  char *ptr, ch;
  char fpath[128], bakpath[128], cmd[256];
  time_t now;
  struct tm *ptime;

  time(&now);
  ptime = localtime(&now);
  /* 建立備份路徑目錄 */
  sprintf(fpath, "%s/acct%02d%02d%02d", BAKPATH, ptime->tm_year % 100, ptime->tm_mon + 1, ptime->tm_mday);
  mkdir(fpath, 0755);

  /* 改變權限使 ftp 傳檔不會漏傳 */
  sprintf(cmd, "cp %s/%s %s/; chmod 644 %s/%s", BBSHOME, FN_SCHEMA, fpath, fpath, FN_SCHEMA);
  system(cmd);

  for (ch = 'a'; ch <= 'z'; ch++)
  {
    sprintf(cmd, "%s/usr/%c", BBSHOME, ch);
    if (chdir(cmd) || !(dirp = opendir(".")))
      exit(-1);

    sprintf(bakpath, "%s/%c", fpath, ch);
    mkdir(bakpath, 0755);

    /* 把各使用者的 .ACCT 都拷貝到 acct/*userid/ */
    while (de = readdir(dirp))
    {
      ptr = de->d_name;

      if (ptr[0] > ' ' && ptr[0] != '.')
      {
	sprintf(cmd, "cp %s/%s %s/%s.ACCT", ptr, FN_ACCT, bakpath, ptr);
	system(cmd);
      }
    }
    closedir(dirp);
  }

  exit(0);
}
