/*
   Copyright (c) 2015, 2020, Oracle and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2.0,
   as published by the Free Software Foundation.

   This program is also distributed with certain software (including
   but not limited to OpenSSL) that is licensed under separate terms,
   as designated in a particular file or component or in included license
   documentation.  The authors of MySQL hereby grant you an additional
   permission to link the program and your derivative works with the
   separately licensed software that they have included with MySQL.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

/*
  Optimizer hint parser grammar
*/

%{
#include "my_inttypes.h"
#include "sql/derror.h"
#include "sql/item_subselect.h"
#include "sql/parse_tree_helpers.h"  // check_resource_group_name_len
#include "sql/parse_tree_hints.h"
#include "sql/parser_yystype.h"
#include "sql/sql_class.h"
#include "sql/sql_const.h"
#include "sql/sql_lex_hints.h"

#define NEW_PTN new (thd->mem_root)

static bool parse_int(longlong *to, const char *from, size_t from_length)
{
  int error;
  const char *end= from + from_length;
  *to= my_strtoll10(from, &end, &error);
  return error != 0 || end != from + from_length;
}

%}

%pure-parser

%parse-param { class THD *thd }
%parse-param { class Hint_scanner *scanner }
%parse-param { class PT_hint_list **ret }

%lex-param { class Hint_scanner *scanner }

%expect 0


/* Hint keyword tokens */

%token MAX_EXECUTION_TIME_HINT 1000
%token RESOURCE_GROUP_HINT 1001

%token BKA_HINT 1002
%token BNL_HINT 1003
%token DUPSWEEDOUT_HINT 1004
%token FIRSTMATCH_HINT 1005
%token INTOEXISTS_HINT 1006
%token LOOSESCAN_HINT 1007
%token MATERIALIZATION_HINT 1008
%token NO_BKA_HINT 1009
%token NO_BNL_HINT 1010
%token NO_ICP_HINT 1011
%token NO_MRR_HINT 1012
%token NO_RANGE_OPTIMIZATION_HINT 1013
%token NO_SEMIJOIN_HINT 1014
%token MRR_HINT 1015
%token QB_NAME_HINT 1016
%token SEMIJOIN_HINT 1017
%token SUBQUERY_HINT 1018
%token DERIVED_MERGE_HINT 1019
%token NO_DERIVED_MERGE_HINT 1020
%token JOIN_PREFIX_HINT 1021
%token JOIN_SUFFIX_HINT 1022
%token JOIN_ORDER_HINT 1023
%token JOIN_FIXED_ORDER_HINT 1024
%token INDEX_MERGE_HINT 1025
%token NO_INDEX_MERGE_HINT 1026
%token SET_VAR_HINT 1027
%token SKIP_SCAN_HINT 1028
%token NO_SKIP_SCAN_HINT 1029
%token HASH_JOIN_HINT 1030
%token NO_HASH_JOIN_HINT 1031

/* Other tokens */

%token HINT_ARG_NUMBER 1032
%token HINT_ARG_IDENT 1033
%token HINT_ARG_QB_NAME 1034
%token HINT_ARG_TEXT 1035
%token HINT_IDENT_OR_NUMBER_WITH_SCALE 1036

%token HINT_CLOSE 1037
%token HINT_ERROR 1038

%token INDEX_HINT 1039
%token NO_INDEX_HINT 1040
%token JOIN_INDEX_HINT 1041
%token NO_JOIN_INDEX_HINT 1042
%token GROUP_INDEX_HINT 1043
%token NO_GROUP_INDEX_HINT 1044
%token ORDER_INDEX_HINT 1045
%token NO_ORDER_INDEX_HINT 1046

%token GROUP_BY_LIS_HINT 1047
%token NO_GROUP_BY_LIS_HINT 1048

/*
  Please add new tokens right above this line.

  To make DIGESTS stable, it is desirable to avoid changing token number values.
*/

/* Types */
%type <hint_type>
  key_level_hint_type_on
  key_level_hint_type_off
  table_level_hint_type_on
  table_level_hint_type_off

