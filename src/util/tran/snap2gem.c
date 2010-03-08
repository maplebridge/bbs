/*-------------------------------------------------------*/
/* util/snap2maplecsgem.c	( NTHU CS MapleBBS )	 */
/*-------------------------------------------------------*/
/* target : M3 MAPLECS gem 轉換程式			 */
/* create : 07/03/31					 */
/* author : smiler.bbs@lexel.twbbs.org			 */
/*-------------------------------------------------------*/
/* syntax : snap2maplecsgem [board]			 */
/*-------------------------------------------------------*/

#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "snap.h"
#include <netinet/in.h>

#define FN_DIR_TMP	".DIR.tmp"
#define FN_DIR_TMP2

char bufini[64];    /* 暫存路徑 */
int lora = 1;

int isopen = 0;       // for debug only
char global_buf[64];

#define MAX_STACK_SIZE 5000000   /* maximum stack size */
#define XNAME_SIZE 32            /* 檔名長度 */
#define usekey  0                /* 是否使用key */


typedef struct t_node *t_pointer;
typedef struct t_node{
  char name[64];
  t_pointer next;
}        t_node;


static t_pointer head = NULL;
static t_pointer tail = NULL;
static t_pointer trace = NULL;


t_pointer create_t(void)
{
  t_pointer root;
  root = (t_pointer) malloc(sizeof (t_node));
  root->name[0] = '\0';
  root->next = NULL;

  return root;
}


void add_t(char *name)
{
  if (!head)
  {
    head = create_t();
    strcpy(head->name,name);
    tail = head;
  }
  else
  {
    tail->next = create_t();
    tail = tail->next;
    strcpy(tail->name,name);
  }
}


void clear_t()	//smiler 1014
{
  trace = head;
  while (1)
  {
    if (trace == tail)
    {
      free(tail);
      head = NULL;
      tail = NULL;
      trace = NULL;
      break;
    }
    else
    {
      trace = head->next;
      free(head);
      head = trace;
    }
  }
}


int Is_inside(char *name)
{
  trace = head;
  while (1)
  {
    if (strcmp(trace->name,name))
    {
      if (trace == tail)
	return 0;
      else
	trace = trace->next;
    }
    else
      return 1;
  }
}


typedef struct{
#if usekey
  int key;
#endif
  char xname[XNAME_SIZE];
}        element;


element stack[MAX_STACK_SIZE];
int top = -1;

int IsEmpty(int top);
int IsFull(int top);
void add(element item);
void Delete(element *selement);
char empty[32] = "                                ";


int IsEmpty(int top)
{
  if (top < 0)
    return 1;
  else
    return 0;
}


int IsFull(int top)
{
  if (top == (MAX_STACK_SIZE-1))
    return 1;
  else
    return 0;
}


void add(element item)
{
  /* add an item to the global stack */
  lora++;
  if (top >= (MAX_STACK_SIZE-1))
  {
    printf("Stack is full now !!\n");
    return;
  }
  top = top + 1;
#if usekey
  stack[top].key = item.key;
#endif
  //stack[top].xname[0]=item.xname[0];
  strcpy(stack[top].xname, item.xname);
}


void Delete(element *selement)
{
  /* return the top element from the stack */
  element deleteelement;
#if usekey
  deleteelement.key = 0;
#endif
  //deleteelement.opd=' ';
  strcpy(deleteelement.xname,empty);

  if (top == -1)
  {
    printf("Stack is empty now !!\n");
    //return deleteelement;
  }
  else
  {
#if usekey
    deleteelement.key = stack[top].key;
#endif
    // deleteelement.opd = stack[top].opd;
    strcpy(deleteelement.xname, stack[top].xname);
#if usekey
    stack[top].key = 0;
    //此處表示stack只存integer,否則要做修改
#endif
    // stack[top].opd=' ';
    strcpy(stack[top].xname,empty);
    top--;
    //printf("inside deleteelement.xname = %s\n",deleteelement.xname);
    //return deleteelement;
    strcpy(selement->xname, deleteelement.xname);
#if usekey
    selement->key = deleteelement.key;
#endif
  }
}


