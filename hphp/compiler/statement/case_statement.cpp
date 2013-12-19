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

#include "hphp/compiler/statement/case_statement.h"
#include "hphp/compiler/expression/scalar_expression.h"
#include "hphp/parser/hphp.tab.hpp"
#include "hphp/compiler/option.h"

using namespace HPHP;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

CaseStatement::CaseStatement
(STATEMENT_CONSTRUCTOR_PARAMETERS,
 ExpressionPtr condition, StatementPtr stmt)
  : Statement(STATEMENT_CONSTRUCTOR_PARAMETER_VALUES(CaseStatement)),
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

int64_t CaseStatement::getLiteralInteger() const {
  assert(m_condition->is(Expression::KindOfScalarExpression));
  ScalarExpressionPtr exp =
    dynamic_pointer_cast<ScalarExpression>(m_condition);
  return exp->getLiteralInteger();
}

string CaseStatement::getLiteralString() const {
  assert(m_condition->is(Expression::KindOfScalarExpression));
  ScalarExpressionPtr exp =
    dynamic_pointer_cast<ScalarExpression>(m_condition);
  return exp->getLiteralString();
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
      assert(false);
      break;
  }
  return ConstructPtr();
}

int CaseStatement::getKidCount() const {
  return 2;
}

void CaseStatement::setNthKid(int n, ConstructPtr cp) {
  switch (n) {
    case 0:
      m_condition = dynamic_pointer_cast<Expression>(cp);
      break;
    case 1:
      m_stmt = dynamic_pointer_cast<Statement>(cp);
      break;
    default:
      assert(false);
      break;
  }
}

void CaseStatement::inferTypes(AnalysisResultPtr ar) {
  assert(false);
}

void CaseStatement::inferAndCheck(AnalysisResultPtr ar, TypePtr type,
                                  bool coerce) {
  if (m_condition) m_condition->inferAndCheck(ar, type, coerce);
  if (m_stmt) m_stmt->inferTypes(ar);
}

///////////////////////////////////////////////////////////////////////////////

void CaseStatement::outputCodeModel(CodeGenerator &cg) {
  auto numProps = 1;
  if (m_condition != nullptr) numProps++;
  if (m_stmt != nullptr) numProps++;
  cg.printObjectHeader("CaseStatement", numProps);
  if (m_condition != nullptr) {
    cg.printPropertyHeader("condition");
    m_condition->outputCodeModel(cg);
  }
  if (m_stmt != nullptr) {
    cg.printPropertyHeader("block");
    cg.printAsBlock(m_stmt);
  }
  cg.printPropertyHeader("sourceLocation");
  cg.printLocation(this->getLocation());
  cg.printObjectFooter();
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void CaseStatement::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  if (m_condition) {
    cg_printf("case ");
    m_condition->outputPHP(cg, ar);
    cg_indentBegin(":\n");
  } else {
    cg_indentBegin("default:\n");
  }
  if (m_stmt) {
    m_stmt->outputPHP(cg, ar);
  }
  cg_indentEnd();
}
