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

#include <compiler/statement/break_statement.h>
#include <runtime/base/complex_types.h>

using namespace HPHP;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

BreakStatement::BreakStatement
(STATEMENT_CONSTRUCTOR_BASE_PARAMETERS, ExpressionPtr exp)
  : Statement(STATEMENT_CONSTRUCTOR_BASE_PARAMETER_VALUES),
    m_exp(exp) {
  m_name = "break";
}

BreakStatement::BreakStatement
(STATEMENT_CONSTRUCTOR_PARAMETERS, ExpressionPtr exp)
  : Statement(STATEMENT_CONSTRUCTOR_PARAMETER_VALUES(BreakStatement)),
    m_exp(exp) {
  m_name = "break";
}

StatementPtr BreakStatement::clone() {
  BreakStatementPtr stmt(new BreakStatement(*this));
  stmt->m_exp = Clone(m_exp);
  return stmt;
}

///////////////////////////////////////////////////////////////////////////////
// parser functions

///////////////////////////////////////////////////////////////////////////////
// static analysis functions

void BreakStatement::analyzeProgram(AnalysisResultPtr ar) {
  if (m_exp) m_exp->analyzeProgram(ar);
}

ConstructPtr BreakStatement::getNthKid(int n) const {
  switch (n) {
    case 0:
      return m_exp;
    default:
      assert(false);
      break;
  }
  return ConstructPtr();
}

int BreakStatement::getKidCount() const {
  return 1;
}

void BreakStatement::setNthKid(int n, ConstructPtr cp) {
  switch (n) {
    case 0:
      m_exp = boost::dynamic_pointer_cast<Expression>(cp);
      break;
    default:
      assert(false);
      break;
  }
}

ExpressionPtr BreakStatement::getExp() {
  return m_exp;
}

StatementPtr BreakStatement::preOptimize(AnalysisResultConstPtr ar) {
  Variant v;
  // break/continue 1 => break/continue;
  if (m_exp && m_exp->getScalarValue(v) &&
      v.isInteger() && v.toInt64() == 1) {
    m_exp = ExpressionPtr();
  }
  return StatementPtr();
}

void BreakStatement::inferTypes(AnalysisResultPtr ar) {
  if (m_exp) m_exp->inferAndCheck(ar, Type::Int64, false);
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void BreakStatement::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  if (m_exp) {
    cg_printf("%s ", m_name);
    m_exp->outputPHP(cg, ar);
    cg_printf(";\n");
  } else {
    cg_printf("%s;\n", m_name);
  }
}

int64_t BreakStatement::getDepth() {
  if (!m_exp) return 1;
  Variant v;
  if (m_exp->getScalarValue(v) &&
      v.isInteger()) {
    int64_t depth = v.toInt64();
    return depth >= 1 ? depth : 1;
  }
  return 0;
}
