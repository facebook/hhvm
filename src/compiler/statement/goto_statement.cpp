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

#include <compiler/statement/goto_statement.h>

using namespace HPHP;
using namespace std;
using namespace boost;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

GotoStatement::GotoStatement
(STATEMENT_CONSTRUCTOR_PARAMETERS, const std::string &label)
  : Statement(STATEMENT_CONSTRUCTOR_PARAMETER_VALUES), m_label(label) {
}

StatementPtr GotoStatement::clone() {
  GotoStatementPtr stmt(new GotoStatement(*this));
  stmt->m_label = m_label;
  return stmt;
}

///////////////////////////////////////////////////////////////////////////////
// parser functions

///////////////////////////////////////////////////////////////////////////////
// static analysis functions

void GotoStatement::analyzeProgramImpl(AnalysisResultPtr ar) {
}

ConstructPtr GotoStatement::getNthKid(int n) const {
  switch (n) {
    default:
      ASSERT(false);
      break;
  }
  return ConstructPtr();
}

int GotoStatement::getKidCount() const {
  return 0;
}

void GotoStatement::setNthKid(int n, ConstructPtr cp) {
  switch (n) {
    default:
      ASSERT(false);
      break;
  }
}

void GotoStatement::inferTypes(AnalysisResultPtr ar) {
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void GotoStatement::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  cg_printf("goto %s;\n", m_label.c_str());
}

void GotoStatement::outputCPPImpl(CodeGenerator &cg, AnalysisResultPtr ar) {
  cg_printf("goto %s;\n", m_label.c_str());
}
