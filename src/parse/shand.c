/*
 * $Id: shand.c,v 1.8 2012/04/04 08:08:20 nagatou Exp $
 */
//static char rcsid[]="@(#)$Id: shand.c,v 1.8 2012/04/04 08:08:20 nagatou Exp $";
//static char copyright[]=
//   "Copyright (C) 1998 Naoyuki Nagatou";

/*****************************************************
   this file is a set of functions for symbol table
  handler.
                   Copyright (C) 1998 Naoyuki Nagatou
 *****************************************************/
#define EXTERN extern
#define SHAND
#include "comm.h"

/*****************************************************
 *  isletter
 *---------------------------------------------------*
 *  ret = isletter(chr);                             *
 *                                                   *
 *  bool ret --- if a argment is in letters,return to*
 *              TRUE. otherwise FALSE.               *
 *  char chr --- charactor.                          *
 *****************************************************/
bool isletters(char chr)
{
#  ifdef DEBUG
   printf("isletters-> ");
#  endif
   if (isalpha(chr) ||
       (chr=='_'))
      return(TRUE);
   else
      return(FALSE);
}

/*****************************************************
 *  isdigits
 *---------------------------------------------------*
 *  ret = isdigits(chr);                             *
 *                                                   *
 *  bool ret --- if a argment is in digits,return to *
 *              TRUE. otherwise FALSE.               *
 *  char chr --- charactor.                          *
 *****************************************************/
bool isdigits(char chr)
{
#  ifdef DEBUG
   printf("isdigits-> ");
#  endif
   return(isdigit(chr));
}
/*****************************************************
 *  n_iswspace
 *---------------------------------------------------*
 *  ret = n_iswspace(chr);                             *
 *                                                   *
 *  bool ret --- if a argment is in whitespace,      *
 *              return to TRUE. otherwise FALSE.     *
 *  char chr --- charactor.                          *
 *****************************************************/
bool n_iswspace(char chr)
{
   return(isspace(chr));
}

/*****************************************************
 *  is the key in the set?
 *---------------------------------------------------*
 *  ret = isin(set,key,potency,size);                *
 *                                                   *
 *  bool ret --- if a argment is in keyword,return to*
 *              TRUE. otherwise FALSE.               *
 *  void *set --- pointer to a set                   *
 *  void key --- pointer to a element of set         *
 *  size_t potency --- potency of a first argment    *
 *  size_t size --- size of seccond argment objects  *
 *****************************************************/
bool isin(char *set[],char *key,size_t potency)
{
   bool truth=(bool)FALSE;
   size_t cnt=0;

#  ifdef DEBUG_SCAN
   printf("isin-> ");
   printf("key=%s, ",key);
#  endif
   for(;cnt<potency;cnt++){
      if (strcmp(set[cnt],key)==0){
         truth = TRUE;
         break;
      }
      else
         truth = (bool)FALSE;
   }
#  ifdef DEBUG_SCAN
   printf("ret=%d\n",truth);
#  endif
   return(truth);
}

/*****************************************************
 *  iskeyword
 *---------------------------------------------------*
 *  ret = iskeyword(chr);                            *
 *                                                   *
 *  bool ret --- if a argment is in keyword,return to*
 *              TRUE. otherwise FALSE.               *
 *  char chr --- charactor.                          *
 *****************************************************/
bool iskeyword(char *key)
{
#  ifdef DEBUG
   printf("iskeyword-> ");
#  endif
   return(isin(nccskwd,key,SYM_SIZE));
}

/*****************************************************
 *  hash function                                    *
 *---------------------------------------------------*
 *  ret = hash(string);                              *
 *                                                   *
 *  int ret --- hash value                           *
 *  char *string --- strings                         *
 *****************************************************/
static unsigned int hash1(char *strings)
{
   unsigned int alpha=65599;

#  ifdef DEBUG
   printf("hash1(%d)-> ",);
#  endif
   if (*strings=='\x00')
      return(HASH_TBL_SIZE);
   else
      return(alpha * (hash1(strings+1)) + (unsigned int)(*strings));
}
int hash(char *strings)
{
   return(hash1(strings) % (unsigned int)HASH_TBL_SIZE);
}

