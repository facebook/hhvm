/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#ifndef incl_HPHP_PHP7_AST_INFO_H
#define incl_HPHP_PHP7_AST_INFO_H

#define AST_NODES(x) \
  x(ZVAL) \
  x(ZNODE) \
  x(FUNC_DECL) \
  x(CLOSURE) \
  x(METHOD) \
  x(CLASS) \
  x(ARG_LIST) \
  x(ARRAY) \
  x(ENCAPS_LIST) \
  x(EXPR_LIST) \
  x(STMT_LIST) \
  x(IF) \
  x(SWITCH_LIST) \
  x(CATCH_LIST) \
  x(PARAM_LIST) \
  x(CLOSURE_USES) \
  x(PROP_DECL) \
  x(CONST_DECL) \
  x(CLASS_CONST_DECL) \
  x(NAME_LIST) \
  x(TRAIT_ADAPTATIONS) \
  x(USE) \
  x(MAGIC_CONST) \
  x(TYPE) \
  x(VAR) \
  x(CONST) \
  x(UNPACK) \
  x(UNARY_PLUS) \
  x(UNARY_MINUS) \
  x(CAST) \
  x(EMPTY) \
  x(ISSET) \
  x(SILENCE) \
  x(SHELL_EXEC) \
  x(CLONE) \
  x(EXIT) \
  x(PRINT) \
  x(INCLUDE_OR_EVAL) \
  x(UNARY_OP) \
  x(PRE_INC) \
  x(PRE_DEC) \
  x(POST_INC) \
  x(POST_DEC) \
  x(YIELD_FROM) \
  x(GLOBAL) \
  x(UNSET) \
  x(RETURN) \
  x(LABEL) \
  x(REF) \
  x(HALT_COMPILER) \
  x(ECHO) \
  x(THROW) \
  x(GOTO) \
  x(BREAK) \
  x(CONTINUE) \
  x(DIM) \
  x(PROP) \
  x(STATIC_PROP) \
  x(CALL) \
  x(CLASS_CONST) \
  x(ASSIGN) \
  x(ASSIGN_REF) \
  x(ASSIGN_OP) \
  x(BINARY_OP) \
  x(GREATER) \
  x(GREATER_EQUAL) \
  x(AND) \
  x(OR) \
  x(ARRAY_ELEM) \
  x(NEW) \
  x(INSTANCEOF) \
  x(YIELD) \
  x(COALESCE) \
  x(STATIC) \
  x(WHILE) \
  x(DO_WHILE) \
  x(IF_ELEM) \
  x(SWITCH) \
  x(SWITCH_CASE) \
  x(DECLARE) \
  x(USE_TRAIT) \
  x(TRAIT_PRECEDENCE) \
  x(METHOD_REFERENCE) \
  x(NAMESPACE) \
  x(USE_ELEM) \
  x(TRAIT_ALIAS) \
  x(GROUP_USE) \
  x(METHOD_CALL) \
  x(STATIC_CALL) \
  x(CONDITIONAL) \
  x(TRY) \
  x(CATCH) \
  x(PARAM) \
  x(PROP_ELEM) \
  x(CONST_ELEM) \
  x(FOR) \
  x(FOREACH)

#endif // incl_HPHP_PHP7_AST_INFO_H
