/*
 * $Id: ehand.c,v 1.3 2011/07/08 10:46:10 nagatou Exp $
 */
//static char rcsid[]="@(#)$Id: ehand.c,v 1.3 2011/07/08 10:46:10 nagatou Exp $";
//static char copyright[]=
//   "Copyright (C) 1998 Naoyuki Nagatou";

/******************************************************
   this file is a set of functions for event handler.
                  Copyright (C) 1998 Naoyuki Nagatou
 ******************************************************/

#define EXTERN extern
#define EHAND
#include "comm.h"
#define SYSCALLS_DEC
#include "syscall_index.h"

/**************************************************
 * print an error message and exit                *
 *------------------------------------------------*
 * void errexit(format,va_alist);                 *
 *                                                *
 *  char *format -- output format                 *
 *  va_dcl va_alist -- argments                   *
 *************************************************/
static void errexit(char *format,...)
{
   va_list args;
   char *p=format;

   va_start(args,format);
   for(;*p;p++){
      if (*p != '%'){
         printf("%c",*p);
         continue;
      }
      switch(*++p){
         case 'd':
            printf("%d",va_arg(args,int));
            break;
         case 's':
            printf("%s",va_arg(args,char *));
            break;
         default:
            printf("%c",*p);
      }
   }
   va_end(args);
   exit(1);
}

/**************************************************
 * allocate and connect a socket using TCP or UDP *
 *------------------------------------------------*
 * ret = connectsock(host,service,protocol);      *
 *                                                *
 *  int ret --- succ: socket discriptor           *
 *              fail: ERROR                       *
 *  char * host -- name of host to which          *
 *                connection is desired           *
 *  char * service - service associated with the  *
 *                  desired port                  *
 *  char * protocol - name of protocol to use,    *
 *                   TCP or UDP.                  *
 **************************************************/
static int connectsock(char * host,char * service,char * protocol)
{
   struct hostent *phe;
   struct servent *pse;
   struct protoent *ppe;
   struct sockaddr_in sin;
   int s=0;
   int type=0;

#  ifdef DEBUG_EHAND
   printf("connectsock->");
#  endif
   bzero((char *)&sin,sizeof(sin));
   sin.sin_family = AF_INET;
   if ((pse=getservbyname(service,protocol))!=NULL)
      sin.sin_port = htons(ntohs((u_short)pse->s_port)+(u_short)PORTBASE);
   else
      if ((sin.sin_port=htons((short)atoi(service)))==0)
         errexit("can't get \"%s\" service entry(connectsock93)\n",service);
   if ((phe=gethostbyname(host))!=NULL){
      bcopy(phe->h_addr,(char *)&sin.sin_addr,phe->h_length);
   }
   else
      if ((sin.sin_addr.s_addr=inet_addr(host))==INADDR_NONE)
         errexit("can't get \"%s\" host entry(connectsock98)\n",host);
#  ifdef DEBUG_EHAND
   printf("\nIPaddres=%s\n",inet_ntoa(sin.sin_addr));
#  endif
   if ((ppe=getprotobyname(protocol))==NULL)
      errexit("can't get \"%s(connectsock100)\" protocol entry\n",protocol);
   if ((strcmp(protocol,"udp")==0)||(strcmp(protocol,"UDP")==0))
      type = SOCK_DGRAM;
   else{
      if ((strcmp(protocol,"tcp")==0)||(strcmp(protocol,"TCP")==0))
         type = SOCK_STREAM;
      else
         errexit("can't get \"%d\" protocol type(connectsock107)\n",type);
   }
   if ((s=socket(PF_INET,type,ppe->p_proto))<0)
      errexit("can't create socket:\"%s\" (connectsock110)\n",strerror(errno));
//      errexit("can't create socket:\"%s\" (connectsock110)\n",sys_errlist[errno]);
//   setsockopt(s,IPPROT_TCP,TCP_NODELY,(char *)1,sizeof(int));
   if (connect(s,(struct sockaddr *)&sin,sizeof(sin))==SOCKET_ERROR){
      errexit("can't connect to \"%s:%s %s\"(connectsock115) \n",
              host,
              service,
              strerror(errno));
   }
   return(s);
}