void stkprintf(void)
{
  int i;
  //printf("top tpo == %d\n",top);
  if (top == -1)
    printf("empty now\n");
  else
  {
#if usekey
    for (i = 0; i <= top; i++)
    {
      printf("%d  ", stack[i].key);
    }
#endif
    printf("\n");
    for (i = 0; i <= top; i++)
    {
      printf("%s  ", stack[i].xname);
    }
    printf("\n");
  }
}

int boardnumber = 0;

/* ----------------------------------------------------- */
/* 轉換主程式						 */
/* ----------------------------------------------------- */


int
trans_hdr(old, new , name)
  MAPLECS_HDR *old;
  HDR *new;
  char name[32];
{
  memset(new, 0, sizeof(HDR));
#if 0
  if (head)
  {
    if(Is_inside(old->xname))
    {
      new->chrono=old->chrono;
      new->xmode=old->xmode;
      new->xid=old->xid;
      str_ncpy(new->xname, old->xname, sizeof(new->xname));
      str_ncpy(new->owner, old->owner, sizeof(new->owner));
      str_ncpy(new->nick, old->nick, sizeof(new->nick));
      new->score=' ';
      str_ncpy(new->date, old->date, sizeof(new->date));
      str_ncpy(new->title, old->title, sizeof(new->title));

      strcpy(name,new->xname);
      printf("======>now %s\n",name);
      return 0;
    }
    else
      add_t(old->xname);
  }
  else
    add_t(old->xname);
#endif
  //if(Is_inside(old->xname))
  if (0)
  {
    printf("=========>now  %s\n", old->xname);
    getchar();
  }
  //printf("%s\n",old->xname);

  new->chrono = ntohl(old->chrono);
  new->xmode = ntohl(old->xmode);
  //修改xmode//

  if (isopen)
    printf("old xmode = %x\n",old->xmode);
  //GEM_GOPHER lexel有,itoc沒有,改位置至 0x10000000
  if ((new->xmode & 0x00040000)==0x00040000)  
    new->xmode = new->xmode - 0x00040000 + 0x10000000;
  //GEM_HTTP   lexel有,itoc沒有,改位置至 0x20000000
  if ((new->xmode & 0x00080000)==0x00080000)  
    new->xmode = new->xmode - 0x00080000 + 0x20000000;
  //存於楓橋的xmode,某一代SA可能忘記renew了
  if ((new->xmode & 0x00000100)==0x00000100)
    new->xmode = new->xmode - 0x00000100;
  if ((new->xmode & 0x00000010)==0x00000010)
    new->xmode = new->xmode - 0x00000010;

  if (isopen)
    printf("new xmode = %x\n",new->xmode);

  new->xid = ntohl(old->xid);

  str_ncpy(new->xname, old->xname, sizeof(new->xname));

  if (isopen)
    printf("%s \n",new->xname);

  strcpy(name,new->xname);

  str_ncpy(new->owner, old->owner, sizeof(new->owner));

  if (isopen)
    printf("%s \n",new->owner);

  str_ncpy(new->nick, old->nick, sizeof(new->nick));

  new->score = 0;

  str_ncpy(new->date, old->date, sizeof(new->date));
  str_ncpy(new->title, old->title, sizeof(new->title));

  if (isopen)
    printf("%s \n",new->title);

  if (isopen)
    getchar();

  if (head)
  {
    if (Is_inside(old->xname))
    {
      printf("======>now %s\n",old->xname);
      return 0;
    }
    else
      add_t(old->xname);
  }
  else
    add_t(old->xname);

  return 1;
}


