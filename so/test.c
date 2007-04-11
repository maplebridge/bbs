#include "bbs.h"

int main(){
  int fd;
  int oldno;

  if ((fd = open("run/stuno", O_RDONLY)) >= 0)
  {
    while (read(fd, &oldno, 4 ) == 4)
    printf("%d\n",oldno);
  }
}
              