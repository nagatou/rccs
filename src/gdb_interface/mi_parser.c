#include <assert.h>
#include <string.h>
#define _GNU_SOURCE
#include <features.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <glib.h>

#include "mi_parser.h"
#include "debugger.h"
#include "log_printf.h"
#include "utils.h"

static char *_strndup(const char *const src, size_t len) {
    char *buf = malloc((len+1)*sizeof(char));
    memset(buf, '\0', len+1*sizeof(char));
    return strncpy(buf, src, len);
}

extern int asprintf (char **__restrict __ptr,
                     __const char *__restrict __fmt, ...)
     __THROW __attribute__ ((__format__ (__printf__, 2, 3)));


/* Response from GDB might consist of multiple lines.
   Split the response into lines for parsing.
   Returns the number of lines in the response.   
*/
int
split_into_lines(char ***lines, char *response) {
	int i;
	int l_index = 0, l_size = 20;//number of lines
	int c_index = 0, c_size = 128;//number of chars in each line.
	char **l;
	char *r = response;
	
	l = (char **) calloc(l_size, sizeof(char *));
	for (i = 0; i < l_size; i++) {
		l[i] = (char *) calloc(c_size, sizeof(char));
	}
	
	while (*r) {
		if (c_index >= c_size) {
			c_size += 128;
			l[l_index] = (char *) realloc(l[l_index], c_size * sizeof(char));
		}
		
		if (*r != '\n') {/*EOL hasn't reached yet*/
			l[l_index][c_index] = *r;
			c_index++;
			r++;
		}
		else {/*EOL*/
			l[l_index][c_index] = '\0';
			c_index = 0;//reset the values for next line to be read in.
			c_size = 128;
			r++;
			if (*r) l_index++;/*If there are more lines to be read*/
			if(l_index >= l_size) {//max number of lines reached. Need to allocate more lines
				int prev_l_size = l_size;
				//int prev_l_index = l_index;
				l_size += 10;//allocate more lines.
				l = (char **) realloc(l, l_size * sizeof(char *));
				for (i = prev_l_size; i < l_size; i++) {
					l[i] = (char *) calloc(c_size, sizeof(char));
				}
			}
		}
	}
	l[l_index+1] = (char *)NULL;
	*lines = l;
	
	return l_index;
}

void
free_gdbmi_result_tuple_foreach(char *key, gdbmi_result *value) {
   free_gdbmi_result(value);
}

void
free_gdbmi_result(gdbmi_result *result) {
   if (result == (gdbmi_result *)NULL)
      return;

   switch(result->type) {
      case GDBMI_LITERAL:
         myfree(result->data.literal);
         break;
      case GDBMI_TUPLE:
         g_hash_table_foreach(result->data.tuple, (GHFunc)free_gdbmi_result_tuple_foreach, (gpointer) NULL);
         g_hash_table_destroy(result->data.tuple);
         break;
      case GDBMI_LIST:
         free_List(result->data.list);
         break;  
   }
   myfree(result->name);
   myfree(result);
   result = NULL;
}

gdbmi_result *
new_gdbmi_result(gdbmi_value_type type, char *name) {
   gdbmi_result *new_result = (gdbmi_result *) mymalloc(sizeof(gdbmi_result));
   
   new_result->class = (char *)NULL;
   if (name)
      new_result->name = strdup(name);
   else 
   	new_result->name = (char *) NULL;
   
   new_result->type = type;
   switch (type) {
      case GDBMI_LITERAL:
         new_result->name = (char *)NULL;
         break;
      case GDBMI_TUPLE:
         new_result->data.tuple = g_hash_table_new(g_str_hash, g_str_equal);
         break;
      case GDBMI_LIST:
         new_result->data.list = new_List();
         break;  
   }
   return new_result;
}

gdbmi_result *
new_gdbmi_literal(char *literal_data) {
   gdbmi_result *result_literal;
   result_literal = new_gdbmi_result(GDBMI_LITERAL, (char *)NULL);
   if (result_literal) {
      result_literal->data.literal = strdup(literal_data);  
   }
   return result_literal;  
}

gdbmi_result *
new_gdbmi_tuple() {
   gdbmi_result *result_tuple;
   result_tuple = new_gdbmi_result(GDBMI_TUPLE, (char *)NULL);
   return result_tuple;
}

gdbmi_result *
new_gdbmi_list() {
   gdbmi_result *result_list;
   result_list = new_gdbmi_result(GDBMI_LIST, (char *)NULL);
   return result_list;
}

void
gdbmi_result_setname(gdbmi_result *result, char *name) {
   assert (result != (gdbmi_result *)NULL && name != (char *)NULL);
   result->name = strdup(name);
}

char *
gdbmi_result_getname(gdbmi_result *result) {
   assert(result && result->name);
   
   return result->name;  
}

void
gdbmi_result_hash_insert(gdbmi_result *tuple, char *key, gdbmi_result *value) {
   assert(tuple && key && value);
   
   g_hash_table_insert(tuple->data.tuple, strdup(key), value); 
}

