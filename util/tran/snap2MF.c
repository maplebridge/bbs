/*-------------------------------------------------------*/
/* util/snap2usr.c	( NTHU CS MapleBBS Ver 3.10 )	 */
/*-------------------------------------------------------*/
/* target : M3 ACCT 轉換程式				 */
/* create : 98/12/15					 */
/* update : 02/04/29					 */
/* author : mat.bbs@fall.twbbs.org			 */
/* modify : itoc.bbs@bbs.tnfsh.tn.edu.tw		 */
/*-------------------------------------------------------*/
/* syntax : snap2usr [userid]				 */
/*-------------------------------------------------------*/


#include "snap_ry.h"
#include <netinet/in.h>



/* ----------------------------------------------------- */
/* 轉換主程式						 */
/* ----------------------------------------------------- */

int transmftype(usint oldtype);

static void trans_MF(MyF* old, MF* new)
{
  usint mftype;
  memset(new, 0, sizeof(MF));

  new->chrono = ntohl(old->chrono);
  
  mftype = ntohl(old->mftype);
  new->mftype = transmftype(mftype);
  
  str_ncpy(new->xname, old->xname, sizeof(new->xname));
  str_ncpy(new->title, old->title, sizeof(new->title));
  if(new->mftype & MF_LINE)
    strcpy(new->title, "------ ------ 分隔線 ------ ------");


}

int
main(argc, argv)
  int argc;
  char *argv[];
{
/*  MF new;*/
  char c;

  if (argc > 2)
  {
    printf("Usage: %s [userid]\n", argv[0]);
    return -1;
  }

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
      MyF old;
      int fd;
      char *str;
      
      struct dirent* de2;
      DIR* dirp2;

      str = de->d_name;
      if (*str <= ' ' || *str == '.')
	continue;

      if ((argc == 2) && str_cmp(str, argv[1]))
	continue;
/*
#ifdef MAK_DIRS
      sprintf(buf, "%s/MF", str);
      mkdir(buf, 0700);
      sprintf(buf, "%s/gem", str);
      mak_links(buf);
#endif
*/
      sprintf(buf, BBSHOME "/usr/%c/%s/MF" , c, str);
      chdir(buf);

      if(!(dirp2 = opendir(".")))
        continue;

      while( de2 = readdir(dirp2))
      {
        int i, counter;
        MF tempmf[1024];
        /*char* str2;*/

        if(de2->d_name[0]=='.'||de2->d_name[0]==' ')
          continue;

        if((fd = open(de2->d_name, O_RDONLY)) < 0)
          continue;

        for(counter=0;read(fd, &old, sizeof(MyF));counter++)
        {
          trans_MF(&old, tempmf+counter);
        }
        close(fd);
        unlink(de2->d_name);
        fd = open(de2->d_name,  O_WRONLY | O_CREAT, 0600);
        
        for(i = 0;i < counter;i++)
        {
          write(fd, tempmf+i, sizeof(MF));
        }
        close(fd);
      }
      
      if((fd = open("@MF", O_RDONLY)) < 0)
        continue;
      MF tempMF;
      int fd2 = open("@MyFavorite",  O_WRONLY | O_CREAT, 0600);
      while( read(fd, &tempMF, sizeof(MF)) )//finding @MF and change
      {
        write(fd2, &tempMF, sizeof(MF));
      }
      close(fd);
      close(fd2);
      unlink("@MF");
      
      closedir(dirp2);
      /*
      if ((fd = open(buf, O_RDONLY)) < 0)
	continue;

      read(fd, &old, sizeof(MyF));
      close(fd);
      unlink(buf);			/* itoc.010831: 砍掉原來的 FN_ACCT */

      /*trans_MF(&old, &new);

      fd = open(buf, O_WRONLY | O_CREAT, 0600);	/* itoc.010831: 重建新的 FN_ACCT */
      /*write(fd, &new, sizeof(ACCT));
      close(fd);*/
    }

    closedir(dirp);    
  }

  return 0;
}

int transmftype(usint oldtype)
{
  int newtype=0;
  if(oldtype & OLDMF_LINE)
    newtype |= MF_LINE;
  if(oldtype & OLDMF_BOARD)
    newtype |= MF_BOARD;
  if(oldtype & OLDMF_FOLDER)
    newtype |= MF_FOLDER;
  if(oldtype & OLDMF_GEM)
    newtype |= MF_GEM;
  return newtype;
}