/**************************************************
 *  connect to a specified TCP service on         *
 * a specified host.                              *
 *------------------------------------------------*
 *  ret = connectTCP(host,service);               *
 *                                                *
 *  int ret --- a discriptor for a connected socket.*
 *  char *host - name of host to which connection *
 *             is desired.                        *
 *  char *service -- service associated with      *
 *               the desired port.                *
 **************************************************/
static int connectTCP(char *host,char *service)
{
#  ifdef DEBUG_EHAND
   printf("connectTCP->");
#  endif
   return connectsock(host,service,"tcp");
}
static int listenTCP(char *service)
{
   int s,t;
   struct sockaddr_in sin;
   struct protoent *ppe;
   struct servent *pse;
   unsigned int i = sizeof(sin);

#  ifdef DEBUG_EHAND
   printf("listenTCP->");
#  endif
   bzero((char *)&sin,sizeof(sin));
   sin.sin_family = AF_INET;
   sin.sin_addr.s_addr = INADDR_ANY;
   if ((pse=getservbyname(service,"tcp"))==(struct servent *)NULL){
      if ((sin.sin_port=htons((short)atoi(service)))==0)
         errexit("can't get \"%s\" service entry(listenTCP153)\n",service);
   }
   else
      sin.sin_port = htons(ntohs((u_short)pse->s_port)+(u_short)PORTBASE);
   if ((ppe = getprotobyname("tcp"))==0)
      error(FATAL,"can not get \"TCP\" protocol entry\n");
   if ((s=socket(PF_INET,SOCK_STREAM,ppe->p_proto))<0)
      error(FATAL,"can't create socket(listenTCP160)");
   if (bind(s,(struct sockaddr *)&sin,sizeof(sin))<0)
      error(FATAL,"can not bind a socket(listenTCP162)");
   listen(s,BACKLOG);
   if ((t = accept(s,(struct sockaddr *)&sin,&i))<0)
      error(FATAL,"bind(listenTCP165)");
   return(t);
}

/**************************************************
 *  connect to a specified UDP service on         *
 * a specified host.                              *
 *------------------------------------------------*
 *  ret = connectUDP(host,service);               *
 *                                                *
 *  int ret --- a discriptor for a connected socket.*
 *  char *host - name of host to which connection *
 *             is desired.                        *
 *  char *service -- service associated with      *
 *               the desired port.                *
 **************************************************/
//static int connectUDP(char *host,char *service)
//{
//#  ifdef DEBUG_EHAND
//   printf("connectUDP->");
//#  endif
//   return connectsock(host,service,"udp");
//}
/**************************************************
 *  analysis host address.                        *
 *------------------------------------------------*
 *  ret = anly_addr(addr);                        *
 *                                                *
 **************************************************/
static void aly_addr_serv(splp addr,splp servc)
{
#  ifdef DEBUG_EHAND
   printf("aly_addr_serv->");
#  endif
   if (*addr==(char)NIL){
      *servc = (char)NIL;
      servc++;
   }
   else{
      *servc = *addr;
      aly_addr_serv(addr+1,servc+1);
   }
}
static void aly_addr_host(splp addr,splp hsc,splp servc)
{
#  ifdef DEBUG_EHAND
   printf("aly_addr_host->");
#  endif
   if (*addr==':'){
      *hsc = (char)NIL;
      aly_addr_serv(addr+1,servc);
   }
   else{
      *hsc = *addr;
      aly_addr_host(addr+1,hsc+1,servc);
   }
}
static void anly_cs(splp addr,splp cs,splp hsc,splp servc)
{
#  ifdef DEBUG_EHAND
   printf("anly_cs->");
#  endif
   if (*addr==':'){
      *cs = (char)NIL;
      aly_addr_host(addr+1,hsc,servc);
   }
   else{
      *cs = *addr;
      anly_cs(addr+1,cs+1,hsc,servc);
   }
}
static hsp anly_addr(splp addr)
{
   hsp rslt=NIL;

#  ifdef DEBUG_EHAND
   printf("anly_addr->");
#  endif
   if ((rslt=(hsp)(malloc(sizeof(hs_t))))==NIL)
      return((hsp)error(FATAL,"allocation error(anly_addr282)\n"));
   else{
//      aly_addr_host(addr,rslt->host,rslt->service);
      anly_cs(addr,rslt->cs,rslt->host,rslt->service);
      return(rslt);
   }
}