%type <hint>
  hint
  max_execution_time_hint
  index_level_hint
  table_level_hint
  qb_level_hint
  qb_name_hint
  set_var_hint
  resource_group_hint

%type <hint_list> hint_list

%type <lexer.hint_string> hint_param_index

%type <hint_param_index_list> hint_param_index_list opt_hint_param_index_list

%type <hint_param_table>
  hint_param_table
  hint_param_table_ext
  hint_param_table_empty_qb

%type <hint_param_table_list>
  hint_param_table_list
  opt_hint_param_table_list
  hint_param_table_list_empty_qb
  opt_hint_param_table_list_empty_qb

%type <lexer.hint_string>
  HINT_ARG_IDENT
  HINT_ARG_NUMBER
  HINT_ARG_QB_NAME
  HINT_ARG_TEXT
  HINT_IDENT_OR_NUMBER_WITH_SCALE
  MAX_EXECUTION_TIME_HINT
  opt_qb_name
  set_var_ident
  set_var_text_value

%type <item>
  set_var_num_item
  set_var_string_item
  set_var_arg

%type <ulong_num>
  semijoin_strategy semijoin_strategies
  subquery_strategy

%%


start:
          hint_list HINT_CLOSE
          { *ret= $1; }
        | hint_list error HINT_CLOSE
          { *ret= $1; }
        | error HINT_CLOSE
          { *ret= NULL; }
        ;

hint_list:
          hint
          {
            $$= NEW_PTN PT_hint_list(thd->mem_root);
            if ($$ == NULL || $$->push_back($1))
              YYABORT; // OOM
          }
        | hint_list hint
          {
            $1->push_back($2);
            $$= $1;
          }
        ;

hint:
          index_level_hint
        | table_level_hint
        | qb_level_hint
        | qb_name_hint
        | max_execution_time_hint
        | set_var_hint
        | resource_group_hint
        ;


max_execution_time_hint:
          MAX_EXECUTION_TIME_HINT '(' HINT_ARG_NUMBER ')'
          {
            longlong n;
            if (parse_int(&n, $3.str, $3.length) || n > UINT_MAX32)
            {
              scanner->syntax_warning(ER_THD(thd,
                                             ER_WARN_BAD_MAX_EXECUTION_TIME));
              $$= NULL;
            }
            else
            {
              $$= NEW_PTN PT_hint_max_execution_time(n);
              if ($$ == NULL)
                YYABORT; // OOM
            }
          }
        ;


opt_hint_param_table_list:
          /* empty */ { $$.init(thd->mem_root); }
        | hint_param_table_list
        ;

hint_param_table_list:
          hint_param_table
          {
            $$.init(thd->mem_root);
            if ($$.push_back($1))
              YYABORT; // OOM
          }
        | hint_param_table_list ',' hint_param_table
          {
            if ($1.push_back($3))
              YYABORT; // OOM
            $$= $1;
          }
        ;

opt_hint_param_table_list_empty_qb:
          /* empty */ { $$.init(thd->mem_root); }
        | hint_param_table_list_empty_qb
        ;

hint_param_table_list_empty_qb:
          hint_param_table_empty_qb
          {
            $$.init(thd->mem_root);
            if ($$.push_back($1))
              YYABORT; // OOM
          }
        | hint_param_table_list_empty_qb ',' hint_param_table_empty_qb
          {
            if ($1.push_back($3))
              YYABORT; // OOM
            $$= $1;
          }
        ;

opt_hint_param_index_list:
          /* empty */ { $$.init(thd->mem_root); }
        | hint_param_index_list
        ;

hint_param_index_list:
          hint_param_index
          {
            $$.init(thd->mem_root);
            if ($$.push_back($1))
              YYABORT; // OOM
          }
        | hint_param_index_list ',' hint_param_index
          {
            if ($1.push_back($3))
              YYABORT; // OOM
            $$= $1;
          }
        ;

hint_param_index:
          HINT_ARG_IDENT
        ;

hint_param_table_empty_qb:
          HINT_ARG_IDENT
          {
            $$.table= $1;
            $$.opt_query_block= NULL_CSTR;
          }
        ;

