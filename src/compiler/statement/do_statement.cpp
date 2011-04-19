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

#include <compiler/statement/do_statement.h>
#include <compiler/analysis/analysis_result.h>
#include <compiler/analysis/block_scope.h>

using namespace HPHP;
using namespace std;
using namespace boost;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

DoStatement::DoStatement
(STATEMENT_CONSTRUCTOR_PARAMETERS,
 StatementPtr stmt, ExpressionPtr condition)
  : LoopStatement(STATEMENT_CONSTRUCTOR_PARAMETER_VALUES),
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

void DoStatement::analyzeProgramImpl(AnalysisResultPtr ar) {
  if (m_stmt) m_stmt->analyzeProgram(ar);
  m_condition->analyzeProgram(ar);
}

ConstructPtr DoStatement::getNthKid(int n) const {
  switch (n) {
    case BodyStmt:
      return m_stmt;
    case CondExpr:
      return m_condition;
    default:
      ASSERT(false);
      break;
  }
  return ConstructPtr();
}

int DoStatement::getKidCount() const {
  return 2;
}

void DoStatement::setNthKid(int n, ConstructPtr cp) {
  switch (n) {
    case BodyStmt:
      m_stmt = boost::dynamic_pointer_cast<Statement>(cp);
      break;
    case CondExpr:
      m_condition = boost::dynamic_pointer_cast<Expression>(cp);
      break;
    default:
      ASSERT(false);
      break;
  }
}

void DoStatement::inferTypes(AnalysisResultPtr ar) {
  if (m_stmt) {
    getScope()->incLoopNestedLevel();
    m_stmt->inferTypes(ar);
    m_condition->inferAndCheck(ar, Type::Boolean, false);
    getScope()->decLoopNestedLevel();
  } else {
    m_condition->inferAndCheck(ar, Type::Boolean, false);
  }
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void DoStatement::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  cg_printf("do ");
  if (m_stmt) {
    m_stmt->outputPHP(cg, ar);
  } else {
    cg_printf("{}\n");
  }
  cg_printf("while (");
  m_condition->outputPHP(cg, ar);
  cg_printf(");\n");
}

void DoStatement::outputCPPImpl(CodeGenerator &cg, AnalysisResultPtr ar) {
  cg_indentBegin("{\n");
  bool e_order = m_condition->preOutputCPP(cg, ar, 0);

  int labelId = cg.createNewLocalId(shared_from_this());
  cg.pushBreakScope(e_order ? labelId | CodeGenerator::InsideSwitch : labelId);

  cppDeclareBufs(cg, ar);
  cg_indentBegin("do {\n");
  cg_printf("LOOP_COUNTER_CHECK(%d);\n", labelId);
  if (m_stmt) {
    m_stmt->outputCPP(cg, ar);
  }
  if (cg.findLabelId("continue", labelId)) {
    cg_printf("continue%d:;\n", labelId);
  }
  if (e_order) {
    m_condition->outputCPPBegin(cg, ar);
    cg_printf("if (!(");
    m_condition->outputCPP(cg, ar);
    cg_printf(")) break;\n");
    m_condition->outputCPPEnd(cg, ar);
  }
  cg_indentEnd("} while (");
  if (e_order) {
    cg_printf("true");
  } else {
    m_condition->outputCPP(cg, ar);
  }
  cg_printf(");\n");
  if (cg.findLabelId("break", labelId)) {
    cg_printf("break%d:;\n", labelId);
  }
  cppEndBufs(cg, ar);
  cg_indentEnd("}\n");
  cg.popBreakScope();
}