int
trans_gem(name ,oxmode)
  char name[32];
  int oxmode;
{
  FILE *fp;
  FILE *fps;
  FILE *f_turned;
  FILE *f_tex;
  HDR hdr;
  MAPLECS_HDR old;
  char bufs[64];
  char bufs1[64];
  char bufs2[64];
  char buf_turned[64];
  char buftmp1[64];
  //char buftmp2[64];
  char tex[64];
  int notopen = 0;
  int t_continue = 0;

  sprintf(buf_turned,"%s/%s.tmp", bufini, name);
  f_turned = fopen(buf_turned, "r");
  if (f_turned ==NULL)  //偵測是否已對相同檔案trans過
  {
    if (name[0] == '@')
    {
      sprintf(bufs1, "%s/%c/%s", bufini, name[0], name);
      sprintf(bufs2, "%s/%c/%s.tmp", bufini, name[0], name);	
    }
    else
    {
      sprintf(bufs1, "%s/%c/%s", bufini, name[7], name);
      sprintf(bufs2, "%s/%c/%s.tmp", bufini, name[7], name);
    }

    if (isopen)
    {
      printf("now in file name %s\n", name);
      printf("initial notopen = %d\n", notopen);
      getchar();
    }

    if (fp = fopen(bufs1, "r"))
    {
      while (((fread(&old, sizeof(old), 1, fp) == 1)) && (notopen == 0))
      {
	char sname[32];
	int sxmode;
	//if(!trans_hdr(&old, &hdr ,sname))
	//  continue;
	t_continue=trans_hdr(&old, &hdr ,sname);
	if (sname[0] == '@')
	  sprintf(bufs, "%s/%c/%s", bufini, sname[0], sname);
	else
	  sprintf(bufs, "%s/%c/%s", bufini, sname[7], sname);
	sxmode = hdr.xmode;

	if (sxmode == 0 || sxmode == 0x00000800)
	{
	  if (isopen)
	    printf("only a article\n");
	  rec_add(bufs2, &hdr, sizeof(HDR));
	}
	else if (((sxmode & 0x00010000) == 0x00010000 ) &&
	  ((sxmode & 0x10000000) == 0x00000000 ))
	{
	  if (isopen)
	    printf("have content\n");
	  rec_add(bufs2, &hdr, sizeof(HDR));
	  if (fps = fopen(bufs, "r"))
	  {
	    if (isopen)
	      printf("do something\n");
	    if (t_continue)  //smiler 1014
	      trans_gem(sname,sxmode);
	    fclose(fps);
	  }
        }
        else
        {
	  if (isopen)
	    printf("only a title\n");
	  rec_add(bufs2, &hdr, sizeof(HDR));
        }
      }
      fclose(fp);
    }

    /* 刪除舊的，把新的更名 */
    if (notopen == 0)
    {
      int result;
      if (isopen)
      {
	printf("now unlink bufs1=%s\n",bufs1);
	getchar();
      }
      if (name[0] == '@')
	sprintf(buftmp1, "cd %s/%c ; cp %s.tmp .. ; cp %s.tmp %s",
	  bufini, name[0], name, name, name);
      else
	sprintf(buftmp1, "cd %s/%c ; cp %s.tmp .. ; cp %s.tmp %s",
	  bufini, name[7], name, name, name);
      system (buftmp1); //若此檔案不曾被trans過,則把 filename.tmp 放進 brd/版名/內,以供之後的辨認

      unlink(bufs1);
      if(isopen)
      {
	printf("prepare to rename....\n");
	printf("bufs1 = %s\n", bufs1);
	printf("bufs2 = %s\n", bufs2);
	printf("now rename(bufs2=%s,bufs1=%s)\n", bufs2, bufs1);
	getchar();
      }
      result = rename(bufs2, bufs1);
      if (result != 0 )
      {
	perror( "Error renaming file" );
	result = rename(bufs2, bufs1);
	if (result != 0 )
	{
	  if (isopen)
	    system ("ls");

	  printf("%3d  %s\n",boardnumber, global_buf);

	  element tmp;
#if usekey
	  tmp.key = 0;
#endif
	  strcpy(tmp.xname, name);
	  add(tmp);

	  if (isopen)
	    printf("send to stack !!\n");
	}
      }
      if (isopen)
      {
	printf("after rename(bufs2=%s,bufs1=%s)\n", bufs2, bufs1); //
	getchar();
      }
    }
  }
}