hint_param_table:
          HINT_ARG_IDENT opt_qb_name
          {
            $$.table= $1;
            $$.opt_query_block= $2;
          }
        ;

hint_param_table_ext:
          hint_param_table
        | HINT_ARG_QB_NAME HINT_ARG_IDENT
          {
            $$.table= $2;
            $$.opt_query_block= $1;
          }
        ;

opt_qb_name:
          /* empty */ { $$= NULL_CSTR; }
        | HINT_ARG_QB_NAME
        ;

qb_level_hint:
          SEMIJOIN_HINT '(' opt_qb_name semijoin_strategies ')'
          {
            $$= NEW_PTN PT_qb_level_hint($3, true, SEMIJOIN_HINT_ENUM, $4);
            if ($$ == NULL)
              YYABORT; // OOM
          }
          |
          NO_SEMIJOIN_HINT '(' opt_qb_name semijoin_strategies ')'
          {
            $$= NEW_PTN PT_qb_level_hint($3, false, SEMIJOIN_HINT_ENUM, $4);
            if ($$ == NULL)
              YYABORT; // OOM
          }
          |
          SUBQUERY_HINT '(' opt_qb_name subquery_strategy ')'
          {
            $$= NEW_PTN PT_qb_level_hint($3, true, SUBQUERY_HINT_ENUM, $4);
            if ($$ == NULL)
              YYABORT; // OOM
          }
          |
          JOIN_PREFIX_HINT '(' opt_hint_param_table_list ')'
          {
            $$= NEW_PTN PT_qb_level_hint(NULL_CSTR, true, JOIN_PREFIX_HINT_ENUM, $3);
            if ($$ == NULL)
              YYABORT; // OOM
          }
          |
          JOIN_PREFIX_HINT '(' HINT_ARG_QB_NAME opt_hint_param_table_list_empty_qb ')'
          {
            $$= NEW_PTN PT_qb_level_hint($3, true, JOIN_PREFIX_HINT_ENUM, $4);
            if ($$ == NULL)
              YYABORT; // OOM
          }
          |
          JOIN_SUFFIX_HINT '(' opt_hint_param_table_list ')'
          {
            $$= NEW_PTN PT_qb_level_hint(NULL_CSTR, true, JOIN_SUFFIX_HINT_ENUM, $3);
            if ($$ == NULL)
              YYABORT; // OOM
          }
          |
          JOIN_SUFFIX_HINT '(' HINT_ARG_QB_NAME opt_hint_param_table_list_empty_qb ')'
          {
            $$= NEW_PTN PT_qb_level_hint($3, true, JOIN_SUFFIX_HINT_ENUM, $4);
            if ($$ == NULL)
              YYABORT; // OOM
          }
          |
          JOIN_ORDER_HINT '(' opt_hint_param_table_list ')'
          {
            $$= NEW_PTN PT_qb_level_hint(NULL_CSTR, true, JOIN_ORDER_HINT_ENUM, $3);
            if ($$ == NULL)
              YYABORT; // OOM
          }
          |
          JOIN_ORDER_HINT '(' HINT_ARG_QB_NAME opt_hint_param_table_list_empty_qb ')'
          {
            $$= NEW_PTN PT_qb_level_hint($3, true, JOIN_ORDER_HINT_ENUM, $4);
            if ($$ == NULL)
              YYABORT; // OOM
          }
          |
          JOIN_FIXED_ORDER_HINT '(' opt_qb_name  ')'
          {
            $$= NEW_PTN PT_qb_level_hint($3, true, JOIN_FIXED_ORDER_HINT_ENUM, 0);
            if ($$ == NULL)
              YYABORT; // OOM
          }
          ;

semijoin_strategies:
          /* empty */ { $$= 0; }
	| semijoin_strategy
          {
            $$= $1;
          }
        | semijoin_strategies ',' semijoin_strategy
          {
            $$= $1 | $3;
          }
        ;

