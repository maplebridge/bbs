/* smiler.080912: 比較檔案中是否有 tag 字串，可略過 ' ' 以及 '\n' 以及 ansi 部分
   現階段應用於 BRD_DOG 各檔案內容分析程式。
*/

#include <stdio.h>
#include <string.h>


int				/* 1: found 0: none */
f_str_sub_space_lf(fpath, tag)
  char *fpath;
  char *tag;
{
  FILE *fp;
  fpos_t pos;
  int cc;		/* cc 放 fpath 讀出來的字元 */
  int c1, c2;		/* c1 c2 放 tag 讀出來的字元 */
  char *p2;
  int in_chi = 0;	/* 1: 前一碼是中文字 */
  int in_chii;		/* 1: 前一碼是中文字 */

  if (!(fp = fopen(fpath, "r")))
    return 0;

  c1 = *tag++;

  while ((cc = fgetc(fp)) != EOF)
  {
    if (in_chi)		/* 跳過中文字後半字符 */
    {
      in_chi ^= 1;
      continue;
    }

    if (cc & 0x80)
      in_chi ^= 1;
    else if (cc >= 'A' && cc <= 'Z')
      cc |= 0x20;

    if (c1 < 0)
      c1 += 256;

    if (cc != c1)	/* 已經 miss 掉了就跳往下一個 cc 吧 */
      continue;

    fgetpos (fp, &pos);	/* current position in the file */
    p2 = tag;

    do
    {
      if (!(c2 = *p2++))
      {
	fclose(fp);
	return 1;
      }
 
      if (c2 < 0)
	c2 += 256;

      if ((cc = fgetc(fp)) == EOF)
      {
	fclose(fp);
	return 0;
      }

#if 0
      if (cc == '\033')		/* 跳過 ansi */
      {
	while ((cc = fgetc(fp)) != EOF)
	{
	  if ((cc < '0' || cc > '9') && cc != ';' && cc != '[')
	    break;
        }
      }

      if ((cc = fgetc(fp)) == EOF)
      {
	fclose(fp);
	return 0;
      }

      if (in_chii || cc & 0x80)
	in_chii ^= 1;
      else if (cc >= 'A' && cc <= 'Z')
	cc |= 0x20;
#endif

      while ((cc != c2) && ((cc == '\n') || (cc == ' ') || (cc == '\033')))
      {
	if (cc == '\033')	/* 跳過 ansi */
	{
	  while ((cc = fgetc(fp)) != EOF)
	  {
	    if ((cc < '0' || cc > '9') && cc != ';' && cc != '[')
	      break;
	  }
	}

	if ((cc = fgetc(fp)) == EOF)
	{
	  fclose(fp);
	  return 0;
	}

	if (in_chii || cc & 0x80)
	  in_chii ^= 1;
	else if (cc >= 'A' && cc <= 'Z')
	  cc |= 0x20;
      }
    } while (cc == c2); 

    fsetpos (fp, &pos);
  }	/* end -- while (cc = fgetc(fp)) -- */

  fclose(fp);
  return 0;
}


int
f_f_str_sub_space_lf(fpath, tag_fpath)
  char *fpath;
  char *tag_fpath;
{
  FILE *fp;
  char str[73];
  int ans = 0;

  if (!(fp = fopen(tag_fpath,"r")))
    return 0;

  while (fgets(str, sizeof(str), fp))
  {
    str[strlen(str) - 1] = '\0';
    if (f_str_sub_space_lf(fpath, str))
    {
      ans = 1;
      break;
    }
  }

  fclose(fp);
  return ans;
}
