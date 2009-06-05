/*

smiler.090416: 此程式針對擁有 ansi code 的 string 作相關計算位置及處理

0. color  => 傳回 strlen_ansi 所 trace 過之 ansi 色碼
             若為 NULL，則不傳


1. encode => 找尋 data 中特定解碼後位置，並回傳該 data 解碼前位置
                                                                                
   ex:
      strlen_ansi("\033[1;33m這只是測試\033[m", 10, "DECODE", color);
      return 17
      (要找尋screen上位置10在此string內之位置)
                                                                                
2. decode => 找尋 data 中解碼前位置，並回傳該位置解碼後長度
                                                                                
   ex:
      strlen_ansi("\033[1;33m這只是測試\033[m", 10, "DECODE", color);
      return 3
      (要計算此string上第10個位置在screen上之位置)

*/

#include "bbs.h"

int
strlen_ansi(data, c, coder, color)
  char *data;
  int c;
  char *coder;
  char *color;
{
  int type;          /* 運作模式: ENCODE or DECODE */
  int count;         /* 計數 */
  int x;             /* 目前讀取至 data+x */
  int ch, ansi;
  int len;
  char *str;
  
  if (!strcmp(coder, "ENCODE"))
    type = 0;        /* ENCODE */
  else
    type = 1;        /* DECODE */


  if (!type)
  {
	count = c;
	len = ANSILINELEN;  /* 最大容許長度 */
  }
  else
  {
	count = 0;
	len = c;
  }


  ansi = 0;
  x = 0;

  str = data - 1;
  //ch = *str;
  len ++;

  while (len)   /* 最大 len 長度 */
  {

    if ((type == 0) && (count <= 0 /*|| ch == '\0'*/) )  /* 已經數到該位置了，return 之 */
       break;
    else if((type == 1) && (x == c))
    {
       x = count;
       break;
    }

    str++;
    len--;
    x++;
    ch = *str;

    if (ch == '\0')                    /* 已讀到行尾，補足剩餘部份 */
    {
      if (!type)
	x = x + count - 1;
      else
        x = count + c - x + 1;
      break;
    }

    if (ansi)
    {
       if (ch == 'm')
	 ansi = 0;

       if (color)
       {
         *color = ch;
         color++;
       }
      
       continue;
    }
    if (ch == KEY_ESC)
    {
       ansi = 1;
       
       if (color)
       {
         *color = ch;
         color++;
       }
       
       continue;
    }

    if (!type)
      count--;                       /* ENCODE 時不算 ansi 數 */
    else
      count++;                       /* DECODE 時計算 strlen - ansi 長度 */

  }

  return x;

}