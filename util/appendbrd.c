/*-------------------------------------------------------*/
/* util/mergebrd.c      ( NTHU CS MapleBBS Ver 3.10 )    */
/*-------------------------------------------------------*/
/* target : merge several boards into one board          */
/* create : 07/03/16                                     */
/* update :   /  /                                       */
/*-------------------------------------------------------*/
/* syntax : appendbrd dst src1 [src2 src3 ...]            */
/*-------------------------------------------------------*/
                                                                                
#include "bbs.h"
                                                                                
                                                                                
static int
is_board(brdname)
  char *brdname;
{
  BRD brd;
  int fd;
  int rc = 0;
                                                                                
  if ((fd = open(FN_BRD, O_RDONLY)) >= 0)
  {
    while (read(fd, &brd, sizeof(BRD)) == sizeof(BRD))
    {
      if (!strcmp(brdname, brd.brdname))
      {
        rc = 1;
        break;
      }
    }
    close(fd);
  }
  return rc;
}
                                                                                
                                                                                
static int
merge_board(src, dst)
  char *src, *dst;
{
  int i;
  char x;
  char fnew[64], fold[64];
  char cmd[256];
  if (!strcmp(src, dst))
  {
    printf("不能將同一板合併\n");
    return 0;
  }
                                                                                
  if (!is_board(src))
  {
    printf("沒有 %s 這板！\n", src);
    return 0;
  }
                                                                                
  for (i = 0; i < 32; i++)
  {
    x = radix32[i];                       /* @/ 拋棄 */
    sprintf(fold, "brd/%s/%c/*", src, x);
    sprintf(fnew, "brd/%s/%c/", dst, x);
    sprintf(cmd, "cp %s %s", fold, fnew); /* 假設沒有同樣檔名的。若有會覆蓋 */
    system(cmd);
  }
                                                                                
  brd_fpath(fold, src, FN_DIR);
  brd_fpath(fnew, dst, FN_DIR);
  sprintf(cmd, "cat %s >> %s", fold, fnew);
  system(cmd);
                                                                                
  return 1;
}
                                                                                
static int
hdr_cmp(a, b)
  HDR *a;
  HDR *b;
{
  return a->chrono - b->chrono;
}
                                                                                
                                                                                
int
main(argc, argv)
  int argc;
  char *argv[];
{
  int i, num;
  char *src, *dst;
                                                                                
  char folder[64];
                                                                                
  if (argc < 3)
  {
    printf("Usage: %s dst src1 [src2 src3 ...]\n", argv[0]);
    return -1;
  }
  chdir(BBSHOME);
                                                                                
  dst = argv[1];
  if (!is_board(dst))
  {
    printf("沒有 %s 這板！\n", dst);
    return -1;
  }
                                                                                
  num = 0;
  for (i = 2; i < argc; i++)
  {
    src = argv[i];
    printf("開始將 %s 板合併到 %s 板內\n", src, dst);
    num += merge_board(src, dst);
  }
                                                                                
  if (num > 0)
  {
    brd_fpath(folder, dst, FN_DIR);
    rec_sync(folder, sizeof(HDR), hdr_cmp, NULL);
    printf("%d 個板合併成功\n", num);
  }
  return 0;
}
