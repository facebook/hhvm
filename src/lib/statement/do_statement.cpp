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

#include <lib/statement/do_statement.h>
#include <lib/analysis/analysis_result.h>
#include <lib/analysis/block_scope.h>

using namespace HPHP;
using namespace std;
using namespace boost;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

DoStatement::DoStatement
(STATEMENT_CONSTRUCTOR_PARAMETERS,
 StatementPtr stmt, ExpressionPtr condition)
  : Statement(STATEMENT_CONSTRUCTOR_PARAMETER_VALUES),
    m_stmt(stmt), m_condition(condition) {
}

StatementPtr DoStatement::clone() {
  DoStatementPtr stmt(new DoStatement(*this));
  stmt->m_stmt = Clone(m_stmt);
  stmt->m_condition = Clone(m_condition);
  return stmt;
}

///////////////////////////////////////////////////////////////////////////////
// parser functions

///////////////////////////////////////////////////////////////////////////////
// static analysis functions

void DoStatement::analyzeProgram(AnalysisResultPtr ar) {
  if (m_stmt) m_stmt->analyzeProgram(ar);
  m_condition->analyzeProgram(ar);
}

ConstructPtr DoStatement::getNthKid(int n) const {
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

int DoStatement::getKidCount() const {
  return 2;
}

int DoStatement::setNthKid(int n, ConstructPtr cp) {
  switch (n) {
    case 0:
      m_stmt = boost::dynamic_pointer_cast<Statement>(cp);
      return 1;
    case 1:
      m_condition = boost::dynamic_pointer_cast<Expression>(cp);
      return 1;
    default:
      return 0;
  }
  ASSERT(0);
}

StatementPtr DoStatement::preOptimize(AnalysisResultPtr ar) {
  ar->preOptimize(m_stmt);
  ar->preOptimize(m_condition);
  return StatementPtr();
}

StatementPtr DoStatement::postOptimize(AnalysisResultPtr ar) {
  ar->postOptimize(m_stmt);
  ar->postOptimize(m_condition);
  return StatementPtr();
}

void DoStatement::inferTypes(AnalysisResultPtr ar) {
  if (m_stmt) {
    ar->getScope()->incLoopNestedLevel();
    m_stmt->inferTypes(ar);
    m_condition->inferAndCheck(ar, Type::Boolean, false);
    ar->getScope()->decLoopNestedLevel();
  } else {
    m_condition->inferAndCheck(ar, Type::Boolean, false);
  }
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void DoStatement::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  cg.printf("do ");
  if (m_stmt) {
    m_stmt->outputPHP(cg, ar);
  } else {
    cg.printf("{}\n");
  }
  cg.printf("while (");
  m_condition->outputPHP(cg, ar);
  cg.printf(");\n");
}

void DoStatement::outputCPP(CodeGenerator &cg, AnalysisResultPtr ar) {
  cg.indentBegin("{\n");
  int labelId = cg.createNewId(ar);
  cg.pushBreakScope(labelId);

  cg.indentBegin("do {\n");
  cg.printf("LOOP_COUNTER_CHECK(%d);\n", labelId);
  if (m_stmt) {
    m_stmt->outputCPP(cg, ar);
  }
  if (cg.findLabelId("continue", labelId)) {
    cg.printf("continue%d:;\n", labelId);
  }
  cg.indentEnd("} while (");
  m_condition->outputCPP(cg, ar);
  cg.printf(");\n");
  if (cg.findLabelId("break", labelId)) {
    cg.printf("break%d:;\n", labelId);
  }
  cg.indentEnd("}\n");
  cg.popBreakScope();
}
