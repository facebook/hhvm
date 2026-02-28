
%{

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

#include <string>
#include "sql/debug_lo_misc.h"
#include "sql/debug_lo_parser.h"
#include "sql/debug_lo_scanner.h"
#include "sql/debug_lock_order.h"
#include "mysql/components/services/log_builtins.h"
#include "mysqld_error.h"

class LO_graph;

void process_arc(LO_parser_param *p,
  char *from,
  char *state,
  char *to,
  bool recursive,
  char *operation,
  int flags,
  char *constraint,
  char *comment);

void process_bind(LO_parser_param *p,
  char *from,
  char *to,
  int flags);

void process_node(LO_parser_param *p,
  char *node,
  char *operation,
  int flags);

void LOCK_ORDER_error(YYLTYPE *yyloc, LO_parser_param *p, const char* msg);

#define YYLEX_PARAM param->m_scanner

%}

%pure-parser
%locations

%lex-param { yyscan_t YYLEX_PARAM }
%parse-param { struct LO_parser_param *param }

%union
{
  char *m_str;
  int m_flags;
}

%start graph

%expect 0

%token ARC_SYM
%token BIND_SYM
%token COMMENT_SYM
%token CONSTRAINT_SYM
%token DEBUG_SYM
%token FLAGS_SYM
%token FROM_SYM
%token IGNORED_SYM
%token LOOP_SYM
%token NODE_SYM
%token OP_SYM
%token RECURSIVE_SYM
%token STATE_SYM
%token <m_str> STR_SYM
%token TO_SYM
%token TRACE_SYM
%token UNFAIR_SYM

%type <m_flags> opt_arc_flags;
%type <m_flags> arc_flags;
%type <m_flags> arc_flag;
%type <m_flags> opt_bind_flags;
%type <m_flags> bind_flags;
%type <m_flags> bind_flag;
%type <m_flags> node_flag;
%type <m_str> opt_comment;
%type <m_str> opt_constraint;
%type <m_str> opt_state;

%%

graph:
      list_of_declarations
    ;

list_of_declarations:
      declaration
    | list_of_declarations declaration
    ;

opt_arc_flags:
      /* empty */
      {
        $$ = 0;
      }
    | FLAGS_SYM arc_flags
      {
        $$ = $2;
      }
    ;

arc_flags:
      arc_flags arc_flag
      {
        $$ = $1 | $2;
      }
    | arc_flag
      {
        $$ = $1;
      }
    ;

arc_flag:
      TRACE_SYM
      {
        $$ = LO_FLAG_TRACE;
      }
    | DEBUG_SYM
      {
        $$ = LO_FLAG_DEBUG;
      }
    | LOOP_SYM
      {
        $$ = LO_FLAG_LOOP;
      }
    | IGNORED_SYM
      {
        $$ = LO_FLAG_IGNORED;
      }
    ;

opt_bind_flags:
      /* empty */
      {
        $$ = 0;
      }
    | FLAGS_SYM bind_flags
      {
        $$ = $2;
      }
    ;

bind_flags:
      bind_flags bind_flag
      {
        $$ = $1 | $2;
      }
    | bind_flag
      {
        $$ = $1;
      }
    ;

bind_flag:
      UNFAIR_SYM
      {
        $$ = LO_FLAG_UNFAIR;
      }
    ;

node_flag:
      IGNORED_SYM
      {
        $$ = LO_FLAG_IGNORED;
      }
    ;

opt_state:
      /* empty */
      {
        $$ = nullptr;
      }
    | STATE_SYM STR_SYM
      {
        $$ = $2;
      }
    ;

opt_constraint:
      /* empty */
      {
        $$ = nullptr;
      }
    | CONSTRAINT_SYM STR_SYM
      {
        $$ = $2;
      }
    ;

opt_comment:
      /* empty */
      {
        $$ = nullptr;
      }
    | COMMENT_SYM STR_SYM
      {
        $$ = $2;
      }
    ;

declaration:
      arc_declaration
    | bind_declaration
    | node_declaration
    ;

/*
  Note: can not use the [from] and $from syntax,
  because this requires a recent bison.
*/