/*****************************************************
 *  registe a spell to the spell table            *
 *---------------------------------------------------*
 *  ret = regspl(lexeme);                            *
 *                                                   *
 *  splp ret --- pointer to top of registerring     *
 *               spell                               *
 *  splp lexeme --- pointer to token                *
 *****************************************************/
static splp regspl(splp lexeme)
{
   char *new_pt=NIL;

#  ifdef DEBUG
   printf("regspl-> ");
   printf("\ntop=%x,btm=%x,pt=%x,cnt=%d\n",
          spl_tbl.top,
          spl_tbl.bottom,
          spl_tbl.point,
          spl_tbl.cunt);
#  endif
   if ((spl_tbl.point)+strlen(lexeme) >= spl_tbl.bottom)
      return((splp)error(FATAL,"spell table is full(regspl163) %s\n", lexeme));
   else{
      new_pt = strncpy(spl_tbl.point,lexeme,strlen(lexeme));
      if (new_pt != spl_tbl.point)
         return((splp)error(FATAL,"yield contradiction in the spell table(regspl167) %s\n", lexeme));
      spl_tbl.point = new_pt+strlen(lexeme);
      *(spl_tbl.point) = '\0';
      spl_tbl.point++;
      spl_tbl.cunt++;
      return(new_pt);
   }
}

/*****************************************************
 *  Initialize a symbol table.
 *---------------------------------------------------*
 *  void init_sym_tbl(table);
 *                                                   *
 *  SYNTBL *table --- a poninter to a symbol table
 *****************************************************/
void init_sym_tbl(SYMTBL *table)
{
   table->area=((symbol *)create_heap_area(sizeof(symbol)*SYM_TBL_SIZE));
   table->next = 0;
   if (table->area == (symbol *)NIL)
      error(FATAL,"Don't allocate(init_sym_tbl193)\n");
   bzero(table->area,sizeof(symbol)*SYM_TBL_SIZE);
}
/*****************************************************
 *  construct symbol.                                *
 *  the symbol table is only one.                    *
 *---------------------------------------------------*
 *  ret=makesym(type,ty,...);                        *
 *                                                   *
 *  token ret --- the returned value:succ non zero   *
 *                                :fail ERROR        *
 *  int type --- element type code(show "scan.h")    *
 *  int ty --- field type code(show "scan.h")        *
 *  ety -- arity                                     *
 *****************************************************/
