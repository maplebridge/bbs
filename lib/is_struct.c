#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>


int
is_struct(fpath, struct_size)	/* 1:此file由struct組成 0:無此檔案 -1:否 */
  char *fpath;
  int  struct_size;
{
  int fd, fsize;
  struct stat st;

  if ((fd = open(fpath, O_RDONLY)) >= 0)
  {
    if (!fstat(fd, &st))
    {
       fsize = st.st_size;
       close(fd);
       return ((fsize % struct_size) == 0) ? 1 : (-1);
    }
    close(fd);
  }
  return 0;

}