arc_declaration:
      ARC_SYM
      FROM_SYM
      STR_SYM /* [from] = 3 */
      opt_state /* [state] = 4 */
      TO_SYM
      STR_SYM /* [to] = 6 */
      opt_arc_flags /* [flags] = 7 */
      opt_constraint /* [constraint] = 8 */
      opt_comment /* [comment] = 9 */
      {
        process_arc(param, $3, $4, $6, false, nullptr, $7, $8, $9);
      }
    | ARC_SYM
      FROM_SYM
      STR_SYM /* [from] = 3 */
      opt_state /* [state] = 4 */
      TO_SYM
      STR_SYM /* [to] = 6 */
      OP_SYM
      STR_SYM /* [op] = 8 */
      opt_arc_flags /* [flags] = 9 */
      opt_constraint /* [constraint] = 10 */
      opt_comment /* [comment] = 11 */
      {
        process_arc(param, $3, $4, $6, false, $8, $9, $10, $11);
      }
    | ARC_SYM
      FROM_SYM
      STR_SYM /* [from] = 3 */
      opt_state /* [state] = 4 */
      TO_SYM
      STR_SYM /* [to] = 6 */
      RECURSIVE_SYM
      OP_SYM
      STR_SYM /* [op] = 9 */
      opt_arc_flags /* [flags] = 10 */
      opt_constraint /* [constraint] = 11 */
      opt_comment /* [comment] == 12 */
      {
        process_arc(param, $3, $4, $6, true, $9, $10, $11, $12);
      }
    ;

bind_declaration:
      BIND_SYM
      STR_SYM /* [from] = 2 */
      TO_SYM
      STR_SYM /* [to] = 4 */
      opt_bind_flags /* [flags] = 5 */
      {
        process_bind(param, $2, $4, $5);
      }
    ;

node_declaration:
      NODE_SYM
      STR_SYM /* [node] = 2 */
      node_flag /* [flag] = 3 */
      {
        process_node(param, $2, nullptr, $3);
      }
    | NODE_SYM
      STR_SYM /* [node] = 2 */
      OP_SYM
      STR_SYM /* [op] = 4 */
      node_flag /* [flag] = 5 */
      {
        process_node(param, $2, $4, $5);
      }
    ;

%%

void process_arc(LO_parser_param *p,
  char *from,
  char *state,
  char *to,
  bool recursive,
  char *operation,
  int flags,
  char *constraint,
  char *comment)
{
  LO_authorised_arc arc;
  arc.m_from_name = from;
  arc.m_from_state = state;
  arc.m_to_name = to;
  arc.m_op_recursive = recursive;
  arc.m_to_operation = operation;
  arc.m_flags = flags;
  arc.m_constraint = constraint;
  arc.m_comment = comment;
  LO_add_authorised_arc(p->m_graph, &arc);
}

void process_bind(LO_parser_param *p,
  char *from,
  char *to,
  int flags)
{
  LO_authorised_arc arc;
  arc.m_from_name = from;
  arc.m_from_state = nullptr;
  arc.m_to_name = to;
  arc.m_op_recursive = false;
  arc.m_to_operation = nullptr;
  arc.m_flags = LO_FLAG_BIND | flags;
  arc.m_constraint = nullptr;
  arc.m_comment = nullptr;
  LO_add_authorised_arc(p->m_graph, &arc);
}

void process_node(LO_parser_param *p,
  char *node,
  char *operation,
  int flags)
{
  LO_node_properties prop;
  prop.m_name = node;
  prop.m_operation = operation;
  prop.m_flags = flags;
  LO_add_node_properties(p->m_graph, &prop);
}

void LOCK_ORDER_error(YYLTYPE * yylloc_param, LO_parser_param * p, const char* msg)
{
  /* message format = "Lock order dependencies file <%s> (%d:%d) - (%d:%d) : %s" */
  LogErr(ERROR_LEVEL, ER_LOCK_ORDER_DEPENDENCIES_SYNTAX,
    p->m_filename,
    yylloc_param->first_line,
    yylloc_param->first_column,
    yylloc_param->last_line,
    yylloc_param->last_column,
    msg);
}

