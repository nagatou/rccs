#include <stdio.h>
#include "mc_interface.h"

extern gdb_handle *g_h;
int 
main(){
   gdbmi_result *result = (gdbmi_result *) NULL;
   buffer buf;
   int i = 0;
   
	mc_init_debugger("test_scale", NULL);	
	
	//printf("%s\n", gdb_get_varname(NULL, "fileman.c", 71));
	
	
	result = gdb_insert_breakpoint(g_h, "11", TRUE);
	//dump_parsed_gdbmi_result(result);
	free_gdbmi_result(result);
	result = (gdbmi_result *)NULL;

	for (i = 0 ; i<4; i++) {
		result = gdb_continue(g_h);
		free_gdbmi_result(result);
		result = (gdbmi_result *)NULL;
		printf("\nc = %s\n", mc_get_value("c", &buf));
	}
	
	/*getchar();
	result = gdb_run(g_h);
	free_gdbmi_result(result);
	result = (gdbmi_result *)NULL;
	*/
	/*result = wait_for_async_stop(g_h);
	dump_parsed_gdbmi_result(result);
	//tmp = g_hash_table_lookup(result->data.tuple, "reason");
	//printf("%s\n", tmp->data.literal);
	free_gdbmi_result(result);
	result = (gdbmi_result *)NULL;
	*/
	/*
	getchar();
	result = gdb_print(g_h, "done");
	free_gdbmi_result(result);
	result = (gdbmi_result *)NULL;
	*/
	
	//printf("starting\n");
	//getchar();
	//mc_send1("fileman.c", 71, "71");
	//printf("\ni = %s\n", mc_get_value("i", &buf));
	
	//getchar();
	//mc_send1("fileman.c", 88, "88");
	//printf("\ni = %s\n", mc_get_value("i", &buf));
	
	//getchar();
	//mc_send1("fileman.c", 90, "90");
	//printf("\ni = %s\n", mc_get_value("i", &buf));
	
	/*result = gdb_continue(g_h);
	free_gdbmi_result(result);
	result = (gdbmi_result *)NULL;*/
	/*
	result = wait_for_async_stop(g_h);
	dump_parsed_gdbmi_result(result);
	free_gdbmi_result(result);
	result 	= 		(gdbmi_result *)NULL;
	*/
	
	//printf("backtracking\n");
	
	//getchar();
	//mc_back(1);	
	//printf("\ni = %s\n", mc_get_value("i", &buf));
	
	//getchar();
	//printf("%s\n", parse_line("79\t\t line = readline (\"FileMan: \");\n"));
	//printf("%s\n", gdb_get_varname(g_h, "fileman.c:72"));
	stop_gdb(g_h);
   return 1;
}
