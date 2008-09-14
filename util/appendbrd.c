/*-------------------------------------------------------*/
/* util/appendbrd.c ( NTHU CS MapleBBS Ver 楓橋驛站 )    */
/*-------------------------------------------------------*/
/* target : M3 楓橋看板複製程式	     			 */
/* create : 08/09/15					 */
/* author : smiler.bbs@bbs.cs.nthu.edu.tw	         */
/*-------------------------------------------------------*/
/* syntax : appendbrd board_src board_dst		 */
/*-------------------------------------------------------*/


#include "bbs.h"

int
main(argc, argv)
  int argc;
  char *argv[];
{
  FILE *fp;                      
  HDR hdr, old;
  char buf[64], folder_src[64], folder_dst[64];
  char brdname_src[BNLEN + 1], brdname_dst[BNLEN + 1];

  if (argc < 2)
  {
    printf("Usage: %s brdname_src brdname_dst \n", argv[0]);
    return -1;
  }

  strcpy(brdname_src, argv[1]);
  strcpy(brdname_dst, argv[2]);

  sprintf(buf, BBSHOME);
  chdir(buf);

  brd_fpath(folder_src, brdname_src, FN_DIR);
  brd_fpath(folder_dst, brdname_dst, FN_DIR);

  if ((fp = fopen(folder_src, "r")))
  {
     while (fread(&old, sizeof(old), 1, fp) == 1)
     {
        hdr_fpath(buf, folder_src, &old);         
        hdr_stamp(folder_dst, HDR_COPY | 'A', &hdr, buf);

        hdr.xmode  = old.xmode;
        hdr.parent_chrono = old.parent_chrono;
        strcpy(hdr.owner, old.owner);
        hdr.stamp  = old.stamp;
        strcpy(hdr.nick , old.nick);
        hdr.score  = old.score;
        strcpy(hdr.date , old.date);
        strcpy(hdr.title, old.title);

        rec_bot(folder_dst, &hdr, sizeof(HDR));
     }
	 
     fclose(fp);

  }
  else
     printf("無此看板或看板內內無資料 !!\n");
 
  return 0;
}
