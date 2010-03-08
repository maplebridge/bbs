#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>


int
is_struct(fpath, struct_size)	/* 1:此 file 由 struct 組成 0:無此檔案 -1:否 */
  char *fpath;
  size_t struct_size;
{
  struct stat st;

  if (!stat(fpath, &st))
    return (st.st_size % struct_size) ? -1 : 1;
  return 0;
}