void
gdbmi_result_list_append(GDBMI_List **l, gdbmi_result *item) {
   GDBMI_List *list = *l;
   if (list->last >= list->count) {
      expand_List(&list);
   }
   list->element[list->last] = item;
   (list->last)++;
}

gdbmi_result *
parse_gdbmi_results(char **res_ptr) {
   gdbmi_result *result = (gdbmi_result *) NULL;
   
   if (**res_ptr == '\0') {
      log_printf(LOG_WARNING, "End of stream reached.\n");
      return result;
   }
   
   if (**res_ptr == '"') {
   /*C-string*/
      char escaped = FALSE;//for parsing escape sequences in the result.
      int buf_size = 256;//initial buffer size
      int cur_buf_len = 0;//length of the current buffer
      char *buf = (char *) mymalloc(sizeof(char) * buf_size);
      
      (*res_ptr)++;
       while (escaped || *res_ptr[0] != '"') {
       	/*reset the 'escaped' flag*/
          if (escaped) {
             escaped = FALSE;
          }
          
          else if (*res_ptr[0] == '\\') {//escape sequence
             escaped = TRUE;                    
          }
          
          buf[cur_buf_len++] = **res_ptr;
          (*res_ptr)++;
          
          if (cur_buf_len >= buf_size) {
             buf_size += 128;
             buf = (char *) realloc(buf, buf_size);
          }
       }
       (*res_ptr)++;//skip the closing '"'
       buf[cur_buf_len++] = '\0';
       result = new_gdbmi_literal(buf);
       myfree(buf);
   }
   
   else if (*res_ptr[0] == '{') {
   /*Tuple*/
      result = new_gdbmi_tuple();
      (*res_ptr)++;
      
      while(*res_ptr[0] != '}') {
         gdbmi_result *tuple_element;
         tuple_element = parse_gdbmi_results(res_ptr);
         if (tuple_element) {       
         //Insert the tuple_element into the hash
         	gdbmi_result_hash_insert(result, \
         			gdbmi_result_getname(tuple_element), tuple_element);
         }
         else {
         	log_printf(LOG_WARNING, "Parse error: NULL element");
         	//result = (gdbmi_result *) NULL;
         	free_gdbmi_result(result);
         	break;
         }
         
         //skip separator ','
         if (*res_ptr[0] == ',')
            (*res_ptr)++;
         else if (*res_ptr[0] != ',' && *res_ptr[0] != '}') {
         	log_printf(LOG_ERROR, "Parse error: invalid separator %c", *res_ptr[0]);
         	//result = (gdbmi_result *) NULL;
         	free_gdbmi_result(result);
         	tuple_element = (gdbmi_result *)NULL;
         	free_gdbmi_result(tuple_element);
         	break;
         }
      }
      (*res_ptr)++;//skip the closing '}'
      return result;
   }
   
   else if (*res_ptr[0] == '[') {
   /*List*/  
      result = new_gdbmi_list();
      (*res_ptr)++;
      
      while (*res_ptr[0] != ']') {
         gdbmi_result *list_item;
         list_item = parse_gdbmi_results(res_ptr);
         if (list_item) {
         //Insert the item into the list
         	gdbmi_result_list_append(&(result->data.list), list_item);
         }
         else {
         	log_printf(LOG_WARNING, "Parse error: NULL element");
         	result = (gdbmi_result *) NULL;
         	free_gdbmi_result(result);
         	break;
         }
         //skip separator ','
         if (*res_ptr[0] == ',')
            (*res_ptr)++;
         else if (*res_ptr[0] != ',' && *res_ptr[0] != ']') {
         	log_printf(LOG_WARNING, "Parse error: invalid list separator %d", *res_ptr[0]);
         	result = (gdbmi_result *) NULL;
         	free_gdbmi_result(result);
         	list_item = (gdbmi_result *)NULL;
         	free_gdbmi_result(list_item);
         	break;
         }
      }
      (*res_ptr)++;//skip the closing ']'
      return result;
   }
   
   else if(isalpha(*res_ptr[0])) {
    /*the result is in [Variable = Value] form*/
      char *temp_ptr = *res_ptr;
      char *variable = (char *)NULL;
      
      /*parse the variable portion*/
      while (**res_ptr != '=') {
         if (**res_ptr == '\0') {
            log_printf(LOG_WARNING, "invalid assignment.");
            return (gdbmi_result *)NULL;
         }
         (*res_ptr)++;
      }
      variable = _strndup(temp_ptr, *res_ptr - temp_ptr);
      (*res_ptr)++;//skip '='
      
      /*parse the value portion*/
      result = parse_gdbmi_results(res_ptr);
      if (result) {
         gdbmi_result_setname(result, variable);
      }
      else {
         log_printf(LOG_ERROR, "error parsing the value portion");  
      }
      
#if 0//ifdef MC_DEBUG      
      if (result->type == GDBMI_LITERAL) {
         printf("%s = %s, ", variable, result->data.literal);
      }
      else {
         printf("%s =  ", variable);
         if (result->type == GDBMI_LIST) {
              
         }
      }
      fflush(stdout);
#endif

      //variable = (char *)NULL;
      myfree(variable);      
   }
   
   return result;
}

