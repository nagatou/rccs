#ifndef __MI_PARSER_H__
#define __MI_PARSER_H__
#include "utils.h"
#include <glib.h>

typedef enum {
   GDBMI_LITERAL,
   GDBMI_TUPLE,
   GDBMI_LIST
} gdbmi_value_type;

struct _gdbmi_result {
	char *class;//stopped/done/running/connected/error/exit
   char *name;//this is the variable name for for assignment results, null for others
   gdbmi_value_type type;//literal or list or tuple
      
   union {
      char *literal;
      GHashTable *tuple;
      struct _gdbmi_list *list;
   } data;
   
   //struct _gdbmi_result *next;//TODO
};

typedef struct _gdbmi_result gdbmi_result;
gdbmi_result *parse_gdbmi_response();
gdbmi_result *parse_gdbmi_results(char **);
void free_gdbmi_result(gdbmi_result *result);

void dump_parsed_gdbmi_result(gdbmi_result *r);

gdbmi_result *mi_parser_get_tuple_value(gdbmi_result *r, char *var);

#endif
