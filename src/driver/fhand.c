/*
 * $Id: fhand.c,v 1.2 2015/02/16 07:08:13 nagatou Exp $
 */
//static char rcsid[]="@(#)$Id: fhand.c,v 1.2 2015/02/16 07:08:13 nagatou Exp $";
//static char copyright[]=
//   "Copyright (C) 1998 Naoyuki Nagatou";

/*****************************************************
  this file is set of functions for a file handler.
                   Copyright (C) 1998 Naoyuki Nagatou
 *****************************************************/
#define EXTERN extern
#define FHAND
#include "comm.h"
#include "fhand.h"

/*****************************************************
 *  get a charactor form a source file.              *
 *---------------------------------------------------*
 *  ret = getnch(void);                               *
 *                                                   *
 *  char ret --- get one charctor from source file.  *
 *****************************************************/
static char deque(queue *que)
{
  char ret=(char)ERROR;

#  ifdef DEBUG_QUE
      printf("deque[");
      printf("%x",que->que_buf.buf[que->top]);
      printf("]->");
#  endif
   if (que->que_buf.cnt<=0)
      return((char)ERROR);
   else{
      ret = que->que_buf.buf[que->top];
      if ((que->top)+1 < BUF_SIZE)
         ++(que->top);
      else
         que->top = 0;
      --(que->que_buf.cnt);
      return(ret);
   }
}
char getnch(void)
{
   char chr=deque(&que);

#  ifdef DEBUG_QUE
      printf("\ngetnch->");
#  endif
   if (chr==(char)ERROR){
      if (isempty_buf(&(seed.buf)))
         chr=(char)fgetc(source_file);
      else{
         if (seed.ptr>=seed.buf.cnt){
            chr = '\n';
            clear_in_buf(&seed);
         }
         else{
            chr = seed.buf.buf[seed.ptr];
            seed.ptr++;
         }
      }
      if (chr=='\n')
         chr=LF;      /* B006 */
      return(chr);
   }
   else
      return(chr);
}
/*****************************************************
 *  read one character from standard input.          *
 *  A white space character is ignored.              *
 *  After reading one, the input buffer of NHK is    *
 * refreshed.                                        *
 *---------------------------------------------------*
 *  int n_getc_stdin(void);                          *
 *                                                   *
 *  int ret --- a letter                             *
 *****************************************************/
int n_getc_stdin(void)
{
   int ret;
#  ifdef DEBUG
   printf("n_getc_stdin->");
   fflush(stdout);
#  endif
   initbuf(&in_buf);
   for(ret=fgetc(stdin);!isalpha(ret);)
      ret=fgetc(stdin);
   initbuf(&in_buf);
   return(ret);
}

/*****************************************************
 *  callback lookahead char to input buffer.         *
 *---------------------------------------------------*
 *  void ungetnch(lookahead);                        *
 *                                                   *
 *  char lookahead --- one lookahead symbol          *
 *****************************************************/
static char enque_chr(queue *que,char inchar)
{
#  ifdef DEBUG_QUE
      printf("enque_chr[");
      printf("%s",que->que_buf.buf);
      printf("]->");
#  endif
   if (que->que_buf.cnt >= BUF_SIZE)
      return((char)ERROR);
   else{
      que->que_buf.buf[que->bottom]=inchar;
      if (que->bottom+1 < BUF_SIZE)
         ++que->bottom;
      else
         que->bottom = 0;
      ++(que->que_buf.cnt);
      return(inchar);
   }
}
void ungetnch(char lookahead)
{
#  ifdef DEBUG_QUE
      printf("\nungetnch[");
      printf("%c",lookahead);
      printf("]->");
#  endif
   if (((char)ERROR) == enque_chr(&que,lookahead))
      error(FATAL,"invalid input buffer(ungetnch110)\n");
}

/*****************************************************
 *  initialize file buffer                           *
 *---------------------------------------------------*
 *  void initsf(source,log);                         *
 *                                                   *
 *  char *source --- source file name                *
 *  char *log --- log file name                      *
 *****************************************************/
void initsf(char *source,char *log)
{
#  ifdef DEBUG_EVAL
   printf("initsf->");
#  endif
   source_file = NIL;
   log_file = NIL;
   if (source == NIL){
      source_file = stdin;
      fflush(stdin);
   }
   else{
      source_file = fopen(source,"r");
      if (source_file==NIL)
         error(FATAL,"cann't open (%s)\n",source);
   }
   initbuf(&(que.que_buf));
   initbuf(&in_buf);
   initbuf(&out_buf);
   setvbuf(source_file,in_buf.buf,_IOFBF,(size_t)BUF_SIZE);
   if (log != NIL){
      if (strlen(log)==0)
         log = "LOGFILEnnn";
      log_file = fopen(log,"a");
      if (log_file==NIL)
         error(FATAL,"cann't open (%s)\n",log);
      setvbuf(log_file,out_buf.buf,_IOFBF,(size_t)BUF_SIZE);
   }
   else
      log_file = NIL;
}

/*****************************************************
 *  initialize buffer                                *
 *---------------------------------------------------*
 *  void initbuf(buf);                               *
 *                                                   *
 *  buffer *arg --- pointer to a buffer              *
 *****************************************************/
