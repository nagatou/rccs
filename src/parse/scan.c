/*
 * $Id: scan.c,v 1.2 2015/02/16 07:08:19 nagatou Exp $
 */
//static char rcsid[]="@(#)$Id: scan.c,v 1.2 2015/02/16 07:08:19 nagatou Exp $";
//static char copyright[]=
//   "Copyright (C) 1998 Naoyuki Nagatou";

/************************************************************************
   this file is a set of functions for scanner. the following matrix is
  a DFA of nccs.
                                      Copyright (C) 1998 Naoyuki Nagatou
 ************************************************************************/
#define EXTERN extern
#define SCAN
#include "comm.h"

/*****************************************************
 *---------------------------------------------------*
 *  void int_state(lexeme,sym);                      *
 *                                                   *
 *****************************************************/
static void int_state(buffer *lexeme,token *sym)
{
   char chr;

   chr = getnch();
#  ifdef DEBUG_SCAN
   printf("int_state-(\"%c\")-> ",chr);
#  endif
   if (isdigits(chr)){
      lexeme->buf[lexeme->cnt] = chr;
      (lexeme->cnt)++;
      lexeme->buf[lexeme->cnt] = '\0';
      int_state(lexeme,sym);
   }
   else{
      ungetnch(chr);
      *sym = regsym(lexeme->buf,VALUE,ICONST);
      longjmp(env,1);
   }
}

/*****************************************************
 *---------------------------------------------------*
 *  void id_state(lexeme,sym);                       *
 *                                                   *
 *****************************************************/
static void id_state(buffer *lexeme,token *sym)
{
   char chr;

   chr = getnch();
#  ifdef DEBUG_SCAN
   printf("id_state-(\"%c\",%s)-> ",chr,lexeme->buf);
#  endif
   if (isletters(chr) || isdigits(chr)){
      lexeme->buf[lexeme->cnt] = chr;
      (lexeme->cnt)++;
      lexeme->buf[lexeme->cnt] = '\0';
      id_state(lexeme,sym);
   }
   else{
      ungetnch(chr);
      if (iskeyword(lexeme->buf)){
         *sym = regsym(lexeme->buf,KEY_WORD);
#        ifdef DEBUG
         printf("\nscan116\n");
#        endif
         longjmp(env,1);
      }
      else{
         *sym = regsym(lexeme->buf,ID);
         longjmp(env,1);
      }
   }
}

/*****************************************************
 *---------------------------------------------------*
 *  void comment_state(lexeme,sym);                  *
 *                                                   *
 *****************************************************/
static void comment_state(buffer *lexeme,token *sym)
{
   char chr;

   chr = getnch();
#  ifdef DEBUG_SCAN
   printf("comment_state-(\"%c\")-> ",chr);
#  endif
   if (chr == LF)
      start_state(lexeme,sym);
   else
      comment_state(lexeme,sym);
}

/*****************************************************
 *---------------------------------------------------*
 *  void str_state(lexeme,sym);                      *
 *                                                   *
 *****************************************************/
static void str_state(buffer *lexeme,token *sym);
static void str_state1(buffer *lexeme,token *sym)
{
   char chr;

   chr = getnch();
#  ifdef DEBUG_SCAN
   printf("str_state1-(\"%c\")-> ",chr);
#  endif
   switch(chr){
      case 'R':
      case 'r':
         lexeme->buf[lexeme->cnt] = CR;
         (lexeme->cnt)++;
         lexeme->buf[lexeme->cnt] = '\0';
         str_state(lexeme,sym);
         break;
      case 'L':
      case 'l':
         lexeme->buf[lexeme->cnt] = LF;
         (lexeme->cnt)++;
         lexeme->buf[lexeme->cnt] = '\0';
         str_state(lexeme,sym);
         break;
      default:
         lexeme->buf[lexeme->cnt] = '/';              /* B038 */
         (lexeme->cnt)++;                             /* B038 */
         lexeme->buf[lexeme->cnt] = chr;              /* B038 */
         (lexeme->cnt)++;                             /* B038 */
         str_state(lexeme,sym);                       /* B038 */
//         error(WARNING,"syntax error(str_state1181)\n");  /* B038 */
   }
}
static void str_state(buffer *lexeme,token *sym)
{
   char chr;

   chr = getnch();
#  ifdef DEBUG_SCAN
   printf("str_state-(\"%c\")-> ",chr);
#  endif
   if (isdigits(chr)||
       isletters(chr)||
       (chr=='~')||
       (chr==' ')||
       (chr=='.')||
       (chr==',')||
       (chr==':')||
       (chr=='>')||
       (chr=='<')||
       (chr=='_')||
       (chr=='-')){
      lexeme->buf[lexeme->cnt] = chr;
      (lexeme->cnt)++;
      lexeme->buf[lexeme->cnt] = '\0';
      str_state(lexeme,sym);
   }
   else{
      if (chr == '"'){
         *sym = regsym(lexeme->buf,VALUE,STR);
         longjmp(env,1);
      }
      else{
         if (chr == '/')
            str_state1(lexeme,sym);
         else
            error(WARNING,"syntax error(str_state168) %c\n", chr);
      }
   }
}

/*****************************************************
 *---------------------------------------------------*
 *  void start_state(lexeme,sym);                    *
 *                                                   *
 *****************************************************/
