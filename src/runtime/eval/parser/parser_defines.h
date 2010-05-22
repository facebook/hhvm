/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#ifndef __EVAL_PARSER_DEFINES_H__
#define __EVAL_PARSER_DEFINES_H__

/* gross but necessary for ylmm */
#define yyparse         eval_parse
#define yylex           eval_lex
#define yyerror         eval_error
#define yylval          eval_lval
#define yychar          eval_char
#define yydebug         eval_debug
#define yynerrs         eval_nerrs

#define yy_create_buffer eval__create_buffer
#define yy_delete_buffer eval__delete_buffer
#define yy_flex_debug eval__flex_debug
#define yy_init_buffer eval__init_buffer
#define yy_flush_buffer eval__flush_buffer
#define yy_load_buffer_state eval__load_buffer_state
#define yy_switch_to_buffer eval__switch_to_buffer
#define yyin eval_in
#define yyleng eval_leng
#define yylineno eval_lineno
#define yyout eval_out
#define yyrestart eval_restart
#define yytext eval_text
#define yywrap eval_wrap
#define yyalloc eval_alloc
#define yyrealloc eval_realloc
#define yyfree eval_free

void _eval_scanner_reset();

#endif /* __EVAL_PARSER_DEFINES_H__ */

