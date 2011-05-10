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

#include <compiler/statement/case_statement.h>
#include <compiler/expression/scalar_expression.h>
#include <util/parser/hphp.tab.hpp>
#include <compiler/option.h>

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

string CaseStatement::getLiteralString() const {
  ASSERT(m_condition->is(Expression::KindOfScalarExpression));
  ScalarExpressionPtr exp =
    dynamic_pointer_cast<ScalarExpression>(m_condition);
  return exp->getLiteralString();
}

void CaseStatement::analyzeProgramImpl(AnalysisResultPtr ar) {
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
      ASSERT(false);
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
      m_condition = boost::dynamic_pointer_cast<Expression>(cp);
      break;
    case 1:
      m_stmt = boost::dynamic_pointer_cast<Statement>(cp);
      break;
    default:
      ASSERT(false);
      break;
  }
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
    cg_printf("case ");
    m_condition->outputPHP(cg, ar);
    cg_indentBegin(":\n");
  } else {
    cg_indentBegin("default:\n");
  }
  if (m_stmt) {
    m_stmt->outputPHP(cg, ar);
  }
  cg_indentEnd("");
}

void CaseStatement::outputCPPImpl(CodeGenerator &cg, AnalysisResultPtr ar) {
  if (m_condition) {
    cg_printf("case ");
    m_condition->outputCPPImpl(cg, ar);
    cg_indentBegin(":\n");
  } else {
    cg_indentBegin("default:\n");
  }
  cg_indentBegin("{\n");
  if (m_stmt) {
    m_stmt->outputCPP(cg, ar);
  }
  cg_indentEnd("}\n");
  cg_indentEnd("");
}

void CaseStatement::outputCPPByNumber(CodeGenerator &cg, AnalysisResultPtr ar,
                                      int varId, int caseNum) {
  if (caseNum >= 0) {
    cg_indentBegin("case_%d_%d:\n", varId, caseNum);
  }
  cg_indentBegin("{\n");
  if (m_stmt) {
    m_stmt->outputCPP(cg, ar);
  }
  cg_indentEnd("}\n");
  if (caseNum >= 0) {
    cg_indentEnd("");
  }
}

void CaseStatement::outputCPPAsIf(CodeGenerator &cg, AnalysisResultPtr ar,
                                  int varId, const char *var, int caseNum) {
  if (m_condition) {
    m_condition->outputCPPBegin(cg, ar);
    cg_printf("if (equal(%s, (", var);
    m_condition->outputCPP(cg, ar);
    cg_printf("))) goto case_%d_%d;\n", varId, caseNum);
    m_condition->outputCPPEnd(cg, ar);
  } else {
    cg_printf("goto case_%d_%d;\n", varId, caseNum);
  }
}