void initbuf(volatile buffer *arg)
{
   int cnt=0;

#  ifdef DEBUG_BUF
   printf("initbuf->");
#  endif
   arg->cnt = 0;
   for(;cnt<BUF_SIZE;++cnt)
      arg->buf[cnt]='\x00';
}
/*****************************************************
 *  Is a buffer empty?
 *---------------------------------------------------*
 *  ret = isempty_buf(buf);
 *
 *  bool ret --- TRUE : if it is empty
 *               FALSE: otherwise
 *  buffer *arg --- pointer to a buffer 
 *****************************************************/
bool isempty_buf(buffer *buf)
{
   if (buf->cnt==0)
      return(TRUE);
   else
      return(FALSE);
}
/*****************************************************
 *  insert characters to a buffer
 *---------------------------------------------------*
 *  buffer ins_buf(buf);                               *
 *                                                   *
 *  buffer *arg --- pointer to a buffer              *
 *****************************************************/
splp ins_buf(buffer *buf,char *strings)
{
   int i,c;

#  ifdef DEBUG_PRINTa
   printf("ins_buf[%p,%d]->",buf,buf->cnt);
#  endif
   if ((buf==(buffer *)NIL)||(strings==(splp)NIL))
      error(FATAL,"Segmentation fault(ins_buf196)\n");
   for(i=0,c=0;i<strlen(strings);i++){
      if (((buf->cnt)+c)>=BUF_SIZE)
         error(FATAL,"Buffer overflow(ins_buf199) (%d)\n", (buf->cnt)+c);
      else{
         if (*(strings+i)==LF){
            strncpy((buf->buf)+(buf->cnt)+c,"/l",2);
            c=+2;
         }
         else{
            if (*(strings+i)==CR){
               strncpy((buf->buf)+(buf->cnt)+c,"/l",2);
               c=+2;
            }
            else{
               strncpy((buf->buf)+(buf->cnt)+c,strings+i,1);
               c++;
            }
        }
      }
   }
   buf->cnt = buf->cnt + (c);
   return(buf->buf);
}
/*****************************************************
 *  append a character to end of a buffer.
 *---------------------------------------------------*
 *  buffer append_buf(buf,string);                   *
 *                                                   *
 *  buffer *buf --- pointer to a buffer              *
 *  symbol string --- pointer to a symbol            *
 *****************************************************/
splp append_buf(buffer *buf,char chr)
{
#  ifdef DEBUG_BUF
   printf("append_buf[%p,%d]->",buf,buf->cnt);
#  endif
   if (buf==(buffer *)NIL)
      error(FATAL,"Segmentation fault(append_buf20)\n");
   if (((buf->cnt)+1)>=BUF_SIZE)
      error(FATAL,"Buffer overflow(append_buf22)\n");
   else{
      buf->buf[buf->cnt] = chr;
      (buf->cnt)++;
      buf->buf[buf->cnt] = '\0';
   }
   return(buf->buf);
}
/*****************************************************
 *  append a string to end of a buffer.
 *---------------------------------------------------*
 *  buffer append_str_buf(buf,str);                   *
 *                                                   *
 *  buffer *buf --- pointer to a buffer              *
 *  symbol str --- pointer to a symbol            *
 *****************************************************/
splp append_str_buf(buffer *buf,char *str)
{
   int i,c;

#  ifdef DEBUG_BUF
   printf("append_str_buf[%p,%d,%s]->",buf,buf->cnt,str);
#  endif
   if ((buf==(buffer *)NIL)||(str==(splp)NIL))
      error(FATAL,"Segmentation fault(append_str_buf69)\n");
   for(i=0,c=0;i<strlen(str);i++){
      if (((buf->cnt)+c)>=BUF_SIZE-1)
         error(FATAL,"Buffer overflow(append_str_buf72)\n");
      else{
         if (*(str+i)==LF){
           strncpy((buf->buf)+(buf->cnt)+c,"/l",2);
            c=+2;
         }
         else{
            if (*(str+i)==CR){
               strncpy((buf->buf)+(buf->cnt)+c,"/l",2);
               c=+2;
            }
            else{
               strncpy((buf->buf)+(buf->cnt)+c,str+i,1);
               *(buf->buf+(buf->cnt)+c) = *(str+i);
               *(buf->buf+(buf->cnt)+(c+1)) = 0x0000;
               c++;
            }
         }
      }
   }
   buf->cnt = buf->cnt + (c);
   return(buf->buf);
}
/*****************************************************
 *  clear the buffer in_buf for
 * a source file.
 *---------------------------------------------------*
 *  void clear_in_buf(seed);
 *
 *  seed_t *str --- a pointer to the dummy input buffer.
 *****************************************************/
void clear_in_buf(seed_t *seed)
{
   seed->ptr = 0;
   initbuf(&(seed->buf));
}
/*****************************************************
 *  set a string to the buffer in_buf for
 * a source file.
 *---------------------------------------------------*
 *  void set_in_buf(str);
 *
 *  char *str --- a pointer to the string.
 *****************************************************/
void set_in_buf(buffer *buf,char *str)
{
   initbuf(buf);
   ins_buf(buf,str);
#  ifdef DEBUG_DUMMY_BUF
   printf("set_in_buf[%s]->",in_buf.buf);
#  endif   
}
/*****************************************************
 *  
 *---------------------------------------------------*
 *  void end(buf);                               *
 *                                                   *
 *  buffer *arg --- pointer to a buffer              *
 *****************************************************/
void n_end(void)
{
   if (!(source_file==stdin)){
      fclose(source_file);
      source_file = bkup;
      fflush(source_file);
   }
   if (log_file != NIL){
      fflush(log_file); /***/
      fclose(log_file); /***/
   }
}
