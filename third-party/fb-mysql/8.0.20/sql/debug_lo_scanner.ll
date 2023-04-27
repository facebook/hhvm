%top{
/* Copyright (c) 2018, 2019, Oracle and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

/* LOCK ORDER, flex scanner for arc data file. */

/*
  flex %option nounistd does not work:
  https://github.com/westes/flex/issues/345
  Work around: define YY_NO_UNISTD_H explicitly
*/

#ifndef YY_NO_UNISTD_H
#define YY_NO_UNISTD_H
#endif
}

%option noinput
%option nounput
%option noyywrap
%option nodefault
%option never-interactive
/* Broken in flex: %option nounistd */
%option 8bit
%option yylineno
%option reentrant
%option extra-type="struct MEM_ROOT *"
%option bison-bridge
%option bison-locations

%{
#include <string.h>

#include "my_alloc.h"
#include "sql/debug_lo_parser.h"
#include "mysql/components/services/log_builtins.h"
#include "mysqld_error.h"

#define YY_DECL int LOCK_ORDER_lex(YYSTYPE *yylval_param, YYLTYPE *yylloc_param, yyscan_t yyscanner)

#define YY_USER_ACTION \
  yylloc_param->first_line = yylineno; \
  yylloc_param->last_line = yylineno; \
  yylloc_param->first_column = yycolumn; \
  yylloc_param->last_column = yycolumn+yyleng; \
  yycolumn += yyleng;
  
/*
  Allocate string token on the caller memory root.
*/
static char* lo_alloc_string(MEM_ROOT *mem_root, const char* str, size_t length) {
  char * result;
  result = static_cast<char*> (mem_root->Alloc(length + 1));
  if (result != nullptr) {
    strncpy(result, str, length + 1);
    result[length] = '\0';
  }
  return result;
}

%}

WHITE_SPACE [ \t]+
NEW_LINE [\n]
COMMENT_LINE [#][^\n]*
STR [^"]*

%%

"ARC" {
    return ARC_SYM;
  }

"BIND" {
    return BIND_SYM;
  }

"CONSTRAINT" {
    return CONSTRAINT_SYM;
  }

"COMMENT" {
    return COMMENT_SYM;
  }

"DEBUG" {
    return DEBUG_SYM;
  }

"FLAGS" {
    return FLAGS_SYM;
  }

"FROM" {
    return FROM_SYM;
  }

"IGNORED" {
    return IGNORED_SYM;
  }

"LOOP" {
    return LOOP_SYM;
  }

"NODE" {
    return NODE_SYM;
  }

"OP" {
    return OP_SYM;
  }

"RECURSIVE" {
    return RECURSIVE_SYM;
  }

"STATE" {
    return STATE_SYM;
  }

"TO" {
    return TO_SYM;
  }

"TRACE" {
    return TRACE_SYM;
  }

"UNFAIR" {
    return UNFAIR_SYM;
  }

"\""{STR}"\"" {
    char * str = lo_alloc_string (yyextra, yytext + 1, yyleng - 2);
    yylval->m_str = str;
    return STR_SYM;
  }

{WHITE_SPACE} {}
{COMMENT_LINE} {}
{NEW_LINE} {}

. {
  char msg[256];
  snprintf(msg, sizeof(msg), "Unrecognized character: %s", yytext);
  /* message format = "Lock order scanner: (%d:%d) - (%d:%d) : %s" */
  LogErr(ERROR_LEVEL, ER_LOCK_ORDER_SCANNER_SYNTAX,
    yylloc_param->first_line,
    yylloc_param->first_column,
    yylloc_param->last_line,
    yylloc_param->last_column,
    msg);
  }

%%

/*
  Helper tool to debug syntax.
  Build with:
    gcc -DTEST_MAIN debug_lo_scanner.cc
  Run with
    a.out lock_order_dependencies.txt
*/
#ifdef TEST_MAIN
#include "debug_lo_scanner.h"

int main(int argc, char *argv[])
{
  if (argc != 2)
  {
    fprintf(stderr, "usage: %s /path/to/lo_arc_data.txt\n", argv[0]);
    return 1;
  }

  const char* filename = argv[1];

  FILE *data = fopen(filename, "r");
  if (data == nullptr) {
    perror(filename);
    return 2;
  }

  yyscan_t my_scanner;
  LOCK_ORDER_lex_init(& my_scanner);
  LOCK_ORDER_set_in(data, my_scanner);

  YYSTYPE my_yylval;
  YYLTYPE my_yylloc;
  int tok;

  while (tok = LOCK_ORDER_lex(& my_yylval, & my_yylloc, my_scanner))
  {
    fprintf(stdout,
            "(%d:%d - %d:%d) ",
            my_yylloc.first_line,
            my_yylloc.first_column,
            my_yylloc.last_line,
            my_yylloc.last_column);

    switch (tok)
    {
      case ARC_SYM:
        fprintf(stdout, "token arc\n");
        break;
      case BIND_SYM:
        fprintf(stdout, "token bind\n");
        break;
      case CONSTRAINT_SYM:
        fprintf(stdout, "token constraint\n");
        break;
      case COMMENT_SYM:
        fprintf(stdout, "token comment\n");
        break;
      case DEBUG_SYM:
        fprintf(stdout, "token debug\n");
        break;
      case FLAGS_SYM:
        fprintf(stdout, "token flags\n");
        break;
      case FROM_SYM:
        fprintf(stdout, "token from\n");
        break;
      case IGNORED_SYM:
        fprintf(stdout, "token ignored\n");
        break;
      case LOOP_SYM:
        fprintf(stdout, "token loop\n");
        break;
      case NODE_SYM:
        fprintf(stdout, "token node\n");
        break;
      case OP_SYM:
        fprintf(stdout, "token op\n");
        break;
      case RECURSIVE_SYM:
        fprintf(stdout, "token recursive\n");
        break;
      case STATE_SYM:
        fprintf(stdout, "token state\n");
        break;
      case TO_SYM:
        fprintf(stdout, "token to\n");
        break;
      case TRACE_SYM:
        fprintf(stdout, "token trace\n");
        break;
      case UNFAIR_SYM:
        fprintf(stdout, "token unfair\n");
        break;
      case STR_SYM:
        fprintf(stdout, "token str\n");
        break;
      default:
        fprintf(stdout, "unknown token %d\n", tok);
        break;
    }
  }
  LOCK_ORDER_lex_destroy(my_scanner);
  fclose(data);
}
#endif /* TEST_MAIN */

