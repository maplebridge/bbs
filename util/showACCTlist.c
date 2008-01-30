/*-------------------------------------------------------*/
/* util/showACCTlist.c  ( NTHU CS MapleBBS Ver 3.10 )    */
/*-------------------------------------------------------*/
/* target : 顯示使用者資料列表 (Modified from showACCT.c)   */
/* create : 08/01/30                                     */
/* update :                                              */
/* author : ckm.bbs@bbs.cs.nthu.edu.tw                   */
/*-------------------------------------------------------*/


#include "bbs.h"

int
main(argc, argv)
  int argc;
  char **argv;
{
  int c;
  
  printf("%-15s %-60s\n", "ID", "Email");
  printf("> ------------------------------------------------------------------------------------------ \n");
  
  for (c = 'a'; c <= 'z'; c++)
  {
    char buf[64];
    struct dirent *de;
    DIR *dirp;

    sprintf(buf, BBSHOME "/usr/%c", c);
    chdir(buf);

    if (!(dirp = opendir(".")))
      continue;

    while (de = readdir(dirp))
    {
      ACCT acct;
      char *str;

      str = de->d_name;
      if (*str <= ' ' || *str == '.')
        continue;

      if ((argc == 2) && str_cmp(str, argv[1]))
        continue;
      
      sprintf(buf, "%s/" FN_ACCT, str);
      if (rec_get(buf, &acct, sizeof(ACCT), 0) < 0)
      {
        printf("%s: read error (maybe no such id?)\n", str);
        continue;
      }
      
      printf("%-15s %-60s\n", acct.userid, acct.email);
    }
  }
 
  return 0;
}
