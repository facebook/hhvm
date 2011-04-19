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

#include <compiler/statement/label_statement.h>

using namespace HPHP;
using namespace std;
using namespace boost;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

LabelStatement::LabelStatement
(STATEMENT_CONSTRUCTOR_PARAMETERS, const std::string &label)
  : Statement(STATEMENT_CONSTRUCTOR_PARAMETER_VALUES), m_label(label) {
}

StatementPtr LabelStatement::clone() {
  LabelStatementPtr stmt(new LabelStatement(*this));
  stmt->m_label = m_label;
  return stmt;
}

///////////////////////////////////////////////////////////////////////////////
// parser functions

///////////////////////////////////////////////////////////////////////////////
// static analysis functions

void LabelStatement::analyzeProgramImpl(AnalysisResultPtr ar) {
}

ConstructPtr LabelStatement::getNthKid(int n) const {
  switch (n) {
    default:
      ASSERT(false);
      break;
  }
  return ConstructPtr();
}

int LabelStatement::getKidCount() const {
  return 0;
}

void LabelStatement::setNthKid(int n, ConstructPtr cp) {
  switch (n) {
    default:
      ASSERT(false);
      break;
  }
}

void LabelStatement::inferTypes(AnalysisResultPtr ar) {
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void LabelStatement::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  cg_printf("%s:\n", m_label.c_str());
}

void LabelStatement::outputCPPImpl(CodeGenerator &cg, AnalysisResultPtr ar) {
  cg_printf("%s:;\n", m_label.c_str());
}
