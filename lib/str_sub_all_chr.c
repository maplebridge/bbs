/* smiler.080912: 可忽略str的 標點符號 以及 ansi 部分 */

#include "dao.h"


char * 
str_sub_all_chr(str, tag) 
  char *str; 
  char *tag;		/* non-empty lowest case pattern */ 
{ 
  int cc, c1, c2;
  char *p1, *p2;
  int in_chi = 0;	/* 1: 前一碼是中文字 */
  int in_chii;		/* 1: 前一碼是中文字 */
  int ansi = 0;     /* 判斷是否為 ansi code */

  cc = *tag++;
 
  while (c1 = *str)
  {
    if (in_chi)
    {
      in_chi ^= 1;
    }
    else
    {
      if (c1 & 0x80)
	     in_chi ^= 1;
      else if (c1 >= 'A' && c1 <= 'Z')
	     c1 |= 0x20;

      if (c1 == cc)
      {
	     p1 = str;
	     p2 = tag;
	     in_chii = in_chi;

	     do
		 {
	       c2 = *p2;
 	       if (!c2)
	          return str;
 
	       p2++;
	       c1 = *++p1;

		   if(c1 == '\033')
			   ansi = 1;

		   while(ansi)
		   {
			   c1 = *++p1;
			   if ((c1 < '0' || c1 > '9') && c1 != ';' && c1 != '[')
			   {
	              ansi = 0;
				  c1 = *++p1;
			   }
		   }

		   if (in_chii || c1 & 0x80)
	          in_chii ^= 1;
	       else if (c1 >= 'A' && c1 <= 'Z')
	          c1 |= 0x20;

		   while((c1 != c2) && 
			   (  (c1 == '`') || (c1 == '~') || (c1 == '!') || (c1 == '@') || (c1 == '#')
			   || (c1 == '$') || (c1 == '%') || (c1 == '^') || (c1 == '&') || (c1 == '*')
			   || (c1 == '(') || (c1 == ')') || (c1 == '-') || (c1 == '_') || (c1 == '=')
			   || (c1 == '+') || (c1 == '[') || (c1 == ']') || (c1 == '\{') || (c1 == '\}')
			   || (c1 == '\\') || (c1 == '|') || (c1 == '\;') || (c1 == ':') || (c1 == '\'')
			   || (c1 == '\"') || (c1 == ',') || (c1 == '<') || (c1 == '.') || (c1 == '>')
			   || (c1 == '/') || (c1 == '?') || (c1 == '\n') || (c1 == '\0') || (c1 == ' ') ) )
		   {
			 c1 = *++p1;

	         if (in_chii || c1 & 0x80)
	            in_chii ^= 1;
	         else if (c1 >= 'A' && c1 <= 'Z')
	            c1 |= 0x20;
		   }

		 } while (c1 == c2); 
	  }
	}
 
    str++;
  }

  return NULL;
}
