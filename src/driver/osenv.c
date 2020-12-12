#define EXTERN
#include "comm.h"
#include "osenv.h"
#include "error.h"

void backup_os_env(FILE *file_ptr, buffer buf)
{
#  ifdef DEBUG_EVAL
   printf("backup_os_env->");
#  endif
   if (os_env_backup.top < OSENV_MAX_DEPTH){
      os_env_backup.bkup[os_env_backup.top]=file_ptr;
      os_env_backup.top++;
   }
   else
      error(FATAL,"the nest of loads is too much.");
}
FILE * reload_os_env(void)
{
#  ifdef DEBUG_EVAL
   printf("reload_os_env->");
#  endif
   if (os_env_backup.top > 0)
      return(os_env_backup.bkup[--os_env_backup.top]);
   else{
      setenv("RCCS_VPATH",os_env_backup.buf.buf,TRUE);
      return((FILE *)error(FATAL,"internal fatal error."));
   }
}
void init_os_env(void)
{
   int i;

#  ifdef DEBUG_EVAL
   printf("init_os_env->");
#  endif
   os_env_backup.top=0;;
   for(i=0;i<OSENV_MAX_DEPTH;i++)
      os_env_backup.bkup[i]=(FILE *)(NIL);
   setenv("RCCS_VPATH",getenv("PWD"),TRUE);
}
