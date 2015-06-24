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

#include "hphp/compiler/statement/static_statement.h"
#include "hphp/compiler/analysis/analysis_result.h"
#include "hphp/compiler/expression/expression.h"
#include "hphp/compiler/expression/expression_list.h"
#include "hphp/compiler/analysis/block_scope.h"
#include "hphp/compiler/analysis/variable_table.h"
#include "hphp/compiler/analysis/function_scope.h"
#include "hphp/compiler/expression/simple_variable.h"
#include "hphp/compiler/expression/assignment_expression.h"
#include "hphp/compiler/expression/constant_expression.h"
#include "hphp/compiler/option.h"

using namespace HPHP;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

StaticStatement::StaticStatement
(STATEMENT_CONSTRUCTOR_PARAMETERS, ExpressionListPtr exp)
  : Statement(STATEMENT_CONSTRUCTOR_PARAMETER_VALUES(StaticStatement)),
    m_exp(exp) {
}

StatementPtr StaticStatement::clone() {
  StaticStatementPtr stmt(new StaticStatement(*this));
  stmt->m_exp = Clone(m_exp);
  return stmt;
}

///////////////////////////////////////////////////////////////////////////////
// parser functions

///////////////////////////////////////////////////////////////////////////////
// static analysis functions

void StaticStatement::analyzeProgram(AnalysisResultPtr ar) {
  m_exp->analyzeProgram(ar);
  if (ar->getPhase() == AnalysisResult::AnalyzeAll) {
    BlockScopePtr scope = getScope();
    for (int i = 0; i < m_exp->getCount(); i++) {
      ExpressionPtr exp = (*m_exp)[i];
      ExpressionPtr variable;
      ExpressionPtr value;
      // turn static $a; into static $a = null;
      if (exp->is(Expression::KindOfSimpleVariable)) {
        variable = dynamic_pointer_cast<SimpleVariable>(exp);
        exp = AssignmentExpressionPtr
          (new AssignmentExpression(exp->getScope(), exp->getRange(),
                                    variable,
                                    CONSTANT("null"),
                                    false));
        (*m_exp)[i] = exp;
      }
      always_assert(exp->is(Expression::KindOfAssignmentExpression));
      AssignmentExpressionPtr assignment_exp =
        dynamic_pointer_cast<AssignmentExpression>(exp);
      variable = assignment_exp->getVariable();
      value = assignment_exp->getValue();
      SimpleVariablePtr var = dynamic_pointer_cast<SimpleVariable>(variable);
      // set the Declaration context here instead of all over this file - this phase
      // is the first to run
      var->setContext(Expression::Declaration);
      Symbol *sym = var->getSymbol();
      sym->setStaticInitVal(value);
    }
  }
}

ConstructPtr StaticStatement::getNthKid(int n) const {
  switch (n) {
    case 0:
      return m_exp;
    default:
      assert(false);
      break;
  }
  return ConstructPtr();
}

int StaticStatement::getKidCount() const {
  return 1;
}

void StaticStatement::setNthKid(int n, ConstructPtr cp) {
  switch (n) {
    case 0:
      m_exp = dynamic_pointer_cast<ExpressionList>(cp);
      break;
    default:
      assert(false);
      break;
  }
}

StatementPtr StaticStatement::preOptimize(AnalysisResultConstPtr ar) {
  BlockScopePtr scope = getScope();
  for (int i = 0; i < m_exp->getCount(); i++) {
    ExpressionPtr exp = (*m_exp)[i];
    AssignmentExpressionPtr assignment_exp =
      dynamic_pointer_cast<AssignmentExpression>(exp);
    ExpressionPtr variable = assignment_exp->getVariable();
    ExpressionPtr value = assignment_exp->getValue();
    SimpleVariablePtr var = dynamic_pointer_cast<SimpleVariable>(variable);
    Symbol *sym = var->getSymbol();
    sym->setStaticInitVal(value);
  }
  return StatementPtr();
}

///////////////////////////////////////////////////////////////////////////////

void StaticStatement::outputCodeModel(CodeGenerator &cg) {
  cg.printObjectHeader("StaticStatement", 2);
  cg.printPropertyHeader("expressions");
  cg.printExpressionVector(m_exp);
  cg.printPropertyHeader("sourceLocation");
  cg.printLocation(this);
  cg.printObjectFooter();
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void StaticStatement::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  cg_printf("static ");
  m_exp->outputPHP(cg, ar);
  cg_printf(";\n");
}
