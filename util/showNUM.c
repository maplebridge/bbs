#include "bbs.h"

static UCACHE *ushm;

static inline void
init_ushm()
{
    ushm = shm_new(UTMPSHM_KEY, sizeof(UCACHE));
}

int
main()
{
    init_ushm();
      printf("0\n%d\n0\n0\n", ushm->count);
        exit(0);
}

