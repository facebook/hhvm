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

#include "hphp/compiler/statement/break_statement.h"
#include "hphp/compiler/expression/scalar_expression.h"
#include "hphp/runtime/base/complex-types.h"

using namespace HPHP;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

BreakStatement::BreakStatement
(STATEMENT_CONSTRUCTOR_BASE_PARAMETERS, uint64_t depth)
  : Statement(STATEMENT_CONSTRUCTOR_BASE_PARAMETER_VALUES),
    m_depth(depth) {
  m_name = "break";
}

BreakStatement::BreakStatement
(STATEMENT_CONSTRUCTOR_PARAMETERS, uint64_t depth)
  : Statement(STATEMENT_CONSTRUCTOR_PARAMETER_VALUES(BreakStatement)),
    m_depth(depth) {
  m_name = "break";
}

StatementPtr BreakStatement::clone() {
  BreakStatementPtr stmt(new BreakStatement(*this));
  stmt->m_depth = m_depth;
  return stmt;
}

///////////////////////////////////////////////////////////////////////////////
// parser functions

///////////////////////////////////////////////////////////////////////////////
// static analysis functions

void BreakStatement::analyzeProgram(AnalysisResultPtr ar) {
}

ConstructPtr BreakStatement::getNthKid(int n) const {
  always_assert(false);
}

int BreakStatement::getKidCount() const {
  return 0;
}

void BreakStatement::setNthKid(int n, ConstructPtr cp) {
  always_assert(false);
}

uint64_t BreakStatement::getDepth() {
  return m_depth;
}

StatementPtr BreakStatement::preOptimize(AnalysisResultConstPtr ar) {
  return StatementPtr();
}

void BreakStatement::inferTypes(AnalysisResultPtr ar) {
}

///////////////////////////////////////////////////////////////////////////////

void BreakStatement::outputCodeModel(CodeGenerator &cg) {
  if (strncmp(m_name, "break", 5)) {
    cg.printObjectHeader("BreakStatement", 2);
  } else {
    cg.printObjectHeader("ContinueStatement", 2);
  }
  cg.printPropertyHeader("depth");
  cg.printValue((int)m_depth);
  cg.printPropertyHeader("sourceLocation");
  cg.printLocation(this->getLocation());
  cg.printObjectFooter();
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void BreakStatement::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  if (m_depth != 1) {
    cg_printf("%s %" PRIu64 ";\n", m_name, m_depth);
  } else {
    cg_printf("%s;\n", m_name);
  }
}
