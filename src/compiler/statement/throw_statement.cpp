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

#include <compiler/statement/throw_statement.h>

using namespace HPHP;
using namespace std;
using namespace boost;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

ThrowStatement::ThrowStatement
(STATEMENT_CONSTRUCTOR_PARAMETERS, ExpressionPtr exp)
  : Statement(STATEMENT_CONSTRUCTOR_PARAMETER_VALUES), m_exp(exp) {
}

StatementPtr ThrowStatement::clone() {
  ThrowStatementPtr stmt(new ThrowStatement(*this));
  stmt->m_exp = Clone(m_exp);
  return stmt;
}

///////////////////////////////////////////////////////////////////////////////
// parser functions

///////////////////////////////////////////////////////////////////////////////
// static analysis functions

void ThrowStatement::analyzeProgramImpl(AnalysisResultPtr ar) {
  m_exp->analyzeProgram(ar);
}

ConstructPtr ThrowStatement::getNthKid(int n) const {
  switch (n) {
    case 0:
      return m_exp;
    default:
      ASSERT(false);
      break;
  }
  return ConstructPtr();
}

int ThrowStatement::getKidCount() const {
  return 1;
}

void ThrowStatement::setNthKid(int n, ConstructPtr cp) {
  switch (n) {
    case 0:
      m_exp = boost::dynamic_pointer_cast<Expression>(cp);
      break;
    default:
      ASSERT(false);
      break;
  }
}

StatementPtr ThrowStatement::preOptimize(AnalysisResultPtr ar) {
  ar->preOptimize(m_exp);
  return StatementPtr();
}

StatementPtr ThrowStatement::postOptimize(AnalysisResultPtr ar) {
  ar->postOptimize(m_exp);
  return StatementPtr();
}

void ThrowStatement::inferTypes(AnalysisResultPtr ar) {
  m_exp->inferAndCheck(ar, NEW_TYPE(Object), true);
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void ThrowStatement::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  cg.printf("throw ");
  m_exp->outputPHP(cg, ar);
  cg.printf(";\n");
}

void ThrowStatement::outputCPPImpl(CodeGenerator &cg, AnalysisResultPtr ar) {
  m_exp->outputCPPBegin(cg, ar);
  cg.printf("throw_exception(");
  m_exp->outputCPP(cg, ar);
  cg.printf(");\n");
  m_exp->outputCPPEnd(cg, ar);
}
