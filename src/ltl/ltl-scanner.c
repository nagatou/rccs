/*
 * $Id: ltl-scanner.c,v 1.2 2011/06/08 10:55:28 nagatou Exp $
 */

#define EXTERN extern
#include "comm.h"
#include "ltl-scanner.h"

static char *_strndup(const char *const src, size_t len) {
    char *buf = malloc((len+1)*sizeof(char));
    memset(buf, '\0', len+1*sizeof(char));
    return strncpy(buf, src, len);
}

ltl_lexeme get_next_lexeme(ltl_scanner_info *info) {
    ltl_lexeme lexeme;

#ifdef LTL_SCANNER_DEBUG
    printf("scanning in: \"%s\"\n", info->src);
#endif

    while (isspace(info->src[0])) {
        ++info->src;    // skip spaces
    }

    if (info->src[0] == '\0') {
        lexeme.type = EOT;
        lexeme.lit = NULL;
        return lexeme;  // end of stream, we need to exit here...
    }
    
    // X, U and R are operators
    if (isletters(info->src[0]) && !(info->src[0] == 'X' || info->src[0] == 'U' || info->src[0] == 'R')){
        size_t len = 1;
        while (isdigits(info->src[len])||isletters(info->src[len])) { ++len; }
        lexeme.lit = _strndup(info->src, len);

        // check if we found a boolean...
        if (strcmp("tt", lexeme.lit) == 0) {
            lexeme.type = TERM_BOOLEAN;
            lexeme.info.termbool = TRUE;
        } else if (strcmp("ff", lexeme.lit) == 0) {
            lexeme.type = TERM_BOOLEAN;
            lexeme.info.termbool = FALSE;
        } else {
            lexeme.type = TERM_ID;
        }

        info->src += len;
        return lexeme;
    }

    if (isdigit(info->src[0])) {
        int len = 1;
        lexeme.type = TERM_DIGITS;
        while (isdigit(info->src[len])) { ++len; }
        lexeme.lit = _strndup(info->src, len);
        lexeme.info.termint = atoi(lexeme.lit);
        info->src += len;
        return lexeme;
    }
    
    char *termlit = malloc(3*sizeof(char));
    termlit[0] = info->src[0];
    termlit[1] = termlit[2] = '\0';
    
    switch (info->src[0]) {
        case '!':
            lexeme.type = NOT_STATE_PROP;
            break;
        case 'U':
            lexeme.type = UNTIL_PATH;
            break;
        case 'R':
            lexeme.type = RELEASE_PATH;
            break;
        case 'X':
            lexeme.type = NEXTTIME_PATH;
            break;
        case '&':
            lexeme.type = AND_PROP;
            break;
        case '|':
            lexeme.type = OR_PROP;
            break;
        case '(':
            lexeme.type = OPENBRACKET_PATH_TERM;
            break;
        case ')':
            lexeme.type = CLOSEBRACKET_PATH_TERM;
            break;
        case '=':
            lexeme.type = FUNC_EQUALS;
            break;
        case '+':
            lexeme.type = FUNC_PLUS;
            break;
        case '-':
            if (info->src[1] == '>') {
                lexeme.type = IMPLIES_STATE_PROP;
                termlit[1] = info->src[1];
                ++info->src;
            } else {
                lexeme.type = FUNC_MINUS;
            }
            break;
        case '/':
            if (info->src[1] == '\\') {
                lexeme.type = AND_PATH;
                termlit[1] = info->src[1];
                ++info->src;
            } else {
                lexeme.type = UNKNOWN;
            }
            break;
        case '\\':
            if (info->src[1] == '/') {
                lexeme.type = OR_PATH;
                termlit[1] = info->src[1];
                ++info->src;
            } else {
                lexeme.type = UNKNOWN;
            }
            break;
        case '[':
            if (info->src[1] == ']') {
                lexeme.type = ALL_PATH;
                termlit[1] = info->src[1];
                ++info->src;
            } else {
                lexeme.type = UNKNOWN;
            }
            break;
        case '<':
            if (info->src[1] == '>') {
                lexeme.type = EXIST_PATH;
                termlit[1] = info->src[1];
                ++info->src;
            } else if (info->src[1] == '=') {
                lexeme.type = FUNC_SMALLEREQUAL;
                termlit[1] = info->src[1];
                ++info->src;
            } else {
                lexeme.type = FUNC_SMALLER;
            }
            break;
        case '>':
            if (info->src[1] == '=') {
                lexeme.type = FUNC_LARGEREQUAL;
                termlit[1] = info->src[1];
                ++info->src;
            } else {
                lexeme.type = FUNC_LARGER;
            }
            break;
        default:
            lexeme.type = UNKNOWN;
    }
    
    ++info->src;
    lexeme.lit = termlit;
    return lexeme;
}

