/* smiler.080912: 比較檔案中是否有 tag 字串，可略過 "標點符號" 以及 "ansi" 部分 
   現階段應用於 BBS_DOG 各檔案內容分析程式。
*/

#include <stdio.h>
#include <string.h>

int			/* 1: found 0: none */
f_str_sub_all_chr(fpath, tag)
  char *fpath;
  char *tag;
{ 
  FILE *fp;
  fpos_t pos;
  int cc, c1, c2;
  char *p2;
  int in_chi = 0;	/* 1: 前一碼是中文字 */
  int in_chii;		/* 1: 前一碼是中文字 */
  int ansi = 0;		/* 判斷是否為 ansi code */

  if (!(fp = fopen(fpath, "r")))
    return 0;

  cc = *tag++;

  while (c1 = fgetc(fp))
  {
    if (feof(fp))
    {
      fclose(fp);
      return 0;
    }

    if (in_chi)
    {
      in_chi ^= 1;
    }
    else
    {
      if (c1 & 0x80)
	in_chi ^= 1;
      else if(c1 >= 'A' && c1 <= 'Z')
	c1 |= 0x20;

      if (cc < 0)
	cc = 256 + cc;

      if (c1 == cc)
      {
	fgetpos (fp, &pos);
	p2 = tag;

	do
	{
	  c2 = *p2;
	  if (c2 < 0)
	    c2 = 256 + c2;

	  if (!c2)
	    return 1;
 
	  p2++;
	  c1 = fgetc(fp);
	  if (feof(fp))
	  {
	    fclose(fp);
	    return 0;
	  }

	  if (c1 == '\033')
	    ansi = 1;

	  while (ansi)
	  {
	    c1 = fgetc(fp);
	    if (feof(fp))
	    {
	      fclose(fp);
	      return 0;
	    }

	    if ((c1 < '0' || c1 > '9') && c1 != ';' && c1 != '[')
	    {
	      ansi = 0;
	      c1 = fgetc(fp);
	      if (feof(fp))
	      {
		fclose(fp);
		return 0;
	      }
	    }
	  }

	  if (in_chii || c1 & 0x80)
	    in_chii ^= 1;
	  else if(c1 >= 'A' && c1 <= 'Z')
	    c1 |= 0x20;

	  while ((c1 != c2) && 
	    ((c1 == '`')  || (c1 == '~') || (c1 == '!')  || (c1 == '@')  || (c1 == '#')
	    || (c1 == '$')  || (c1 == '%') || (c1 == '^')  || (c1 == '&')  || (c1 == '*')
	    || (c1 == '(')  || (c1 == ')') || (c1 == '-')  || (c1 == '_')  || (c1 == '=')
	    || (c1 == '+')  || (c1 == '[') || (c1 == ']')  || (c1 == '\{') || (c1 == '}')
	    || (c1 == '\\') || (c1 == '|') || (c1 == ';') || (c1 == ':')  || (c1 == '\'')
	    || (c1 == '\"') || (c1 == ',') || (c1 == '<')  || (c1 == '.')  || (c1 == '>')
	    || (c1 == '/')  || (c1 == '?') || (c1 == '\n') || (c1 == ' ')  || (c1 == '\033') ))
	  {
	    if (c1 == '\033')
	      ansi = 1;

	    while (ansi)
	    {
	      c1 = fgetc(fp);
	      if (feof(fp))
	      {
		fclose(fp);
		return 0;
	      }

	      if ((c1 < '0' || c1 > '9') && c1 != ';' && c1 != '[')
	      {
		ansi = 0;
		c1 = fgetc(fp);
		if (feof(fp))
		{
		  fclose(fp);
		  return 0;
		}
	      }
	    }

	    c1 = fgetc(fp);
	    if (feof(fp))
	    {
	      fclose(fp);
	      return 0;
	    }

	    if (in_chii || c1 & 0x80)
	      in_chii ^= 1;
	    else if(c1 >= 'A' && c1 <= 'Z')
	      c1 |= 0x20;
	  }
	} while (c1 == c2); 

	fsetpos (fp, &pos);

      }
    }
  }
  fclose(fp);
  return 0;
}


int
f_f_str_sub_all_chr(fpath, tag_fpath)
  char *fpath;
  char *tag_fpath;
{
  FILE *fp;
  char str[73];
  int ans = 0;

  if (!(fp = fopen(tag_fpath,"r")))
    return 0;

  while (fgets(str, 73, fp))
  {
    str[strlen(str) - 1] = '\0';
    if (f_str_sub_space_lf(fpath, str))
    {
      fclose(fp);
      return 1;
    }
  }
  fclose(fp);
  return 0;
}
