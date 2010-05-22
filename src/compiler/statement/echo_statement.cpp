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

#include <compiler/statement/echo_statement.h>
#include <compiler/expression/expression_list.h>

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

void EchoStatement::analyzeProgramImpl(AnalysisResultPtr ar) {
  for (int i = 0; i < m_exp->getCount(); i++) {
    (*m_exp)[i]->analyzeProgram(ar);
  }
}

ConstructPtr EchoStatement::getNthKid(int n) const {
  switch (n) {
    case 0:
      return m_exp;
    default:
      ASSERT(false);
      break;
  }
  return ConstructPtr();
}

int EchoStatement::getKidCount() const {
  return 1;
}

void EchoStatement::setNthKid(int n, ConstructPtr cp) {
  switch (n) {
    case 0:
      m_exp = boost::dynamic_pointer_cast<ExpressionList>(cp);
      break;
    default:
      ASSERT(false);
      break;
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

void EchoStatement::outputCPPImpl(CodeGenerator &cg, AnalysisResultPtr ar) {
  if (m_exp->getCount() > 1) cg.indentBegin("{\n");
  for (int i = 0; i < m_exp->getCount(); i++) {
    (*m_exp)[i]->outputCPPBegin(cg, ar);
    cg.printf("echo(");
    (*m_exp)[i]->outputCPP(cg, ar);
    cg.printf(");\n");
    (*m_exp)[i]->outputCPPEnd(cg, ar);
  }
  if (m_exp->getCount() > 1) cg.indentEnd("}\n");
}