static void start_state(buffer *lexeme,token *sym)
{
   char chr;
   char lookahead;

   chr = getnch();
#  ifdef DEBUG_SCAN
   printf("start_state-(\"%c\")-> ",chr);
#  endif
   if (isletters(chr)){
      lexeme->buf[lexeme->cnt] = chr;
      (lexeme->cnt)++;
      lexeme->buf[lexeme->cnt] = '\0';
      id_state(lexeme,sym);
   }
   if (isdigits(chr)){
      lexeme->buf[lexeme->cnt] = chr;
      (lexeme->cnt)++;
      lexeme->buf[lexeme->cnt] = '\0';
      int_state(lexeme,sym);
   }
   if (n_iswspace(chr))
      start_state(lexeme,sym);
#  ifdef DEBUG
   printf("others-(\"%c\")-> ",chr);
#  endif
   if (chr == '"')
      str_state(lexeme,sym);
   if (chr == ';')
      comment_state(lexeme,sym);
   switch(chr){
      case '(':
         *sym = regsym(lexeme->buf,PARENTHE,FR);
         longjmp(env,1);
         break;
      case ')':
         *sym = regsym(lexeme->buf,PARENTHE,AF);
         longjmp(env,1);
         break;
      case '[':
         *sym = regsym(lexeme->buf,BRACKET,FR);
         longjmp(env,1);
         break;
      case ']':
         *sym = regsym(lexeme->buf,BRACKET,AF);
         longjmp(env,1);
         break;
      case '{':
         *sym = regsym(lexeme->buf,BRACE,FR);
         longjmp(env,1);
         break;
      case '}':
         *sym = regsym(lexeme->buf,BRACE,AF);
         longjmp(env,1);
         break;
      case ',':
         *sym = regsym(lexeme->buf,VALUE_OP,SEQ);
         longjmp(env,1);
         break;
      case '+':
         lookahead = getnch();
         if (lookahead == '+'){
            *sym = regsym(lexeme->buf,AGENT_OP,SUM);
            longjmp(env,1);
         }
         else{
            ungetnch(lookahead);
            *sym = regsym(lexeme->buf,VALUE_OP,PLUS);
            longjmp(env,1);
         }
         break;
      case '-':
         *sym = regsym(lexeme->buf,VALUE_OP,MINS);
         longjmp(env,1);
         break;
      case '*':
         *sym = regsym(lexeme->buf,VALUE_OP,MULT);
         longjmp(env,1);
         break;
      case '/':
         *sym = regsym(lexeme->buf,VALUE_OP,DIV);
         longjmp(env,1);
         break;
      case '%':
         *sym = regsym(lexeme->buf,VALUE_OP,MOD);
         longjmp(env,1);
         break;
      case ':':
         *sym = regsym(lexeme->buf,AGENT_OP,PRE);
         longjmp(env,1);
         break;
      case '~':
         *sym = regsym(lexeme->buf,AGENT_OP,CO);
         longjmp(env,1);
         break;
      case '|':
         lookahead = getnch();
         if (lookahead == '|'){
            *sym = regsym(lexeme->buf,AGENT_OP,COM);
            longjmp(env,1);
         }
         else{
            ungetnch(lookahead);
            *sym = regsym(lexeme->buf,BOOL_OP,OR);
            longjmp(env,1);
         }
         break;
      case '&':
         *sym = regsym(lexeme->buf,BOOL_OP,AND);
         longjmp(env,1);
         break;
      case '!':
         *sym = regsym(lexeme->buf,BOOL_OP,NOT);
         longjmp(env,1);
         break;
      case '<':
         lookahead = getnch();
         if (lookahead == '='){
            *sym = regsym(lexeme->buf,COMP_OP,LT);
            longjmp(env,1);
         }
         else{
            ungetnch(lookahead);
            *sym = regsym(lexeme->buf,COMP_OP,LE);
            longjmp(env,1);
         }
         break;
      case '>':
         lookahead = getnch();
         if (lookahead == '='){
            *sym = regsym(lexeme->buf,COMP_OP,GT);
            longjmp(env,1);
         }
         else{
            ungetnch(lookahead);
            *sym = regsym(lexeme->buf,COMP_OP,GR);
            longjmp(env,1);
         }
         break;
      case '=':
         *sym = regsym(lexeme->buf,COMP_OP,EQ);
         longjmp(env,1);
         break;
      case ERROR:
         start_state(lexeme,sym);
         break;
      case EOF:
         longjmp(env,2);
         break;
      default:
         error(WARNING,"syntax error(%c)(start_state326)\n",chr);
   } /*** end of switch ***/
} /*** end of function ***/

/*****************************************************
 *---------------------------------------------------*
 *  token = scanner(sym);                            *
 *                                                   *
 *****************************************************/
token scanner(token *sym)
{
   int ret;
   buffer lexeme; /***/

#  ifdef DEBUG_SCAN
   printf("scanner-> ");
#  endif
   initbuf(&lexeme);
   if ((ret=setjmp(env))==0){
      start_state(&lexeme,sym);
   }
   else{
      if (ret == 1){
         return(*sym);
      }
      return((token)NIL);
   }
   return((token)NIL);
}