/**************************************************
 *  register a label bounded with socket.         *
 *------------------------------------------------*
 *  ret = registry(label);                        *
 *                                                *
 **************************************************/
static bindLSp mberLS(bindLSp ls,char *spl)
{
#  ifdef DEBUG_EHAND
   printf("mberLS->");
#  endif
   if (ls ==NIL)
      return((bindLSp)NIL);
   else{
      if (strcmp(ls->label,spl)==0)
         return(ls);
      else
         return(mberLS(ls->next,spl));
   }
}
static bindLSp makeLS(splp label,int socket,splp host,int qualifier)
{
   bindLSp new_LS=NIL;

#  ifdef DEBUG_EHAND
   printf("makeLS->");
#  endif
   if ((new_LS=(bindLSp)(malloc(sizeof(bindLS_t))))==NIL)
      return((bindLSp)error(FATAL,"allocation error(makeLS318)\n"));
   else{
      new_LS->label = label;
      new_LS->pair.qualifier = qualifier;
      new_LS->pair.line_or_socket = socket;
      strcpy(new_LS->pair.file_or_host,host);
      new_LS->next = (bindLSp)NIL;
      return(new_LS);
   }
}
static bindLSp apdLS1(bindLSp crrt,splp label,int socket,splp host,bindLSp pre,int qualifier)
{
#  ifdef DEBUG_EHAND
   printf("apdLS1->");
#  endif
   if (pre==NIL)
      return((bindLSp)error(FATAL,"Segmentation fault(apdLS1333)."));
   if (crrt==NIL){
      return((pre->next = makeLS(label,socket,host,qualifier)));
   }
   else
      return(apdLS1(crrt->next,label,socket,host,crrt,qualifier));
}
static bindLSp apdLS(bindLSp *top,splp label,int socket,splp host,int qualifier)
{
#  ifdef DEBUG_EHAND
   printf("apdLS->");
#  endif
   if (*top==NIL)
      return(*top=makeLS(label,socket,host,qualifier));
   else
      return(apdLS1((*top)->next,label,socket,host,*top,qualifier));
}
bindLSp registry1(splp label,int socket,splp host,int qualifier)
{
   bindLSp ety=NIL;

#  ifdef DEBUG_EHAND
   printf("registry1->");
#  endif
      if ((ety=mberLS(hash_tbl_LS[hash(label)],
                      label))!=(bindLSp)NIL){
         return(ety);
      }
      else
         return(apdLS(&hash_tbl_LS[hash(label)],
                      label,
                      socket,
                      host,
                      qualifier));
   return((bindLSp)error(FATAL,"invalid label(registry1367)\n"));
}
static bindLSp registry(element_t label,int socket,splp host,int qualifier)
{
   token lval=NIL;

#  ifdef DEBUG_EHAND
   printf("registry->");
#  endif
   lval = gettk(label);
   if ((lval->token_name==ID)&&
       (lval->attr.id.type==ACT)){
      return(registry1(lval->attr.id.spl_ptr,
                       socket,
                       //host.entry.tk->attr.value.fld.strings.str,
                       host,
                       qualifier));
   }
   return((bindLSp)error(FATAL,"invalid label(registry383)\n"));
}

/**************************************************
 *  retrieve a label bounded with socket.         *
 *------------------------------------------------*
 *  ret = retrieval(label);                       *
 *                                                *
 **************************************************/
bindLSp retrieval1(element_t label)
{
   token lval=NIL;
   bindLSp ety=NIL;

#  ifdef DEBUG_EHAND
   printf("retrieval->");
#  endif
   lval = gettk(label);
   if ((lval->token_name==ID)&&
       (lval->attr.id.type==ACT)){
      if ((ety=mberLS(hash_tbl_LS[hash(lval->attr.id.spl_ptr)],
                      lval->attr.id.spl_ptr))==(bindLSp)NIL)
         return((bindLSp)(NIL));
      else
         return(ety);
   }
   return((bindLSp)error(FATAL,"invalid label(retrieval409)\n"));
}
static int retrieval(element_t label)
{
   bindLSp ret=retrieval1(label);

   if (ret==(bindLSp)(NIL))
      return((int)error(FATAL,"unboud label(retrieval416):%s\n",gettk(label)->attr.id.spl_ptr));
   else
      return(ret->pair.line_or_socket);
}
/**************************************************
 *  retrieve a pair of a principal and a sys_call.*
 *------------------------------------------------*
 *  ret = retrieve_filename_lineno_pair(label);   *
 *                                                *
 *  filename_lineno_pair_t * ret --- *
 *  element_t label --- *
 **************************************************/