static char * obtain_ptrto_keywd(char *set[],char *key,size_t potency)
{
   char *keywd=(char*)NIL;
   size_t cnt=0;

#  ifdef DEBUG_SCAN
   printf("obtain_prtto_keywd-> ");
#  endif
   for(;cnt<potency;cnt++){
      if (strcmp(set[cnt],key)==0){
         keywd = set[cnt];
         break;
      }
      else
         keywd = set[cnt];
   }
   return(keywd);
}
static token mksym(void)
{
   token new_sym=NIL;

   if ((symbol_table.area==(token)NIL)||(symbol_table.next>=SYM_TBL_SIZE))
      return((token)error(FATAL,"use up the symbol area(mksym219)\n"));
   else{
      new_sym=symbol_table.area+symbol_table.next;
      ++symbol_table.next;
      new_sym->token_name = -1;
      return(new_sym);
   }
}
token makesym(token_t name_type,...)
{
   token new_sym=(token)NIL;
   va_list ap;
   field_t fld_type;
   splp sval;

#  ifdef DEBUG
   printf("makesym-> ");
   printf("name_type=%d\n",name_type);
#  endif
   va_start(ap,name_type);
   switch(name_type){
      case ID:
         new_sym=mksym();
         new_sym->token_name = name_type;
         new_sym->attr.id.spl_ptr = regspl(va_arg(ap,splp));
         break;
      case KEY_WORD:
         new_sym=mksym();
         new_sym->token_name = name_type;
         sval = va_arg(ap,splp);
         new_sym->attr.keywd.str=obtain_ptrto_keywd(nccskwd,sval,SYM_SIZE);
         break;
      case VALUE:
         new_sym=mksym();
         new_sym->token_name = name_type;
         fld_type = va_arg(ap,field_t);
         new_sym->attr.value.type = fld_type; 
         switch(fld_type){
            case ICONST:
               new_sym->attr.value.fld.iconst.int_v = atoi(va_arg(ap,splp));
               break;
            case STR:
               sval = va_arg(ap,splp);
               if (STR_LEN < strlen(sval))
                  return(error(FATAL,"invalid strings(makesym275) %s\n", sval));
               else
                  strcpy(new_sym->attr.value.fld.strings.str,sval);
               break;
            default:
               return((token)error(FATAL,"invalid field type(makesym280) %d\n", fld_type));
         }
         break;
      case PARENTHE:
      case BRACE:
      case BRACKET:
         new_sym=mksym();
         new_sym->token_name = name_type;
         new_sym->attr.par.fr_or_af = va_arg(ap,int);
         break;
      case AGENT_OP:
      case COMP_OP:
      case BOOL_OP:
      case VALUE_OP:
         new_sym=mksym();
         new_sym->token_name = name_type;
         fld_type = new_sym->attr.op.type = va_arg(ap,field_t);
         switch(fld_type){
            /*** unary ***/
            case CO:   /*** agent of ***/
            case PRE:  /*** agent of ***/
            case RES:  /*** agent of ***/
            case REL:  /*** agent of ***/
            case CON:  /*** agent of ***/
            case REC:  /*** agent of ***/
            case NOT:  /*** boolean op ***/
            case DEF:  /*** agent of ***/
            case DEFINIT:  /*** agent of ***/
            case IF:   /*** agent of ***/
            case BIND: /*** agent of ***/
            case RECV: /*** agent of ***/
            case SEND: /*** agent of ***/
               new_sym->attr.op.number = UNARY;
               break;
            /*** binary ***/
            case SUM:  /*** agent of ***/
            case COM:  /*** agent of ***/
            case EQ:   /*** comp op ***/
            case GR:   /*** comp op ***/
            case GT:   /*** comp op ***/
            case LE:   /*** comp op ***/
            case LT:   /*** comp op ***/
            case OR:   /*** boolean op ***/
            case AND:  /*** boolean op ***/
            case PLUS: /*** value op ***/
            case MINS: /*** value op ***/
            case MULT: /*** value op ***/
            case DIV: /*** value op ***/
            case MOD:  /*** value op ***/
            case SEQ:  /*** value op ***/
               new_sym->attr.op.number = BINARY;
               break;
            default:
               return((token)error(FATAL,"invalid field type(makesym332) %d\n", fld_type));
         }
         break;
      default:
         return((token)error(FATAL,"invalid name type(makesym336) %d\n", name_type));
   }
   va_end(ap);
   return(new_sym);
}

/*****************************************************
 *  registe token to the symbol table                 *
 *---------------------------------------------------*
 *  ret = regsym(lexeme,type,type...);               *
 *                                                   *
 *  token ret --- pointer to cell of registerring    *
 *               cell                                *
 *  char *lexeme --- pointer to token                *
 *  int name_type --- name type code                 *
 *  int field_type --- field type code               *
 *****************************************************/