gdbmi_result *
parse_gdbmi_response(gdb_handle *g_h) {
   gdbmi_result *result = (gdbmi_result *)NULL;
   char **lines;
	int len = 0;
	int i;
   
   assert(g_h->response != (char *)NULL);
   
	len = split_into_lines(&lines, g_h->response);
   
	for(i = 0; i <= len; i++) {
		switch (lines[i][0]) {
   		/*Discard console-stream, target-stream and log-stream output. */
			case '~' :
			case '&' :
			case '@' :
				//printf("%s\n", lines[i]);
				break;
			/**/
						/*Async-output*/
			case '*':
			case '+':
			case '=':
						/*result-output*/
			case '^':
			   if (strchr(lines[i], ',')) {
			   	char *res_temp = (char *) NULL;
   			   //char *t = "{BreakpointTable={nr_rows=\"2\",nr_cols=\"6\",hdr=[{width=\"3\",alignment=\"-1\",col_name=\"number\",colhdr=\"Num\"},{width=\"14\",alignment=\"-1\",col_name=\"type\",colhdr=\"Type\"},{width=\"4\",alignment=\"-1\",col_name=\"disp\",colhdr=\"Disp\"},{width=\"3\",alignment=\"-1\",col_name=\"enabled\",colhdr=\"Enb\"},{width=\"10\",alignment=\"-1\",col_name=\"addr\",colhdr=\"Address\"},{width=\"40\",alignment=\"2\",col_name=\"what\",colhdr=\"What\"}],body=[bkpt={number=\"1\",type=\"breakpoint\",disp=\"keep\",enabled=\"y\",addr=\"0x08050f5d\",func=\"main\",file=\"main.c\",line=\"122\",times=\"1\"},bkpt={number=\"2\",type=\"breakpoint\",disp=\"keep\",enabled=\"y\",addr=\"0x0096fbae\",func=\"anjuta_plugin_activate\",file=\"anjuta-plugin.c\",line=\"395\",times=\"1\"}]}}";
   			   char *res = (char *)NULL;// = (char *) mymalloc(sizeof(char) * strlen(lines[i]));
   			   
   			   asprintf(&res, "{%s}", (strchr(lines[i], ',') + 1));
   			   res_temp = res;
   			   result = parse_gdbmi_results(&res);
   			   //if (result->type == GDBMI_TUPLE) {
      			   //gdbmi_result *temp = g_hash_table_lookup(result->data.tuple, "msg");
      			   //temp = g_hash_table_lookup(temp->data.tuple, "body");
      			   //temp = g_hash_table_lookup(temp->data.list->element[1]->data.tuple, "func");
      			   //printf("\n body = {func = %s}\n", temp->data.literal);  
   			   //}
			   	myfree(res_temp);
#ifdef MC_DEBUG
			   	printf("FROM > %s\n\n", lines[i]);//debug
#endif
			   }
			   /*else {
			   	printf("%s\n", lines[i]);
			   }*/
				break;
			default :
				//printf("%s\n", lines[i]);//MC_DEBUG			
				//log_printf(LOG_WARNING, "parse_gdbmi_response: Error in result-output from gdb\n");
				break;
		}
	}
   
   /*Free lines*/
   for (i = 0; i <= len; i++) {
	   myfree(lines[i]);
   }
	myfree(lines);
	
   return result;
}

void
dump_parsed_gdbmi_result(gdbmi_result *r) ;

static void
gdbmi_tuple_dump_foreach(char *key, gdbmi_result *value) {
   dump_parsed_gdbmi_result(value);
}

void
dump_parsed_gdbmi_result(gdbmi_result *r) {
   int i;
   gdbmi_result *result = r;
   if (r == (gdbmi_result *)NULL) {
   	return;
   }
   switch (result->type) {
      case GDBMI_LITERAL:
         printf("%s = %s, ",result->name, result->data.literal);         
         break;
      case GDBMI_TUPLE:
         if (result->name) printf("%s = ", result->name);
         printf(" {");
         g_hash_table_foreach(result->data.tuple, (GHFunc) gdbmi_tuple_dump_foreach, (gpointer) NULL);
         printf("} ");
         break;
      case GDBMI_LIST:
         if (result->name) printf("%s = ", result->name);
         printf(" [");
         for (i=0; i < result->data.list->last; i++) {
            dump_parsed_gdbmi_result(result->data.list->element[i]);  
         }
         printf(" ] ");
         break;  
   }
   fflush(stdout);
}

gdbmi_result *
mi_parser_get_tuple_value(gdbmi_result *r, char *var) {
	gdbmi_result *result = (gdbmi_result *)NULL;
	
	if (!r || !var) {
		return (gdbmi_result *)NULL;
	}
	
	result = g_hash_table_lookup(r->data.tuple, var);
	if (!result) {
		log_printf(LOG_WARNING, "Parse error: Could not parse the value of %s", var);
		return (gdbmi_result *)NULL;
	}
	return result;
}
