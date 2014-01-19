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

#include "hphp/compiler/statement/global_statement.h"
#include "hphp/compiler/statement/block_statement.h"
#include "hphp/compiler/expression/expression_list.h"
#include "hphp/compiler/analysis/block_scope.h"
#include "hphp/compiler/analysis/variable_table.h"
#include "hphp/compiler/analysis/analysis_result.h"
#include "hphp/compiler/analysis/code_error.h"
#include "hphp/compiler/expression/simple_variable.h"
#include "hphp/compiler/expression/dynamic_variable.h"
#include "hphp/compiler/analysis/function_scope.h"

using namespace HPHP;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

GlobalStatement::GlobalStatement
(STATEMENT_CONSTRUCTOR_PARAMETERS, ExpressionListPtr exp)
  : Statement(STATEMENT_CONSTRUCTOR_PARAMETER_VALUES(GlobalStatement)),
    m_exp(exp) {

  std::set<string> seen;
  for (int i = 0; i < m_exp->getCount(); i++) {
    ExpressionPtr exp = (*m_exp)[i];
    exp->setContext(Expression::Declaration);
    if (exp->is(Expression::KindOfSimpleVariable)) {
      const string &name = static_pointer_cast<SimpleVariable>(exp)->getName();
      if (!seen.insert(name).second) {
        m_exp->removeElement(i--);
      }
    }
  }
}

StatementPtr GlobalStatement::clone() {
  GlobalStatementPtr stmt(new GlobalStatement(*this));
  stmt->m_exp = Clone(m_exp);
  return stmt;
}

///////////////////////////////////////////////////////////////////////////////
// parser functions

///////////////////////////////////////////////////////////////////////////////
// static analysis functions

void GlobalStatement::analyzeProgram(AnalysisResultPtr ar) {
  m_exp->analyzeProgram(ar);
}

ConstructPtr GlobalStatement::getNthKid(int n) const {
  switch (n) {
    case 0:
      return m_exp;
    default:
      assert(false);
      break;
  }
  return ConstructPtr();
}

int GlobalStatement::getKidCount() const {
  return 1;
}

void GlobalStatement::setNthKid(int n, ConstructPtr cp) {
  switch (n) {
    case 0:
      m_exp = dynamic_pointer_cast<ExpressionList>(cp);
      break;
    default:
      assert(false);
      break;
  }
}

StatementPtr GlobalStatement::preOptimize(AnalysisResultConstPtr ar) {
  if (!m_exp->getCount()) {
    return NULL_STATEMENT();
  }
  return StatementPtr();
}

StatementPtr GlobalStatement::postOptimize(AnalysisResultConstPtr ar) {
  if (!m_exp->getCount()) {
    return NULL_STATEMENT();
  }
  return StatementPtr();
}

void GlobalStatement::inferTypes(AnalysisResultPtr ar) {
  IMPLEMENT_INFER_AND_CHECK_ASSERT(getScope());

  BlockScopePtr scope = getScope();
  for (int i = 0; i < m_exp->getCount(); i++) {
    ExpressionPtr exp = (*m_exp)[i];
    VariableTablePtr variables = scope->getVariables();
    variables->setAttribute(VariableTable::NeedGlobalPointer);
    if (exp->is(Expression::KindOfSimpleVariable)) {
      SimpleVariablePtr var = dynamic_pointer_cast<SimpleVariable>(exp);
      const std::string &name = var->getName();
      /* If we have already seen this variable in the current scope and
         it is not a global variable, record this variable as "redeclared"
         which will force Variant type.
       */
      variables->setAttribute(VariableTable::InsideGlobalStatement);
      variables->checkRedeclared(name, KindOfGlobalStatement);
      variables->addLocalGlobal(name);
      var->setContext(Expression::Declaration);
      var->inferAndCheck(ar, Type::Any, true);
      variables->forceVariant(ar, name, VariableTable::AnyVars);
      variables->clearAttribute(VariableTable::InsideGlobalStatement);
    } else {
      variables->forceVariants(ar, VariableTable::AnyVars);
      variables->setAttribute(VariableTable::ContainsLDynamicVariable);
      assert(exp->is(Expression::KindOfDynamicVariable));
      exp->inferAndCheck(ar, Type::Any, true);
    }
  }
}

///////////////////////////////////////////////////////////////////////////////

void GlobalStatement::outputCodeModel(CodeGenerator &cg) {
  cg.printObjectHeader("GlobalStatement", 2);
  cg.printPropertyHeader("expressions");
  m_exp->outputCodeModel(cg);
  cg.printPropertyHeader("sourceLocation");
  cg.printLocation(this->getLocation());
  cg.printObjectFooter();
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void GlobalStatement::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  cg_printf("global ");
  m_exp->outputPHP(cg, ar);
  cg_printf(";\n");
}
