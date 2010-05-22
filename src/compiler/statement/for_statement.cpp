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

#include <compiler/statement/for_statement.h>
#include <compiler/expression/expression_list.h>
#include <compiler/analysis/analysis_result.h>
#include <compiler/analysis/block_scope.h>

using namespace HPHP;
using namespace std;
using namespace boost;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

static void setListKind(ExpressionPtr e)
{
  if (e && e->is(Expression::KindOfExpressionList)) {
    ExpressionListPtr list =
      static_pointer_cast<ExpressionList>(e);
    list->setListKind(ExpressionList::ListKindComma);
  }
}

ForStatement::ForStatement
(STATEMENT_CONSTRUCTOR_PARAMETERS,
 ExpressionPtr exp1, ExpressionPtr exp2, ExpressionPtr exp3, StatementPtr stmt)
  : Statement(STATEMENT_CONSTRUCTOR_PARAMETER_VALUES),
    m_exp1(exp1), m_exp2(exp2), m_exp3(exp3), m_stmt(stmt) {

  setListKind(m_exp1);
  setListKind(m_exp2);
  setListKind(m_exp3);
}

StatementPtr ForStatement::clone() {
  ForStatementPtr stmt(new ForStatement(*this));
  stmt->m_exp1 = Clone(m_exp1);
  stmt->m_exp2 = Clone(m_exp2);
  stmt->m_exp3 = Clone(m_exp3);
  stmt->m_stmt = Clone(m_stmt);
  return stmt;
}

///////////////////////////////////////////////////////////////////////////////
// parser functions

///////////////////////////////////////////////////////////////////////////////
// static analysis functions

void ForStatement::analyzeProgramImpl(AnalysisResultPtr ar) {
  if (m_exp1) m_exp1->analyzeProgram(ar);
  if (m_exp2) m_exp2->analyzeProgram(ar);
  if (m_exp3) m_exp3->analyzeProgram(ar);
  if (m_stmt) m_stmt->analyzeProgram(ar);
}

ConstructPtr ForStatement::getNthKid(int n) const {
  switch (n) {
    case 0:
      return m_exp1;
    case 1:
      return m_exp2;
    case 2:
      return m_stmt;
    case 3:
      return m_exp3;
    default:
      ASSERT(false);
      break;
  }
  return ConstructPtr();
}

int ForStatement::getKidCount() const {
  return 4;
}

void ForStatement::setNthKid(int n, ConstructPtr cp) {
  switch (n) {
    case 0:
      m_exp1 = boost::dynamic_pointer_cast<Expression>(cp);
      break;
    case 1:
      m_exp2 = boost::dynamic_pointer_cast<Expression>(cp);
      break;
    case 2:
      m_stmt = boost::dynamic_pointer_cast<Statement>(cp);
      break;
    case 3:
      m_exp3 = boost::dynamic_pointer_cast<Expression>(cp);
      break;
    default:
      ASSERT(false);
      break;
  }
}

StatementPtr ForStatement::preOptimize(AnalysisResultPtr ar) {
  ar->preOptimize(m_exp1);
  ar->preOptimize(m_exp2);
  ar->preOptimize(m_exp3);
  if (m_stmt) {
    ar->getScope()->incLoopNestedLevel();
    ar->preOptimize(m_stmt);
    ar->getScope()->decLoopNestedLevel();
  }
  return StatementPtr();
}

StatementPtr ForStatement::postOptimize(AnalysisResultPtr ar) {
  ar->postOptimize(m_exp1);
  ar->postOptimize(m_exp2);
  ar->postOptimize(m_exp3);
  if (m_stmt) {
    ar->getScope()->incLoopNestedLevel();
    ar->postOptimize(m_stmt);
    ar->getScope()->decLoopNestedLevel();
  }
  return StatementPtr();
}

void ForStatement::inferTypes(AnalysisResultPtr ar) {
  if (m_exp1) m_exp1->inferAndCheck(ar, NEW_TYPE(Any), false);
  if (m_exp2) m_exp2->inferAndCheck(ar, Type::Boolean, false);
  if (m_stmt) {
    ar->getScope()->incLoopNestedLevel();
    m_stmt->inferTypes(ar);
    if (m_exp3) m_exp3->inferAndCheck(ar, NEW_TYPE(Any), false);
    ar->getScope()->decLoopNestedLevel();
  } else {
    if (m_exp3) m_exp3->inferAndCheck(ar, NEW_TYPE(Any), false);
  }
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void ForStatement::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  cg.printf("for (");
  if (m_exp1) m_exp1->outputPHP(cg, ar);
  cg.printf("; ");
  if (m_exp2) m_exp2->outputPHP(cg, ar);
  cg.printf("; ");
  if (m_exp3) m_exp3->outputPHP(cg, ar);
  cg.printf(") ");
  if (m_stmt) {
    m_stmt->outputPHP(cg, ar);
  } else {
    cg.printf("{}\n");
  }
}

void ForStatement::outputCPPImpl(CodeGenerator &cg, AnalysisResultPtr ar) {
  cg.indentBegin("{\n");

  bool e1_order = m_exp1 && m_exp1->preOutputCPP(cg, ar, 0);
  bool e2_order = m_exp2 && m_exp2->preOutputCPP(cg, ar, 0);
  bool e3_order = m_exp3 && m_exp3->preOutputCPP(cg, ar, 0);

  int labelId = cg.createNewId(ar);

  cg.pushBreakScope(e3_order ? labelId | CodeGenerator::InsideSwitch : labelId);

  if (e1_order) {
    m_exp1->outputCPPUnneeded(cg, ar);
  }
  cg.printf("for (");
  if (m_exp1 && !e1_order && m_exp1->hasEffect()) {
    m_exp1->outputCPPUnneeded(cg, ar);
  }
  cg.printf("; ");
  if (m_exp2 && !e2_order) m_exp2->outputCPP(cg, ar);
  cg.printf("; ");
  if (m_exp3 && !e3_order && m_exp3->hasEffect()) {
    m_exp3->outputCPPUnneeded(cg, ar);
  }
  cg.indentBegin(") {\n");
  if (e2_order) {
    m_exp2->outputCPPBegin(cg, ar);
    cg.printf("if (!(");
    m_exp2->outputCPP(cg, ar);
    cg.printf(")) break;\n");
    m_exp2->outputCPPEnd(cg, ar);
  }
  cg.printf("LOOP_COUNTER_CHECK(%d);\n", labelId);
  if (m_stmt) {
    m_stmt->outputCPP(cg, ar);
  }
  if (cg.findLabelId("continue", labelId)) {
    cg.printf("continue%d:;\n", labelId);
  }
  if (e3_order) {
    m_exp3->outputCPPUnneeded(cg, ar);
  }
  cg.indentEnd("}\n");
  if (cg.findLabelId("break", labelId)) {
    cg.printf("break%d:;\n", labelId);
  }
  cg.indentEnd("}\n");
  cg.popBreakScope();
}
