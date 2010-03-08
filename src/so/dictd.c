/*-------------------------------------------------------*/
/* dictd.c    ( YZU WindTOPBBS Ver 3.10 )		 */
/*-------------------------------------------------------*/
/* author : statue.bbs@bbs.yzu.edu.tw			 */
/* target : 字典					 */
/* create : 01/11/18					 */
/* update : 08/12/11					 */
/*-------------------------------------------------------*/


#include "bbs.h"
#include "iconv.h"


#if 0
smiler.080203:
1.字典相關功能，請見 http://dict.tw/doc/Dict_BSD.htm
  以及 http://netlab.cse.yzu.edu.tw/~statue/freebsd/zh-tut/dict.html

2.iconv.h 相關編碼問題，請見 http://netlab.cse.yzu.edu.tw/~statue/freebsd/zh-tut/converter.html
  特別注意: gcc -I/usr/local/include -L/usr/local/lib -liconv -o my_iconv my_iconv.c
#endif


/* 代碼轉換:從一種編碼轉為另一種編碼 */
static int code_convert(char *tocode, char *fromcode, char *inbuf, int inlen, char *outbuf, int outlen)
{
  iconv_t cd;

  if (!(cd = iconv_open(tocode, fromcode)))
    return -1;

  memset(outbuf, 0, outlen);
  if (iconv(cd, (const char **) &inbuf, (size_t *) &inlen,  &outbuf, (size_t *) &outlen) == -1)
  {
    iconv_close(cd);
    return -1;
  }

  iconv_close(cd);
  return 0;
}


//big5轉為utf-8碼
static int b2u(char *inbuf, int inlen, char *outbuf, int outlen)
{
  return code_convert("UTF-8", "BIG5", inbuf, inlen, outbuf, outlen);
}


int
main_dictd()
{
  char word[64];
  char word2[128];
  char word_utf8[256];
  char tmp[256];
  char fname[64];
  char fname_tmp[64];
  uschar *str, *ptr, *pend;
  int is_ch, is_eng;

  while(1)
  {
    clear();
    move(0, 30);
    outs("\033[1;37;44m◎ 多用途字典 ◎\033[m\n\n");
    outs("此字典來源為 FreeBSD 的 dict-database 以及 cdict5 字典。須輸入完整單字或片語。\n\n");
    outs("若欲取得完整的字典列表，請輸入 -D\n\n");
    outs("查詢英英字典會耗費較久時間，因此預設使用英漢字典查詢\n");
    outs("若要進行完整的字典查詢，請在查詢單字前加上 '-a '\n例:\nword: \033[7m-a book\033[m\n");
#if 0
    outs("字典代號   字典說明\n");
    outs("============================================================================\n");
//    outs("moecomp    Taiwan MOE computer dictionary\n");
//    outs("netterm    Network Terminology\n");
//    outs("pydict     pydict data\n");
//    outs("cedict     Chinese to English dictionary\n");
    outs("web1913    Webster's Revised Unabridged Dictionary (1913)\n");
    outs("wn         WordNet (r) 2.0\n");
    outs("gazetteer  U.S. Gazetteer (1990)\n");
    outs("jargon     Jargon File (4.3.1, 29 Jun 2001)\n");
    outs("foldoc     The Free On-line Dictionary of Computing (27 SEP 03)\n");
    outs("elements   Elements database 20001107\n");
    outs("easton     Easton's 1897 Bible Dictionary\n");
    outs("hitchcock  Hitchcock's Bible Names Dictionary (late 1800's)\n");
    outs("vera       Virtual Entity of Relevant Acronyms (Version 1.9, June 2002)\n");
    outs("devils     THE DEVIL'S DICTIONARY ((C)1911 Released April 15 1993)\n");
    outs("world95    The CIA World Factbook (1995)\n");
    outs("cdict5     Chinese to English and English to Chinese Dictionary\n");
#endif
    memset(word, 0, sizeof(word));
    memset(word2, 0, sizeof(word2));
    is_ch =  is_eng = 0;

    if (!vget(b_lines, 0, "word: ", word, sizeof(word) - 1, DOECHO))
      return 0;

    str = word;
    ptr = word2;
    pend = str + strlen(word);

    while (str < pend)
    {
      if (IS_ZHC_HI(*str) && is_zhc_low(str, 1))
      {
        is_ch = 1;

	if (*(str + 1) == 0x5C)
	{
	  *ptr = *str;
	  *(ptr + 1) = *(str + 1);
	  *(ptr + 2) = 0x5C;
	  ptr++;
	}
	else
	{
	  *ptr = *str;
	  *(ptr + 1) = *(str + 1);
	}
	ptr++;
	str++;
      }
      else
      {
        /* smiler.081211: security reason， 僅接受中文，英文，空格符號輸入 */
	if((*str >= 'a' && *str <= 'z') || (*str >= 'A' && *str <= 'Z') ||
	  (*str == ' ') || (*str == '*') || (*str == '?') || (*str == '-'))
	{
	  *ptr = *str;
	  if (*ptr != ' ')
	    is_eng = 1;
	}
	else
	{
	  vmsg("字典僅接受中文/英文/或是 {*,?,-}");
	  return 0;
	}
      }
#if 0
      if(is_ch && is_eng)
      {
	vmsg("不可同時輸入中英文");
	return 0;
      }
#endif
      str++;
      ptr++;
    }

    sprintf(fname_tmp, "tmp/%s.dictd.tmp", cuser.userid);
    sprintf(fname, "tmp/%s.dictd", cuser.userid);

    if (word[0] == '-' && word[1] == 'a' && word[2] == ' ')
    {
      strcpy(word, word + 3);

      /* smiler.080203: 轉換輸入 由 big5 轉為 utf8 */
      b2u(word, strlen(word), word_utf8, sizeof(word_utf8));

      /* dict */
      sprintf(tmp, "/usr/local/bin/dict \"%s\" > %s", word_utf8, fname_tmp);
      system(tmp);

      /* UTF-8 轉 BIG5 */
      sprintf(tmp, "/usr/local/bin/iconv -f UTF-8 -t big5 %s > %s", fname_tmp, fname);
      system(tmp);
    }
    else
      unlink(fname);

    /* cdict5 */
    sprintf(tmp, BBSHOME"/bin/cdict5/cdict5 \"%s\" >> %s", word, fname);
    system(tmp);

    more(fname, NULL);
#if 0
    /* 使用者可自行按 C 存入暫存檔 */
    sprintf(tmp, "是否把 %s 的查詢結果寄回自己信箱？ (y/N) ", word);
    if (cuser.userlevel && vans(tmp) == 'y')
    {
      sprintf(tmp, "[備 忘 錄] %s 的字典查詢結果", word);
      mail_self(fname, cuser.userid, tmp, 0);
    }
#endif
    unlink(fname_tmp);
    unlink(fname);
  }

  return 0;
}
