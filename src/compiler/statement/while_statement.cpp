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

#include <compiler/statement/while_statement.h>
#include <compiler/analysis/analysis_result.h>
#include <compiler/analysis/block_scope.h>

using namespace HPHP;
using namespace std;
using namespace boost;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

WhileStatement::WhileStatement
(STATEMENT_CONSTRUCTOR_PARAMETERS,
 ExpressionPtr condition, StatementPtr stmt)
  : LoopStatement(STATEMENT_CONSTRUCTOR_PARAMETER_VALUES),
    m_condition(condition), m_stmt(stmt) {
}

StatementPtr WhileStatement::clone() {
  WhileStatementPtr stmt(new WhileStatement(*this));
  stmt->m_condition = Clone(m_condition);
  stmt->m_stmt = Clone(m_stmt);
  return stmt;
}

///////////////////////////////////////////////////////////////////////////////
// parser functions

///////////////////////////////////////////////////////////////////////////////
// static analysis functions

void WhileStatement::analyzeProgramImpl(AnalysisResultPtr ar) {
  m_condition->analyzeProgram(ar);
  if (m_stmt) m_stmt->analyzeProgram(ar);
}

ConstructPtr WhileStatement::getNthKid(int n) const {
  switch (n) {
    case CondExpr:
      return m_condition;
    case BodyStmt:
      return m_stmt;
    default:
      ASSERT(false);
      break;
  }
  return ConstructPtr();
}

int WhileStatement::getKidCount() const {
  return 2;
}

void WhileStatement::setNthKid(int n, ConstructPtr cp) {
  switch (n) {
    case CondExpr:
      m_condition = boost::dynamic_pointer_cast<Expression>(cp);
      break;
    case BodyStmt:
      m_stmt = boost::dynamic_pointer_cast<Statement>(cp);
      break;
    default:
      ASSERT(false);
      break;
  }
}

void WhileStatement::inferTypes(AnalysisResultPtr ar) {
  m_condition->inferAndCheck(ar, Type::Boolean, false);
  if (m_stmt) {
    getScope()->incLoopNestedLevel();
    m_stmt->inferTypes(ar);
    getScope()->decLoopNestedLevel();
  }
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void WhileStatement::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  cg_printf("while (");
  m_condition->outputPHP(cg, ar);
  cg_printf(") ");
  if (m_stmt) {
    m_stmt->outputPHP(cg, ar);
  } else {
    cg_printf("{}\n");
  }
}

void WhileStatement::outputCPPImpl(CodeGenerator &cg, AnalysisResultPtr ar) {
  int labelId = cg.createNewLocalId(shared_from_this());
  cg.pushBreakScope(labelId);
  cg_indentBegin("{\n");

  bool e_order = m_condition->preOutputCPP(cg, ar, 0);

  cppDeclareBufs(cg, ar);
  cg_printf("while (");
  if (e_order) {
    cg_printf("true");
  } else {
    m_condition->outputCPP(cg, ar);
  }
  cg_indentBegin(") {\n");
  if (e_order) {
    m_condition->outputCPPBegin(cg, ar);
    cg_printf("if (!(");
    m_condition->outputCPP(cg, ar);
    cg_printf(")) break;\n");
    m_condition->outputCPPEnd(cg, ar);
  }
  cg_printf("LOOP_COUNTER_CHECK(%d);\n", labelId);
  if (m_stmt) {
    m_stmt->outputCPP(cg, ar);
  }
  if (cg.findLabelId("continue", labelId)) {
    cg_printf("continue%d:;\n", labelId, labelId);
  }
  cg_indentEnd("}\n");
  if (cg.findLabelId("break", labelId)) {
    cg_printf("break%d:;\n", labelId);
  }
  cppEndBufs(cg, ar);
  cg_indentEnd("}\n");
  cg.popBreakScope();
}