semijoin_strategy:
          FIRSTMATCH_HINT      { $$= OPTIMIZER_SWITCH_FIRSTMATCH; }
        | LOOSESCAN_HINT       { $$= OPTIMIZER_SWITCH_LOOSE_SCAN; }
        | MATERIALIZATION_HINT { $$= OPTIMIZER_SWITCH_MATERIALIZATION; }
        | DUPSWEEDOUT_HINT     { $$= OPTIMIZER_SWITCH_DUPSWEEDOUT; }
        ;

subquery_strategy:
          MATERIALIZATION_HINT { $$=
                                   static_cast<long>(SubqueryExecMethod::EXEC_MATERIALIZATION); }
        | INTOEXISTS_HINT      { $$= static_cast<long>(SubqueryExecMethod::EXEC_EXISTS); }
        ;


table_level_hint:
          table_level_hint_type_on '(' opt_hint_param_table_list ')'
          {
            $$= NEW_PTN PT_table_level_hint(NULL_CSTR, $3, true, $1);
            if ($$ == NULL)
              YYABORT; // OOM
          }
        | table_level_hint_type_on
          '(' HINT_ARG_QB_NAME opt_hint_param_table_list_empty_qb ')'
          {
            $$= NEW_PTN PT_table_level_hint($3, $4, true, $1);
            if ($$ == NULL)
              YYABORT; // OOM
          }
        | table_level_hint_type_off '(' opt_hint_param_table_list ')'
          {
            $$= NEW_PTN PT_table_level_hint(NULL_CSTR, $3, false, $1);
            if ($$ == NULL)
              YYABORT; // OOM
          }
        | table_level_hint_type_off
          '(' HINT_ARG_QB_NAME opt_hint_param_table_list_empty_qb ')'
          {
            $$= NEW_PTN PT_table_level_hint($3, $4, false, $1);
            if ($$ == NULL)
              YYABORT; // OOM
          }
        ;

index_level_hint:
          key_level_hint_type_on
          '(' hint_param_table_ext opt_hint_param_index_list ')'
          {
            $$= NEW_PTN PT_key_level_hint($3, $4, true, $1);
            if ($$ == NULL)
              YYABORT; // OOM
          }
        | key_level_hint_type_off
          '(' hint_param_table_ext opt_hint_param_index_list ')'
          {
            $$= NEW_PTN PT_key_level_hint($3, $4, false, $1);
            if ($$ == NULL)
              YYABORT; // OOM
          }
        ;

table_level_hint_type_on:
          BKA_HINT
          {
            $$= BKA_HINT_ENUM;
          }
        | BNL_HINT
          {
            $$= BNL_HINT_ENUM;
          }
        | HASH_JOIN_HINT
          {
            $$= HASH_JOIN_HINT_ENUM;
          }
        | DERIVED_MERGE_HINT
          {
            $$= DERIVED_MERGE_HINT_ENUM;
          }
        ;

table_level_hint_type_off:
          NO_BKA_HINT
          {
            $$= BKA_HINT_ENUM;
          }
        | NO_BNL_HINT
          {
            $$= BNL_HINT_ENUM;
          }
        | NO_HASH_JOIN_HINT
          {
            $$= HASH_JOIN_HINT_ENUM;
          }
        | NO_DERIVED_MERGE_HINT
          {
            $$= DERIVED_MERGE_HINT_ENUM;
          }
        ;

key_level_hint_type_on:
          MRR_HINT
          {
            $$= MRR_HINT_ENUM;
          }
        | NO_RANGE_OPTIMIZATION_HINT
          {
            $$= NO_RANGE_HINT_ENUM;
          }
        | INDEX_MERGE_HINT
          {
            $$= INDEX_MERGE_HINT_ENUM;
          }
        | SKIP_SCAN_HINT
          {
            $$= SKIP_SCAN_HINT_ENUM;
          }
        | GROUP_BY_LIS_HINT
          {
            $$= GROUP_BY_LIS_HINT_ENUM;
          }
        | INDEX_HINT
          {
            $$= INDEX_HINT_ENUM;
          }
        | JOIN_INDEX_HINT
          {
            $$= JOIN_INDEX_HINT_ENUM;
          }
        | GROUP_INDEX_HINT
          {
            $$= GROUP_INDEX_HINT_ENUM;
          }
        | ORDER_INDEX_HINT
          {
            $$= ORDER_INDEX_HINT_ENUM;
          }
        ;

