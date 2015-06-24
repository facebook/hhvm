/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/compiler/statement/if_branch_statement.h"
#include "hphp/compiler/expression/constant_expression.h"

using namespace HPHP;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

IfBranchStatement::IfBranchStatement
(STATEMENT_CONSTRUCTOR_PARAMETERS,
 ExpressionPtr condition, StatementPtr stmt)
  : Statement(STATEMENT_CONSTRUCTOR_PARAMETER_VALUES(IfBranchStatement)),
    m_condition(condition), m_stmt(stmt) {
}

StatementPtr IfBranchStatement::clone() {
  IfBranchStatementPtr stmt(new IfBranchStatement(*this));
  stmt->m_condition = Clone(m_condition);
  stmt->m_stmt = Clone(m_stmt);
  return stmt;
}

///////////////////////////////////////////////////////////////////////////////
// parser functions

///////////////////////////////////////////////////////////////////////////////
// static analysis functions

void IfBranchStatement::analyzeProgram(AnalysisResultPtr ar) {
  if (m_condition) m_condition->analyzeProgram(ar);
  if (m_stmt) m_stmt->analyzeProgram(ar);
}

ConstructPtr IfBranchStatement::getNthKid(int n) const {
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

int IfBranchStatement::getKidCount() const {
  return 2;
}

void IfBranchStatement::setNthKid(int n, ConstructPtr cp) {
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

///////////////////////////////////////////////////////////////////////////////

void IfBranchStatement::outputCodeModel(CodeGenerator &cg) {
  if (m_condition == nullptr) {
    cg.printAsBlock(m_stmt);
    return;
  }
  auto numProps = 3;
  cg.printObjectHeader("ConditionalStatement", numProps);
  cg.printPropertyHeader("condition");
  m_condition->outputCodeModel(cg);
  cg.printPropertyHeader("trueBlock");
  cg.printAsBlock(m_stmt);
  cg.printPropertyHeader("sourceLocation");
  cg.printLocation(this);
  cg.printObjectFooter();
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void IfBranchStatement::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  if (m_condition) {
    cg_printf("if (");
    m_condition->outputPHP(cg, ar);
    cg_printf(") ");
  } else {
    cg_printf(" ");
  }
  if (m_stmt) {
    m_stmt->outputPHP(cg, ar);
  } else {
    cg_printf("{}\n");
  }
}
