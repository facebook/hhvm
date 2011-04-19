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

#include <compiler/statement/global_statement.h>
#include <compiler/statement/block_statement.h>
#include <compiler/expression/expression_list.h>
#include <compiler/analysis/block_scope.h>
#include <compiler/analysis/variable_table.h>
#include <compiler/analysis/analysis_result.h>
#include <compiler/analysis/code_error.h>
#include <compiler/expression/simple_variable.h>
#include <compiler/expression/dynamic_variable.h>
#include <compiler/analysis/function_scope.h>

using namespace HPHP;
using namespace std;
using namespace boost;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

GlobalStatement::GlobalStatement
(STATEMENT_CONSTRUCTOR_PARAMETERS, ExpressionListPtr exp)
  : Statement(STATEMENT_CONSTRUCTOR_PARAMETER_VALUES),
    m_exp(exp) {

  set<string> seen;
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

void GlobalStatement::analyzeProgramImpl(AnalysisResultPtr ar) {
  m_exp->analyzeProgram(ar);
}

ConstructPtr GlobalStatement::getNthKid(int n) const {
  switch (n) {
    case 0:
      return m_exp;
    default:
      ASSERT(false);
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
      m_exp = boost::dynamic_pointer_cast<ExpressionList>(cp);
      break;
    default:
      ASSERT(false);
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
      if (exp->is(Expression::KindOfDynamicVariable)) {
        exp->inferAndCheck(ar, Type::Any, true);
      } else {
        assert(false);
      }
    }
  }
  FunctionScopePtr func = getFunctionScope();
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void GlobalStatement::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  cg_printf("global ");
  m_exp->outputPHP(cg, ar);
  cg_printf(";\n");
}

void GlobalStatement::outputCPPImpl(CodeGenerator &cg, AnalysisResultPtr ar) {
  BlockScopePtr scope = getScope();
  if (m_exp->getCount() > 1) cg_indentBegin("{\n");
  for (int i = 0; i < m_exp->getCount(); i++) {
    ExpressionPtr exp = (*m_exp)[i];
    if (exp->is(Expression::KindOfSimpleVariable)) {
      SimpleVariablePtr var = dynamic_pointer_cast<SimpleVariable>(exp);
      const string &name = var->getName();
      VariableTablePtr variables = scope->getVariables();
      if (variables->needLocalCopy(name)) {
        cg_printf("%s%s.assignRef(g->%s);\n",
                  Option::VariablePrefix, name.c_str(),
                  variables->getGlobalVariableName(cg, ar, name).c_str());
      }
    } else if (exp->is(Expression::KindOfDynamicVariable)) {
      DynamicVariablePtr var = dynamic_pointer_cast<DynamicVariable>(exp);
      ExpressionPtr exp = var->getSubExpression();
      exp->outputCPPBegin(cg, ar);
      int id = cg.createNewLocalId(shared_from_this());
      cg_printf("CStrRef dgv_%d((", id);
      exp->outputCPP(cg, ar);
      cg_printf("));\n");
      cg_printf("variables->get(dgv_%d).assignRef(g->get(dgv_%d));\n", id, id);
      exp->outputCPPEnd(cg, ar);
    } else {
      assert(false);
    }
  }
  if (m_exp->getCount() > 1) cg_indentEnd("}\n");
}
