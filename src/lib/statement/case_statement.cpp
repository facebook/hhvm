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

#include <lib/statement/case_statement.h>
#include <lib/expression/scalar_expression.h>
#include <lib/parser/hphp.tab.hpp>
#include <lib/option.h>

using namespace HPHP;
using namespace std;
using namespace boost;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

CaseStatement::CaseStatement
(STATEMENT_CONSTRUCTOR_PARAMETERS,
 ExpressionPtr condition, StatementPtr stmt)
  : Statement(STATEMENT_CONSTRUCTOR_PARAMETER_VALUES),
    m_condition(condition), m_stmt(stmt) {
}

StatementPtr CaseStatement::clone() {
  CaseStatementPtr stmt(new CaseStatement(*this));
  stmt->m_condition = Clone(m_condition);
  stmt->m_stmt = Clone(m_stmt);
  return stmt;
}

///////////////////////////////////////////////////////////////////////////////
// parser functions

///////////////////////////////////////////////////////////////////////////////
// static analysis functions

bool CaseStatement::isLiteralInteger() const {
  if (!m_condition->is(Expression::KindOfScalarExpression)) return false;
  ScalarExpressionPtr exp =
    dynamic_pointer_cast<ScalarExpression>(m_condition);
  return exp->isLiteralInteger();
}

bool CaseStatement::isLiteralString() const {
  if (!m_condition->is(Expression::KindOfScalarExpression)) return false;
  ScalarExpressionPtr exp =
    dynamic_pointer_cast<ScalarExpression>(m_condition);
  return exp->isLiteralString();
}

int64 CaseStatement::getLiteralInteger() const {
  ASSERT(m_condition->is(Expression::KindOfScalarExpression));
  ScalarExpressionPtr exp =
    dynamic_pointer_cast<ScalarExpression>(m_condition);
  return exp->getLiteralInteger();
}

void CaseStatement::analyzeProgram(AnalysisResultPtr ar) {
  if (m_condition) m_condition->analyzeProgram(ar);
  if (m_stmt) m_stmt->analyzeProgram(ar);
}

ConstructPtr CaseStatement::getNthKid(int n) const {
  switch (n) {
    case 0:
      return m_condition;
    case 1:
      return m_stmt;
    default:
      return ConstructPtr();
  }
  ASSERT(0);
}

int CaseStatement::getKidCount() const {
  return 2;
}

int CaseStatement::setNthKid(int n, ConstructPtr cp) {
  switch (n) {
    case 0:
      m_condition = boost::dynamic_pointer_cast<Expression>(cp);
      return 1;
    case 1:
      m_stmt = boost::dynamic_pointer_cast<Statement>(cp);
      return 1;
    default:
      return 0;
  }
  ASSERT(0);
}

StatementPtr CaseStatement::preOptimize(AnalysisResultPtr ar) {
  ar->preOptimize(m_condition);
  ar->preOptimize(m_stmt);
  return StatementPtr();
}

StatementPtr CaseStatement::postOptimize(AnalysisResultPtr ar) {
  ar->postOptimize(m_condition);
  ar->postOptimize(m_stmt);
  return StatementPtr();
}

void CaseStatement::inferTypes(AnalysisResultPtr ar) {
  ASSERT(false);
}

void CaseStatement::inferAndCheck(AnalysisResultPtr ar, TypePtr type,
                                  bool coerce) {
  if (m_condition) m_condition->inferAndCheck(ar, type, coerce);
  if (m_stmt) m_stmt->inferTypes(ar);
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void CaseStatement::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  if (m_condition) {
    cg.printf("case ");
    m_condition->outputPHP(cg, ar);
    cg.indentBegin(":\n");
  } else {
    cg.indentBegin("default:\n");
  }
  if (m_stmt) {
    m_stmt->outputPHP(cg, ar);
  }
  cg.indentEnd("");
}

void CaseStatement::outputCPP(CodeGenerator &cg, AnalysisResultPtr ar) {
  if (m_condition) {
    cg.printf("case ");
    m_condition->outputCPPImpl(cg, ar);
    cg.indentBegin(":\n");
  } else {
    cg.indentBegin("default:\n");
  }
  cg.indentBegin("{\n");
  if (m_stmt) {
    m_stmt->outputCPP(cg, ar);
  }
  cg.indentEnd("}\n");
  cg.indentEnd("");
}

void CaseStatement::outputCPPByNumber(CodeGenerator &cg, AnalysisResultPtr ar,
                                      int caseNum) {
  cg.indentBegin("case %d:\n", caseNum);
  cg.indentBegin("{\n");
  if (m_stmt) {
    m_stmt->outputCPP(cg, ar);
  }
  cg.indentEnd("}\n");
  cg.indentEnd("");
}

void CaseStatement::outputCPPAsIf(CodeGenerator &cg, AnalysisResultPtr ar,
                                  int varId, int caseVar, int caseNum) {
  if (m_condition) {
    cg.printf("if (equal(%s%d, (", Option::TempPrefix, varId);
    m_condition->outputCPP(cg, ar);
    cg.indentBegin("))) {\n");
  } else {
    cg.indentBegin("if (true) {\n");
  }
  cg.printf("%s%d = %d;\n", Option::TempPrefix, caseVar, caseNum);
  cg.indentEnd("}");
}
