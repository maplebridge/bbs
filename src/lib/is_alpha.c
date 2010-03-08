#include <ctype.h>


int
is_alpha(ch)
  int ch;
{
  return isalpha(ch);
}


#if 0
int
is_alpha(ch)
  int ch;
{
  return ((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z'));
}
#endif