filename_lineno_pair_t *retrieve_filename_lineno_pair(element_t label)
{
   bindLSp ety=retrieval1(label);

#  ifdef DEBUG_MONITOR
   printf("retrieve_filename_lineno_pair->");
#  endif
   if (ety==(bindLSp)(NIL))
      return((filename_lineno_pair_t *)error(FATAL,"invalid label(retrieve436)\n"));
   else
      return(&(ety->pair));
}

/**************************************************
 *  associate label to a TCP socket               *
 *------------------------------------------------*
 *  ret = n_bind(label,host);                     *
 *                                                *
 *  char *host - name of host to which connection *
 *             is desired.                        *
 **************************************************/
char *n_tolower(char *lah,buffer *buf)
{
#  ifdef DEBUG_PARSE
   printf("n_tolower->");
#  endif
   if ((lah == (char *)NIL)||(buf == (buffer *)NIL))
      return((char *)error(FATAL,"Segmentation fault(n_tolower1129)\n"));
   else{   
      int len=strlen(lah);
      int c=0;
      for(initbuf(buf);c<len;c++)
         append_buf(buf,(char)(0x00FF&tolower(lah[c])));
      return(buf->buf);
   }
}

int syscallname_to_num(char *name)
{
   int num=-1;
   buffer tmpbuf;
   buffer buftmp;

#  ifdef DEBUG_EHAND
   printf("syscallname_to_num->");
#  endif
   if (name==(char *)NIL)
      return((int)error(FATAL,"Segmentation fault(syscallname_to_num431)\n"));
   for(num=0;num<NUMofSYSCALL;num++){
      if (!strcmp(n_tolower(syscalls[num],&tmpbuf),
                  n_tolower(name,&buftmp)))
         return(num);
   }
   return((int)error(FATAL,"Invalid function call:%s(syscallname_to_num450)\n",name));
}
//#ifdef DEBUG_MC
//int mc_set_breakpoint(char *file,int lineno,char flag){
//   return(lineno);
//}
//#endif
list_t n_bind(element_t label,element_t host)
{
   hsp host_serv=NIL;

#  ifdef DEBUG_EHAND
   printf("n_bind->");
#  endif
   if ((host.type==TOKEN)&&
       (host.entry.tk->token_name==VALUE)&&
       (host.entry.tk->attr.value.type==STR)&&
       (label.type==TOKEN)&&
       (label.entry.tk->token_name==ID)&&
       (label.entry.tk->attr.id.type==ACT)){
      host_serv = anly_addr(host.entry.tk
                             ->attr.value.fld.strings.str);
/***
 * When host_serv is the same it does not make sockets.
 *this mechanisem will be added.
 ***/
      if (strcmp("client",host_serv->cs)==0)
         registry(label,
                  listenTCP(host_serv->service),
                  host_serv->host,
                  FILE_SYS);
      else{
         if (strcmp("server",host_serv->cs)==0)
            registry(label,
                     connectTCP(host_serv->host,host_serv->service),
                     host_serv->host,
                     FILE_SYS);
         else{
            if (strcmp("syscall",host_serv->cs)==0)
               registry(label,
                        syscallname_to_num(host_serv->service),
                        host_serv->host,
                        SYS_CALL);
            else{
               if (strcmp("target",host_serv->cs)==0)
                  registry(label,
//                           mc_set_breakpoint(host_serv->host,atoi(host_serv->service),(char)FALSE),
                           atoi(host_serv->service),
                           host_serv->host,
                           BREAK_POINT);
               else
                  error(FATAL,"invalid address %s(n_bind455)",host_serv->cs);
            }
         }
      }
      free(host_serv);
      return(makenull(NIL));
   }
   else
      return((list_t)error(FATAL,"unbound(n_bind164): "));
}

/**************************************************
 *  send a data from a socket associated a label. *
 *------------------------------------------------*
 *                                                *
 **************************************************/