static hash_entry_t * ismember_sym(hash_entry_t *ls,splp lexeme,token_t name,...)
{
   va_list ap;
#  ifdef DEBUG_SHAND
   printf("ismember_sym->");
#  endif
   va_start(ap,name);
   if (ls==(hash_entry_t *)NIL){
      va_end(ap);
      return((hash_entry_t *)NIL);
   }
   else{
      switch((ls->sym_p)->token_name){
         case ID:{
            if ((ls->sym_p)->token_name==name){
               if (strcmp((ls->sym_p)->attr.id.spl_ptr,lexeme)==0)
                  return(ls);
               else{
                  switch(name){
                     case ID:
                        return(ismember_sym(ls->next,lexeme,name));
                        break;
                     default:
                        return((hash_entry_t *)error(FATAL,"invalid name type(ismember_sym380) %d\n",name));
                  }
               }
            }
            else{
               switch(name){
                  case KEY_WORD:
                     return(ismember_sym(ls->next,lexeme,name));
                     break;
                  case VALUE:
                  case PARENTHE:
                  case BRACE:
                  case BRACKET:
                  case AGENT_OP:
                  case COMP_OP:
                  case BOOL_OP:
                  case VALUE_OP:{
                     field_t fld_type=va_arg(ap,field_t);
                     va_end(ap);
                     return(ismember_sym(ls->next,lexeme,name,fld_type));
                     break;
                  }
                  default:
                     return((hash_entry_t *)error(FATAL,"invalid name type(ismember_sym394) %d\n",name));
               }
            }
            break;
         }
         case KEY_WORD:
            if ((ls->sym_p)->token_name==name){
               if (strcmp((ls->sym_p)->attr.keywd.str,lexeme)==0)
                  return(ls);
               else{
                  switch(name){
                     case KEY_WORD:
                        return(ismember_sym(ls->next,lexeme,name));
                        break;
                     default:
                        return((hash_entry_t *)error(FATAL,"invalid name type(ismember_sym379) %d\n",name));
                  }
               }
            }
            else{
               switch(name){
                  case ID:
                     return(ismember_sym(ls->next,lexeme,name));
                     break;
                  case VALUE:
                  case PARENTHE:
                  case BRACE:
                  case BRACKET:
                  case AGENT_OP:
                  case COMP_OP:
                  case BOOL_OP:
                  case VALUE_OP:
                     return(ismember_sym(ls->next,lexeme,name,va_arg(ap,field_t)));
                     break;
                  default:
                     return((hash_entry_t *)error(FATAL,"invalid name type(ismember_sym425) %d\n",name));
               }
            }
            break;
         case VALUE:{
            if ((ls->sym_p)->token_name==name){
               field_t fld_type=va_arg(ap,field_t);
               switch(ls->sym_p->attr.value.type){
                  case ICONST:
                     if (ls->sym_p->attr.value.type==fld_type){
                        if (atoi(lexeme)==ls->sym_p->attr.value.fld.iconst.int_v)
                           return(ls);
                        else{
                           switch(name){
                              case VALUE:
                                 return(ismember_sym(ls->next,lexeme,name,fld_type));
                                 break;
                              default:
                                 return((hash_entry_t *)error(FATAL,"invalid name type(ismember_sym447) %d\n",name));
                           }
                        }
                     }
                     else{
                        switch(name){
                           case VALUE:
                              return(ismember_sym(ls->next,lexeme,name,fld_type));
                              break;
                           default:
                              return((hash_entry_t *)error(FATAL,"invalid name type(ismember_sym461) %d\n",name));
                        }
                     }
                     break;
                  case STR:
                     if (ls->sym_p->attr.value.type==fld_type){
                        if (strcmp((ls->sym_p)->attr.value.fld.strings.str,lexeme)==0)
                           return(ls);
                        else{
                           switch(name){
                              case VALUE:
                                 return(ismember_sym(ls->next,lexeme,name,fld_type));
                                 break;
                              default:
                                 return((hash_entry_t *)error(FATAL,"invalid name type(ismember_sym479) %d\n",name));
                           }
                        }
                     }
                     else{
                        switch(name){
                           case VALUE:
                              return(ismember_sym(ls->next,lexeme,name,fld_type));
                              break;
                           default:
                              return((hash_entry_t *)error(FATAL,"invalid name type(ismember_sym493) %d\n",name));
                        }
                     }
                     break;
                  default:
                     return((hash_entry_t *)error(FATAL,"do not support a field type(ismember_sym401) %d\n",(ls->sym_p)->token_name));
               }
            }
            else{
               switch(name){
                  case ID:
                  case KEY_WORD:
                     return(ismember_sym(ls->next,lexeme,name));
                     break;
                  case VALUE:
                  case PARENTHE:
                  case BRACE:
                  case BRACKET:
                  case AGENT_OP:
                  case COMP_OP:
                  case BOOL_OP:
                  case VALUE_OP:{
                     field_t fld_type=va_arg(ap,field_t);
                     return(ismember_sym(ls->next,lexeme,name,fld_type));
                     break;
                  }
                  default:
                     return((hash_entry_t *)error(FATAL,"invalid name type(ismember_sym528) %d\n",name));
               }
            }
         }
            break;
         case PARENTHE:
         case BRACE:
         case BRACKET:{
            field_t fld_type=va_arg(ap,field_t);
            if ((ls->sym_p)->token_name==name){
               if ((ls->sym_p->attr.par.fr_or_af)==fld_type)
                  return(ls);
               else
                  return(ismember_sym(ls->next,lexeme,name,fld_type));
            }
            else
               switch(name){
                  case ID:
                  case KEY_WORD:
                     return(ismember_sym(ls->next,lexeme,name));
                     break;
                  case VALUE:
                  case PARENTHE:
                  case BRACE:
                  case BRACKET:
                  case AGENT_OP:
                  case COMP_OP:
                  case BOOL_OP:
                  case VALUE_OP:{
                     return(ismember_sym(ls->next,lexeme,name,fld_type));
                     break;
                  }
                  default:
                     return((hash_entry_t *)error(FATAL,"invalid name type(ismember_sym578) %d\n",name));
               }
            break;
         }
         case AGENT_OP:
         case COMP_OP:
         case BOOL_OP:
         case VALUE_OP:{
            field_t fld_type=va_arg(ap,field_t);
            if ((ls->sym_p)->token_name==name){
               if ((ls->sym_p)->attr.op.type==fld_type)
                  return(ls);
               else
                  return(ismember_sym(ls->next,lexeme,name,fld_type));
            }
            else{
               switch(name){
                  case ID:
                  case KEY_WORD:
                     return(ismember_sym(ls->next,lexeme,name));
                     break;
                  case VALUE:
                  case PARENTHE:
                  case BRACE:
                  case BRACKET:
                  case AGENT_OP:
                  case COMP_OP:
                  case BOOL_OP:
                  case VALUE_OP:{
                     return(ismember_sym(ls->next,lexeme,name,fld_type));
                     break;
                  }
                  default:
                     return((hash_entry_t *)error(FATAL,"invalid name type(ismember_sym578) %d\n",name));
               }
            }
            break;
         }
         default:
            return((hash_entry_t *)error(FATAL,"do not support a name type(ismember_sym404) %d\n",(ls->sym_p)->token_name));
      }
   }
}
static hash_entry_t * makeLS_sym(splp lexeme,token_t name,...)
{
   hash_entry_t * new_LS=NIL;
   va_list ap;

#  ifdef DEBUG_SHAND
   printf("makeLS_sym->");
#  endif
   va_start(ap,name);
   if ((new_LS=(hash_entry_t *)(malloc(sizeof(hash_entry_t))))==NIL){
      va_end(ap);
      return((hash_entry_t *)error(FATAL,"allocation error(makeLS_sym391)\n"));
   }
   else{
      switch(name){
         case ID:
         case KEY_WORD:
            new_LS->sym_p = makesym(name,lexeme);
            new_LS->next = (hash_entry_t *)NIL;
            break;
         case VALUE:
            new_LS->sym_p = makesym(name,va_arg(ap,int),lexeme);
            new_LS->next = (hash_entry_t *)NIL;
            break;
         case PARENTHE:
         case BRACE:
         case BRACKET:
            new_LS->sym_p = makesym(name,va_arg(ap,int));
            new_LS->next = (hash_entry_t *)NIL;
            break;
         case AGENT_OP:
         case COMP_OP:
         case BOOL_OP:
         case VALUE_OP:
            new_LS->sym_p = makesym(name,va_arg(ap,int));
            new_LS->next = (hash_entry_t *)NIL;
            break;
         default:
            va_end(ap);
            return((hash_entry_t *)error(FATAL,"do not support in this function(makeLS_sym406)\n"));
      }
   }
   va_end(ap);
   return(new_LS);
}
static hash_entry_t * apdLS1_sym(hash_entry_t * crrt,hash_entry_t * pre,splp lexeme,token_t name,...)
{
   field_t fld_type=-1;
   va_list ap;

#  ifdef DEBUG_SHAND
   printf("apdLS1_sym->");
#  endif
   va_start(ap,name);
   if (pre==NIL)
      return((hash_entry_t *)error(FATAL,"Segmentation fault(apdLS1_sym394)."));
   if (crrt==NIL){
      switch(name){
         case ID:
         case KEY_WORD:
            return(pre->next = makeLS_sym(lexeme,name));
            break;
         case VALUE:
            fld_type = va_arg(ap,field_t);
            return(pre->next = makeLS_sym(lexeme,name,fld_type));
            break;
         case PARENTHE:
         case BRACE:
         case BRACKET:
            return(pre->next = makeLS_sym(lexeme,name,va_arg(ap,int)));
            break;
         case AGENT_OP:
         case COMP_OP:
         case BOOL_OP:
         case VALUE_OP:
            return(pre->next = makeLS_sym(lexeme,name,va_arg(ap,int)));
            break;
         default:
            va_end(ap);
            return((hash_entry_t *)error(FATAL,"do not support in this function(apdLS1_sym438)\n"));
      }
   }
   else{
      switch(name){
         case ID:
         case KEY_WORD:
            return(apdLS1_sym(crrt->next,crrt,lexeme,name));
            break;
         case VALUE:
            fld_type = va_arg(ap,field_t);
            return(apdLS1_sym(crrt->next,crrt,lexeme,name,fld_type));
            break;
         case PARENTHE:
         case BRACE:
         case BRACKET:
            return(apdLS1_sym(crrt->next,crrt,lexeme,name,va_arg(ap,int)));
            break;
         case AGENT_OP:
         case COMP_OP:
         case BOOL_OP:
         case VALUE_OP:
            return(apdLS1_sym(crrt->next,crrt,lexeme,name,va_arg(ap,int)));
            break;
         default:
            va_end(ap);
            return((hash_entry_t *)error(FATAL,"do not support in this function(apdLS1_sym438)\n"));
      }
   }
}
static hash_entry_t * apdLS_sym(hash_entry_t **top,splp lexeme,token_t name,...)
{
   field_t fld_type=-1;
   va_list ap;

#  ifdef DEBUG_SHAND
   printf("apdLS_sym->");
#  endif
   va_start(ap,name);
   if ((*top)==(hash_entry_t *)NIL){
      switch(name){
         case ID:
         case KEY_WORD:
            return((*top)=makeLS_sym(lexeme,name));
         case VALUE:
            fld_type = va_arg(ap,field_t);
            return((*top)=makeLS_sym(lexeme,name,fld_type));
            break;
         case PARENTHE:
         case BRACE:
         case BRACKET:
            return((*top)=makeLS_sym(lexeme,name,va_arg(ap,int)));
            break;
         case AGENT_OP:
         case COMP_OP:
         case BOOL_OP:
         case VALUE_OP:
            return((*top)=makeLS_sym(lexeme,name,va_arg(ap,int)));
            break;
         default:
            va_end(ap);
            return((hash_entry_t *)error(FATAL,"do not support the type(apdLS_sym707) %d\n",name));
      }
   }
   else{
      switch(name){
         case ID:
         case KEY_WORD:
            return(apdLS1_sym((*top)->next,(*top),lexeme,name));
            break;
         case VALUE:
            fld_type = va_arg(ap,field_t);
            return(apdLS1_sym((*top)->next,(*top),lexeme,name,fld_type));
            break;
         case PARENTHE:
         case BRACE:
         case BRACKET:
            return(apdLS1_sym((*top)->next,(*top),lexeme,name,va_arg(ap,int)));
            break;
         case AGENT_OP:
         case COMP_OP:
         case BOOL_OP:
         case VALUE_OP:
            return(apdLS1_sym((*top)->next,(*top),lexeme,name,va_arg(ap,int)));
            break;
         default:
            va_end(ap);
            return((hash_entry_t *)error(FATAL,"do not support in this function(apdLS_sym465)\n"));
      }
   }
}
token regsym(splp lexeme,token_t name_type,...)
{
   symbol *sym=NIL;
   field_t fld_type;
   va_list ap;
   hash_entry_t * ety=(hash_entry_t *)NIL;

#  ifdef DEBUG
   printf("regsym-> ");
#  endif
   va_start(ap,name_type);
   switch(name_type){
      case ID:
      case KEY_WORD:
         if ((ety=ismember_sym(hash_table[hash(lexeme)],lexeme,name_type))!=(hash_entry_t *)NIL){
            va_end(ap);
            return(ety->sym_p);
         }
         else{
            va_end(ap);
            return((apdLS_sym(&(hash_table[hash(lexeme)]),lexeme,name_type))->sym_p);
         }
         break;
      case VALUE:
         fld_type = va_arg(ap,field_t);
         if ((ety=ismember_sym(hash_table[hash(lexeme)],lexeme,name_type,fld_type))!=(hash_entry_t *)NIL){
            va_end(ap);
            return(ety->sym_p);
         }
         else{
            va_end(ap);
            return((apdLS_sym(&(hash_table[hash(lexeme)]),lexeme,name_type,fld_type))->sym_p);
         }
         break;
      case PARENTHE:
      case BRACE:
      case BRACKET:
         sym = makesym(name_type,va_arg(ap,int));
         break;
      case AGENT_OP:
      case COMP_OP:
      case BOOL_OP:
      case VALUE_OP:{
         fld_type = va_arg(ap,field_t);
         if ((ety=ismember_sym(hash_table[hash(lexeme)],lexeme,name_type,fld_type))!=(hash_entry_t *)NIL){
            va_end(ap);
            return(ety->sym_p);
         }
         else{
            va_end(ap);
            return((apdLS_sym(&(hash_table[hash(lexeme)]),lexeme,name_type,fld_type))->sym_p);
         }
         break;
      }
      default:
         return((token)error(FATAL,"invalid name type(regsym582) %d\n", name_type));
   }
   va_end(ap);
   return(sym);
}

