/*
 * $Id: ltl-test.c,v 1.2 2012/06/16 09:07:35 nagatou Exp $
 */

#define EXTERN
#include "comm.h"
#include "ltl-scanner.h"
#include "ltl-parser.h"
#include "normalizer.h"
#include "ltl-converter.h"
#include "buchi-converter.h"
#include "dot.h"

#ifdef LTL_TEST
# define main_ltl_test main
#endif

void dotest(char *ltl) {
    ltl_scanner_info scinfo;
    scinfo.src = ltl;
    ltl_lexeme l;
    ltl_parser_state painfo;
    painfo.last = painfo.top = NULL;
    buffer buf;
    
    do {
        l = get_next_lexeme(&scinfo);
//        printf("got lexeme \"%s\" (Id: %d)\n", l.lit, l.type);
        ltl_parse(&painfo, &l);
//        initbuf(&buf);
//        printf(" -> %s\n", ltl_token2string(painfo.top, &buf));
        free(l.lit);
    } while ( l.type != EOT );

   ltl2dot(painfo.top, "ltl-tree.dot");
//   initbuf(&buf);
//   printf("%s\n", ltl_token2string(painfo.top, &buf));

    painfo.top = ltl_normalize(painfo.top);
//    printf("normalizer done...");
    
    ltl2dot(painfo.top, "ltl-tree-norm.dot");
//    initbuf(&buf);
//    printf("normalized: %s\n", ltl_token2string(painfo.top, &buf));
    
    linked_list nodes = ltl2buchi(painfo.top);
    buchi2dot(&nodes, "buchi-graph.dot");
    
//    initbuf(&buf);
//    printf("%s\n", buchi2ccs(&nodes, &buf));
    
    free_ltl_token(painfo.top);
    free_buchi_nodes(&nodes);
//    printf("done\n");
}

int main_ltl_test(int argc,char *argv[]) {
    if (argc == 2)
        dotest(argv[1]);
    else
        dotest("a U b U c");

    return 0;
}
