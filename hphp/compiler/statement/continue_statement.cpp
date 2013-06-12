/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/compiler/statement/continue_statement.h"

using namespace HPHP;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

ContinueStatement::ContinueStatement
(STATEMENT_CONSTRUCTOR_PARAMETERS, uint64_t depth)
  : BreakStatement(STATEMENT_CONSTRUCTOR_PARAMETER_VALUES(ContinueStatement),
                   depth) {
  m_name = "continue";
}

StatementPtr ContinueStatement::clone() {
  ContinueStatementPtr stmt(new ContinueStatement(*this));
  stmt->m_depth = m_depth;
  return stmt;
}

///////////////////////////////////////////////////////////////////////////////
// parser functions

///////////////////////////////////////////////////////////////////////////////
// static analysis functions

///////////////////////////////////////////////////////////////////////////////
// code generation functions
