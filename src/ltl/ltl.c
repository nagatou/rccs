/*
 * $Id: ltl.c,v 1.3 2012/06/16 09:07:35 nagatou Exp $
 */

#define EXTERN extern
#include "comm.h"
#include "ltl-scanner.h"
#include "ltl-parser.h"
#include "normalizer.h"
#include "ltl-converter.h"
#include "buchi-converter.h"

char *ltl2ccs(const char *const ltlsrc, buffer* buf) {
    ltl_scanner_info scinfo;
    scinfo.src = ltlsrc;
    ltl_lexeme l;
    ltl_parser_state painfo;
    buffer ba;
    
    painfo.last = painfo.top = NULL;
    do {
        l = get_next_lexeme(&scinfo);
        ltl_parse(&painfo, &l);
        free(l.lit);
    } while ( l.type != EOT );
    
    painfo.top = ltl_normalize(painfo.top);
    
    initbuf(&ba);
    printf("normalized: %s\n", ltl_token2string(painfo.top, &ba));
  
    linked_list nodes = ltl2buchi(painfo.top);
    buchi2ccs(&nodes, buf);
    
    free_ltl_token(painfo.top);
    free_buchi_nodes(&nodes);
    printf("%s\n",buf->buf);

    return buf->buf;
}