key_level_hint_type_off:
          NO_ICP_HINT
          {
            $$= ICP_HINT_ENUM;
          }
        | NO_MRR_HINT
          {
            $$= MRR_HINT_ENUM;
          }
        | NO_INDEX_MERGE_HINT
          {
            $$= INDEX_MERGE_HINT_ENUM;
          }
        | NO_SKIP_SCAN_HINT
          {
            $$= SKIP_SCAN_HINT_ENUM;
          }
        | NO_GROUP_BY_LIS_HINT
          {
            $$= GROUP_BY_LIS_HINT_ENUM;
          }
        | NO_INDEX_HINT
          {
            $$= INDEX_HINT_ENUM;
          }
        | NO_JOIN_INDEX_HINT
          {
            $$= JOIN_INDEX_HINT_ENUM;
          }
        | NO_GROUP_INDEX_HINT
          {
            $$= GROUP_INDEX_HINT_ENUM;
          }
        | NO_ORDER_INDEX_HINT
          {
            $$= ORDER_INDEX_HINT_ENUM;
          }
        ;

qb_name_hint:
          QB_NAME_HINT '(' HINT_ARG_IDENT ')'
          {
            $$= NEW_PTN PT_hint_qb_name($3);
            if ($$ == NULL)
              YYABORT; // OOM
          }
        ;

set_var_hint:
          SET_VAR_HINT '(' set_var_ident '=' set_var_arg ')'
          {
            $$= NEW_PTN PT_hint_sys_var($3, $5);
            if ($$ == NULL)
              YYABORT; // OOM
          }
        ;

resource_group_hint:
         RESOURCE_GROUP_HINT '(' HINT_ARG_IDENT ')'
         {
           if (check_resource_group_name_len($3, Sql_condition::SL_WARNING))
             YYERROR;

           $$= NEW_PTN PT_hint_resource_group($3);
           if ($$ == nullptr)
              YYABORT; // OOM
         }
       ;

set_var_ident:
          HINT_ARG_IDENT
        | MAX_EXECUTION_TIME_HINT
        ;

set_var_num_item:
          HINT_ARG_NUMBER
          {
            longlong n;
            if (parse_int(&n, $1.str, $1.length))
            {
              scanner->syntax_warning(ER_THD(thd, ER_WRONG_SIZE_NUMBER));
              $$= NULL;
            }
            else
            {
              $$= NEW_PTN Item_int((ulonglong)n);
              if ($$ == NULL)
                YYABORT; // OOM
            }
          }
        | HINT_IDENT_OR_NUMBER_WITH_SCALE
          {
            longlong n;
            if (parse_int(&n, $1.str, $1.length - 1))
            {
              scanner->syntax_warning(ER_THD(thd, ER_WRONG_SIZE_NUMBER));
              $$= NULL;
            }
            else
            {
              int multiplier;
              switch ($1.str[$1.length - 1]) {
              case 'K': multiplier= 1024; break;
              case 'M': multiplier= 1024 * 1024; break;
              case 'G': multiplier= 1024 * 1024 * 1024; break;
              default:
                DBUG_ASSERT(0); // should not happen
                YYABORT;        // for sure
              }
              if (1.0L * n * multiplier > LLONG_MAX)
              {
                scanner->syntax_warning(ER_THD(thd, ER_WRONG_SIZE_NUMBER));
                $$= NULL;
              }
              else
              {
                $$= NEW_PTN Item_int((ulonglong)n * multiplier);
                if ($$ == NULL)
                  YYABORT; // OOM
              }
            }
          }
        ;

set_var_text_value:
        HINT_ARG_IDENT
        | HINT_ARG_TEXT
        ;

set_var_string_item:
        set_var_text_value
        {
          $$= NEW_PTN Item_string($1.str, $1.length, thd->charset());
          if ($$ == NULL)
            YYABORT; // OOM
        }

set_var_arg:
    set_var_string_item
    | set_var_num_item
    ;