/*****************************************************
 *  initialize the spell table                       *
 *---------------------------------------------------*
 *  void initspltbl(void);                           *
 *                                                   *
 *****************************************************/
void initspltbl(spell_table *tbl)
{
#  ifdef DEBUG_EVAL
   printf("spell=%08x ",(unsigned int)tbl->spell);
   printf("initspltbl-> ");
#  endif
   tbl->top = tbl->spell;
   tbl->bottom = tbl->spell + SPL_TBL_SIZE;
   tbl->point = tbl->spell;
   tbl->cunt = 0;
}

/*****************************************************
 *  print a symbol                                   *
 *---------------------------------------------------*
 *  void prtsym(sym);                                *
 *                                                   *
 *  token sym --- pointer to a symbol                *
 *****************************************************/
void prtsym(int fun,token sym,...)
{
   va_list argp;
   buffer *buf=(buffer *)NIL;

#  ifdef DEBUG
   printf("prtsym-> ");
#  endif
   va_start(argp,sym);
   if (fun==BUF){
      buf = va_arg(argp,buffer *);
      if (buf == (buffer *)NIL){
         va_end(argp);
         error(FATAL,"Segmentation fault(prtsym418) (buf==NIL).\n");
      }
   }
   switch(sym->token_name){
      case ID:
         if (fun==BUF)
            ins_buf(buf,sym->attr.id.spl_ptr);
         else
            printf("%s",sym->attr.id.spl_ptr);
         break;
      case VALUE:
         switch(sym->attr.value.type){
            case ICONST:
               if (fun==BUF)
                  ins_buf(buf,cvtia(sym->attr.value.fld.iconst.int_v));
               else
                  printf("%d",sym->attr.value.fld.iconst.int_v);
               break;
            case STR:
               if (fun==BUF){
                  ins_buf(buf,"\"");
                  ins_buf(buf, sym->attr.value.fld.strings.str);
                  ins_buf(buf,"\"");
               }
               else
                  printf("\"%s\"", sym->attr.value.fld.strings.str);
               break;
            default:
               error(FATAL,"invalid field type(prtsym446) %d\n", sym->attr.value.type);
         }
         break;
      case PARENTHE:
         if (sym->attr.par.fr_or_af == FR)
            printf("token_name=\"PARENTHE (\" type=\"FR\" \n");
         else
            printf("token_name=\"PARENTHE )\" type=\"AF\" \n");
         break;
      case BRACE:
         if (sym->attr.par.fr_or_af == FR)
            printf("token_name=\"BREACE {\" type=\"FR\" \n");
         else
            printf("token_name=\"BREACE }\" type=\"AF\" \n");
         break;
      case BRACKET:
         if (sym->attr.par.fr_or_af == FR)
            printf("token_name=\"BRACKET [\" type=\"FR\" \n");
         else
            printf("token_name=\"BRACKET ]\" type=\"AF\" \n");
         break;
      case AGENT_OP:
         switch(sym->attr.op.type){
            case PRE:
               if (fun==BUF)
                  ins_buf(buf,":");
               else
                  printf("PRE");
               break;
            case SUM:
               if (fun==BUF)
                  ins_buf(buf,"++");
               else
                  printf("SUM");
               break;
            case COM:
               if (fun==BUF)
                  ins_buf(buf,"||");
               else
                  printf("COM");
               break;
            case RES:
               if (fun==PRN)
                  printf("RES");
               break;
            case REL:
               if (fun==PRN)
                  printf("REL");
               break;
            case CON:
               if (fun==PRN)
                  printf("CON");
               break;
            case REC:
               if (fun==PRN)
                  printf("REC");
               break;
            case CO:
               if (fun==PRN)
                  printf("CO");
               break;
            case IF:
               if (fun==BUF)
                  ins_buf(buf,"if");
               else
                  printf("IF");
               break;
            case DEF:
               if (fun==BUF)
                  ins_buf(buf,"define");
               else
                  printf("DEF");
               break;
            case DEFINIT:
               if (fun==BUF)
                  ins_buf(buf,"definit");
               else
                  printf("DEFINIT");
               break;
            case BIND:
               if (fun==BUF)
                  ins_buf(buf,"bind");
               else
                  printf("BIND");
               break;
            case SEND:
               if (fun==BUF)
                  ins_buf(buf,"~");
               else
                  printf("SEND");
               break;
            case RECV:
               if (fun==PRN)
                  printf("RECV");
               break;
            default:
               error(FATAL,"invalid field type(prtsym536) %d\n", sym->attr.op.type);
         }
         break;
      case COMP_OP:
         switch(sym->attr.op.type){
            case EQ:
               if (fun==BUF)
                  ins_buf(buf,"=");
               else
                  printf("=");
               break;
            case GR:
               if (fun==BUF)
                  ins_buf(buf,">");
               else
                  printf(">");
               break;
            case GT:
               if (fun==BUF)
                  ins_buf(buf,">=");
               else
                  printf(">=");
               break;
            case LE:
               if (fun==BUF)
                  ins_buf(buf,"<");
               else
                  printf("<");
               break;
            case LT:
               if (fun==BUF)
                  ins_buf(buf,"<=");
               else
                  printf("<=");
               break;
            default:
               error(FATAL,"invalid field type(prtsym572) %d\n", sym->attr.op.type);
         }
         break;
      case BOOL_OP:
         switch(sym->attr.op.type){
            case OR:
               if (fun==BUF)
                  ins_buf(buf,"|");
               else
                  printf("OR");
               break;
            case AND:
               if (fun==BUF)
                  ins_buf(buf,"&");
               else
                  printf("AND");
               break;
            case NOT:
               if (fun==BUF)
                  ins_buf(buf,"!");
               else
                  printf("NOT");
               break;
            default:
               error(FATAL,"invalid field type(prtsym596) %d\n", sym->attr.op.type);
         }
         break;
      case VALUE_OP:
         switch(sym->attr.op.type){
            case PLUS:
               if (fun==BUF)
                  ins_buf(buf,"+");
               else
                  printf("PLUS");
               break;
            case MINS:
               if (fun==BUF)
                  ins_buf(buf,"-");
               else
                  printf("MINS");
               break;
            case MULT:
               if (fun==BUF)
                  ins_buf(buf,"*");
               else
                  printf("MULT");
               break;
            case DIV:
               if (fun==BUF)
                  ins_buf(buf,"/");
               else
                  printf("DIV");
               break;
            case MOD:
               if (fun==BUF)
                  ins_buf(buf,"%");
               else
                  printf("MOD");
               break;
            case SEQ:
               if (fun==PRN)
                  printf("SEQ");
               break;
            default:
               error(FATAL,"invalid field type(prtsym636) %d\n", sym->attr.op.type);
         }
         break;
      case KEY_WORD:
         printf("token_name=\"KEY_WORD\" str=%s\n",sym->attr.keywd.str);
         break;
      default:
         error(FATAL,"invalid name type=%d,%d(prtsym643)\n",sym->token_name,sym->attr.op.type);
   }
   va_end(argp);
}