#ifdef DEBUG_MC
static int mc_send(element_t label,char value[])
{
   return(TRUE);
}
#endif
//static void addCRLF(buffer *src)
//{
//#  ifdef DEBUG_EHAND
//   printf("addCRLF->");
//#  endif
//   strcat(src->buf,"\r\n");
//#  ifdef DEBUG_EHAND_AA
//   printf("\nsend_buf= %s\n",src->buf);
//   printf("\nsend_buf= %x\n",src->buf[9]);
//   printf("send_buf= %x\n",src->buf[10]);
//#  endif
//}
static char *ls2ary(list_t data,char *array)
{
#  ifdef DEBUG_EHAND
   printf("ls2ary->");
#  endif
   if (isempty(data))
      return(array);
   else{
      char *test = (char *)(getval(car(getls(car(data)))));
      return(strcat(ls2ary(cdr(data),
                           array),
                    test));
   }
}
static char *ls2int(list_t data,buffer *array)
{
#  ifdef DEBUG_EHAND
   printf("ls2int->");
#  endif
   if (isempty(data))
      return(array->buf);
   else{
      ins_buf(array,cvtia((int)(getval(car(getls(car(data)))))));
      return(ls2int(cdr(data),array));
   }
}
list_t n_send(element_t label,list_t data)
{
   filename_lineno_pair_t *pair=retrieve_filename_lineno_pair(label);
   buffer send_buf;

#  ifdef DEBUG_EHAND
   printf("n_send->");
#  endif
   initbuf(&send_buf);
   if (pair->qualifier == BREAK_POINT){
      if (!isempty_buf(&target))
         mc_send(label,ls2int(data,&send_buf));
      else
         return((list_t)error(WARNING|EEL,"Cannot send the data for the target(n_send571).\n"));
   }
   else{
      ls2ary(data,send_buf.buf);
//      addCRLF(&send_buf);
      write(retrieval(label),send_buf.buf,strlen(send_buf.buf));
   }
   return(makenull(NIL));
}
/**************************************************
 *  receve a data from a socket associated a label*
 *------------------------------------------------*
 *                                                *
 **************************************************/
#ifdef DEBUG_MC
char *mc_recv(element_t label,buffer *buf)
{
   return("123");
}
#endif
static list_t ary2ls(char *array)
{
#  ifdef DEBUG_EHAND
   printf("ary2ls->");
#  endif
   return(cons(*makelet(LIST,
                        cons(*makelet(TOKEN,
                                     makesym(VALUE,
                                             STR,
                                             array)),
                            makenull(NIL))),
               makenull(NIL)));
}
static list_t int2ls(char *array)
{
#  ifdef DEBUG_EHAND
   printf("int2ls->");
#  endif
   return(cons(*makelet(LIST,
                       cons(*makelet(TOKEN,
                                     makesym(VALUE,
                                             ICONST,
                                             array)),
                            makenull(NIL))),
               makenull(NIL)));
}
list_t n_recv(element_t label)
{
   buffer recv_buf;
   int r;
   filename_lineno_pair_t *pair = retrieve_filename_lineno_pair(label);

#  ifdef DEBUG_EHAND
   printf("n_recv->");
#  endif
   initbuf(&recv_buf);
   if (pair->qualifier == BREAK_POINT)
      return(int2ls(mc_recv(label,&recv_buf)));
   else{
      for(r=0;r==0;){
         r = read(pair->line_or_socket,recv_buf.buf,BUF_SIZE);
         if (r == -1)
            errexit("socket read failed: %s\n",strerror(errno));
      }
      return(ary2ls(recv_buf.buf));
   }
}
/**************************************************
 *  receve a data from a socket associated a label*
 *------------------------------------------------*
 *                                                *
 **************************************************/
bool n_select(element_t label)
{
   filename_lineno_pair_t *pair = retrieve_filename_lineno_pair(label);
   fd_set rfds;
//   struct timeval time={(long)10,(long)5}; 

#  ifdef DEBUG_EHAND
   printf("n_select->");
#  endif
   if (pair->qualifier == BREAK_POINT)
      return(TRUE);
   else{
      FD_ZERO(&rfds);
      FD_SET(pair->line_or_socket,&rfds);
      if (select(getdtablesize(),&rfds,(fd_set *)0,(fd_set *)0,NULL)<0)
         return((bool)FALSE);
      else
         return((bool)TRUE);
   }
}