int
main(argc, argv)
  int argc;
  char *argv[];
{
  FILE *fp;
  FILE *fps;
  HDR hdr;

  if (argc > 2)
  {
    printf("Usage: %s [board]\n", argv[0]);
    return -1;
  }

  int sxmode;
  char buf[64];
  char buf2[64];
  char bufs[64];

  struct dirent *de;
  DIR *dirp;

  sprintf(buf, BBSHOME"/gem/brd");
  chdir(buf);

  dirp = opendir(".");

  while (de = readdir(dirp))
  {
    MAPLECS_HDR old;
    char *str;

    str = de->d_name;
    if (*str <= ' ' || *str == '.')
      continue;

    if ((argc == 2) && str_cmp(str, argv[1]))
      continue;

    sprintf(buf, "%s/" FN_DIR, str);
    sprintf(buf2, "%s/" FN_DIR_TMP, str);
    sprintf(bufini, "%s", str);

    if ((fp = fopen(buf, "r")))
    {
      while (fread(&old, sizeof(old), 1, fp) == 1)
      {
	char name[32];
	int t_continue = 0;
	//if (!trans_hdr(&old, &hdr ,name))
	//  continue;
	t_continue = trans_hdr(&old, &hdr ,name);
	//printf("%s\n",hdr.title);
	//printf("t_continue=%d\n",t_continue);
	//getchar();
	if (isopen)
	{
	  printf("1.%s = \n\n",name);
	  getchar();
	}

	//rec_add(buf2, &hdr, sizeof(HDR));

	if (name[0] == '@')
	  sprintf(bufs, "%s/%c/%s", bufini, name[0], name);
	else
	  sprintf(bufs, "%s/%c/%s", bufini, name[7], name);
	sxmode = hdr.xmode;
	if (sxmode == 0 || sxmode == 0x00000800)
	  rec_add(buf2, &hdr, sizeof(HDR));
	else if(((sxmode & 0x00010000)==0x00010000) &&
	  ((sxmode & 0x10000000)==0x00000000))
	{
	  rec_add(buf2, &hdr, sizeof(HDR));
	  if (fps = fopen(bufs, "r"))
	  {
	    //if(!Is_inside(name))  //smiler 1014
	    if (t_continue)
	      trans_gem(name,0x00010000);
	    fclose(fps);
	  }
	}
	else
	  rec_add(buf2, &hdr, sizeof(HDR));
      }
      fclose(fp);

      boardnumber++;
      printf("%3d  %s\n",boardnumber,buf);
      strcpy(global_buf,buf);
      /*刪除舊的,把新的更名*/
      unlink(buf);
      rename(buf2, buf);
      clear_t();	//smiler 1014
      while (top > (-1))
      {
	char buftmp[64];
	char buftmp2[64];
	element tmp;
#if usekey
	tmp.key = 0;
#endif
	Delete(&tmp);
	if (tmp.xname[0] == '@')
	{
	  sprintf(buftmp, "%s/%c/%s", bufini, tmp.xname[0], tmp.xname);
	  sprintf(buftmp2, "%s/%c/%s.tmp", bufini, tmp.xname[0], tmp.xname);
	}
	else
	{
	  sprintf(buftmp, "%s/%c/%s", bufini, tmp.xname[7], tmp.xname);
	  sprintf(buftmp2, "%s/%c/%s.tmp", bufini, tmp.xname[7], tmp.xname);
	}
	rename(buftmp2,buftmp);
      }
    }
    //char buf_rm[64];
    //sprintf(buf_rm,"cd %s;rm -rf *.tmp;",bufini);
    //system(buf_rm);
  }

  while (top > (-1))
  {
    char buftmp[64];
    char buftmp2[64];
    element tmp;
#if usekey
    tmp.key = 0;
#endif
    Delete(&tmp);
    if (tmp.xname[0] == '@')
    {
      sprintf(buftmp, "%s/%c/%s", bufini, tmp.xname[0], tmp.xname);
      sprintf(buftmp2, "%s/%c/%s.tmp", bufini, tmp.xname[0], tmp.xname);
    }
    else
    {
      sprintf(buftmp, "%s/%c/%s", bufini, tmp.xname[7], tmp.xname);
      sprintf(buftmp2, "%s/%c/%s.tmp", bufini, tmp.xname[7], tmp.xname);
    }
    rename(buftmp2,buftmp);
  }
  closedir(dirp);

  if (isopen)
  {
    printf("\n\n\nthe lucky number is %d \n\n\n",lora);
    getchar();
  }
  return 0;
}
