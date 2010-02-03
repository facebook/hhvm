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

#include <lib/statement/echo_statement.h>
#include <lib/expression/expression_list.h>

using namespace HPHP;
using namespace std;
using namespace boost;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

EchoStatement::EchoStatement
(STATEMENT_CONSTRUCTOR_PARAMETERS, ExpressionListPtr exp)
  : Statement(STATEMENT_CONSTRUCTOR_PARAMETER_VALUES), m_exp(exp) {
}

StatementPtr EchoStatement::clone() {
  EchoStatementPtr stmt(new EchoStatement(*this));
  stmt->m_exp = Clone(m_exp);
  return stmt;
}

///////////////////////////////////////////////////////////////////////////////
// parser functions

///////////////////////////////////////////////////////////////////////////////
// static analysis functions

void EchoStatement::analyzeProgram(AnalysisResultPtr ar) {
  for (int i = 0; i < m_exp->getCount(); i++) {
    (*m_exp)[i]->analyzeProgram(ar);
  }
}

StatementPtr EchoStatement::preOptimize(AnalysisResultPtr ar) {
  ar->preOptimize(m_exp);
  return StatementPtr();
}

StatementPtr EchoStatement::postOptimize(AnalysisResultPtr ar) {
  ar->postOptimize(m_exp);
  return StatementPtr();
}

void EchoStatement::inferTypes(AnalysisResultPtr ar) {
  m_exp->inferAndCheck(ar, Type::String, false);
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void EchoStatement::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  cg.printf("echo ");
  m_exp->outputPHP(cg, ar);
  cg.printf(";\n");
}

void EchoStatement::outputCPP(CodeGenerator &cg, AnalysisResultPtr ar) {
  if (m_exp->getCount() > 1) cg.indentBegin("{\n");
  for (int i = 0; i < m_exp->getCount(); i++) {
    cg.printf("echo(");
    (*m_exp)[i]->outputCPP(cg, ar);
    cg.printf(");\n");
  }
  if (m_exp->getCount() > 1) cg.indentEnd("}\n");
}
