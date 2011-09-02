/*-------------------------------------------------------*/
/* util/transkkhdr.c	( NTHU CS MapleBBS Ver 3.00 )	 */
/*-------------------------------------------------------*/
/* target : KKCity 看板 .DIR 轉換			 */
/* create : 10/10/03				 	 */
/* update : 					 	 */
/*-------------------------------------------------------*/


#include "bbs.h"

#if 0

這支程式用來將 KKCity 的看板 index 檔轉換為 maple(楓橋) 相容形式。
轉換的 flag 對照可以在 trans_hdr() 中查看，
使用前要先開好看板，並將 KKCity 的看板備份檔置換到正確的 ~/brd/<brdname> 位置，
然後執行 transkkhdr <brdname>

#endif

/*-------------------------------------------------------*/
/* 轉換程式						 */
/*-------------------------------------------------------*/


#define FN_DIR_TMP	".DIR.tmp"
#define FN_DIR_OLD	".DIR.old"


static void
trans_hdr(board)
  char *board;
{
  int fd;
  HDR old, new;
  char tmp_dir[64], old_dir[64], cur_dir[64];
//  struct stat st;

  brd_fpath(tmp_dir, board, FN_DIR_TMP);
  brd_fpath(old_dir, board, FN_DIR_OLD);
  brd_fpath(cur_dir, board, FN_DIR);
  rename(cur_dir, old_dir);

  if (dashf(tmp_dir))
    unlink(tmp_dir);

  if ((fd = open(old_dir, O_RDONLY)) >= 0)
  {
    while (read(fd, &old, sizeof(HDR)) == sizeof(HDR))
    {
      memset(&new, 0, sizeof(HDR));
      memcpy(&new, &old, sizeof(HDR));

      if (old.xmode & 0x00001000)
      {
	/* 轉換的動作在此 */
	new.nick[sizeof(new.nick) - 1] = '\0';
	strcpy(new.mark, "重要");
	new.color = '3';
	new.xmode &= ~0x00001000;
	new.xmode |= POST_BOTTOM;
      }
      else
      {
	new.nick[sizeof(new.nick) - 1] = '\0';
	strcpy(new.mark, "");
	new.color = '\0';
      }

      if (old.xmode & 0x10000000)
      {
	new.xmode &= ~0x10000000;
	new.xmode |= POST_SCORE;
	new.score = *(old.owner + 79);
      }
      else
	new.score = 0;

      if (old.xmode & 0x0004)
      {
	new.xmode &= ~0x0004;
	new.xmode |= POST_GEM;
      }

      if (old.xmode & 0x10000)
      {
	new.xmode &= ~0x10000;
	new.xmode |= POST_RESTRICT;
      }

      if (old.xmode & 0x0200)
      {
	new.xmode &= ~0x0200;
	new.xmode |= POST_INCOME;
      }

      new.owner[75] = '\0';
      new.stamp = 0;
      new.nick[29] = '\0';
      strcpy(new.value, "");
      rec_add(tmp_dir, &new, sizeof(HDR));
    }
    close(fd);
  }

  if (dashf(cur_dir))
    printf("error: %s\n", board);
  else
    rename(tmp_dir, cur_dir);
/*
  printf(" - %s\n", board);
  stat(index, &st);
  printf("      - old: %d\n", st.st_size);
  stat(folder, &st);
  printf("      - new: %d\n\n", st.st_size);
*/
}

/*-------------------------------------------------------*/
/* 主程式						 */
/*-------------------------------------------------------*/

int
main(argc, argv)
  int argc;
  char *argv[];
{
  int fd;
  BRD brd;

  /* argc == 1 轉全部板 */
  /* argc == 2 轉某特定板 */

  if (argc != 2)
  {
    printf("Usage: %s <target_board>\n", argv[0]);
    exit(-1);
  }

  chdir(BBSHOME);
  if (!dashf(FN_BRD))
  {
    printf("ERROR! Can't open " FN_BRD "\n");
    exit(-1);
  }

  if ((fd = open(FN_BRD, O_RDONLY)) >= 0)
  {
    while (read(fd, &brd, sizeof(BRD)) == sizeof(BRD))
    {
      if (!*brd.brdname)	/* 此板已被刪除 */
	continue;

      if (argc == 1)
      {
	trans_hdr(brd.brdname);
      }
      else if (!str_cmp(brd.brdname, argv[1]))
      {
	trans_hdr(brd.brdname);
	break;
      }
    }
    close(fd);
  }
  return 0;		//  exit(0);
}

