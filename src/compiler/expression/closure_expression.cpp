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

#include <compiler/expression/closure_expression.h>
#include <compiler/expression/parameter_expression.h>
#include <compiler/expression/expression_list.h>
#include <compiler/expression/simple_variable.h>
#include <compiler/statement/function_statement.h>
#include <compiler/analysis/variable_table.h>
#include <compiler/analysis/function_scope.h>

using namespace HPHP;
using namespace std;
using namespace boost;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

ClosureExpression::ClosureExpression
(EXPRESSION_CONSTRUCTOR_PARAMETERS, FunctionStatementPtr func,
 ExpressionListPtr vars)
    : Expression(EXPRESSION_CONSTRUCTOR_PARAMETER_VALUES),
      m_func(func), m_vars(vars) {

  if (m_vars) {
    m_values = ExpressionListPtr
      (new ExpressionList(m_vars->getScope(), m_vars->getLocation(),
                          KindOfExpressionList));
    for (int i = 0; i < m_vars->getCount(); i++) {
      ParameterExpressionPtr param =
        dynamic_pointer_cast<ParameterExpression>((*m_vars)[i]);
      string name = param->getName();

      SimpleVariablePtr var(new SimpleVariable(param->getScope(),
                                               param->getLocation(),
                                               KindOfSimpleVariable,
                                               name));
      if (param->isRef()) {
        var->setContext(RefValue);
      }
      m_values->addElement(var);
    }
  }
}

ExpressionPtr ClosureExpression::clone() {
  ClosureExpressionPtr exp(new ClosureExpression(*this));
  Expression::deepCopy(exp);
  exp->m_func = Clone(m_func);
  exp->m_vars = Clone(m_vars);
  return exp;
}

ConstructPtr ClosureExpression::getNthKid(int n) const {
  switch (n) {
    case 0:
      return m_values;
    default:
      ASSERT(false);
      break;
  }
  return ConstructPtr();
}

int ClosureExpression::getKidCount() const {
  return 1;
}

void ClosureExpression::setNthKid(int n, ConstructPtr cp) {
  switch (n) {
    case 0:
      m_values = boost::dynamic_pointer_cast<ExpressionList>(cp);
      break;
    default:
      ASSERT(false);
      break;
  }
}

///////////////////////////////////////////////////////////////////////////////
// parser functions

///////////////////////////////////////////////////////////////////////////////
// static analysis functions

void ClosureExpression::analyzeProgram(AnalysisResultPtr ar) {
  m_func->analyzeProgram(ar);

  if (m_vars) {
    m_values->analyzeProgram(ar);

    if (ar->getPhase() == AnalysisResult::AnalyzeAll) {
      m_func->getFunctionScope()->setClosureVars(m_vars);

      // closure function's variable table (not containing function's)
      VariableTablePtr variables = m_func->getFunctionScope()->getVariables();
      for (int i = 0; i < m_vars->getCount(); i++) {
        ParameterExpressionPtr param =
          dynamic_pointer_cast<ParameterExpression>((*m_vars)[i]);
        string name = param->getName();
        {
          Symbol *sym = variables->addSymbol(name);
          sym->setClosureVar();
          if (param->isRef()) {
            sym->setRefClosureVar();
          }
        }
      }
      return;
    }
    if (ar->getPhase() == AnalysisResult::AnalyzeFinal) {
      // closure function's variable table (not containing function's)
      VariableTablePtr variables = m_func->getFunctionScope()->getVariables();
      for (int i = 0; i < m_vars->getCount(); i++) {
        ParameterExpressionPtr param =
          dynamic_pointer_cast<ParameterExpression>((*m_vars)[i]);
        string name = param->getName();

        // so we can assign values to them, instead of seeing CVarRef
        Symbol *sym = variables->getSymbol(name);
        if (sym && sym->isParameter()) {
          sym->setLvalParam();
        }
      }
    }
  }
}

TypePtr ClosureExpression::inferTypes(AnalysisResultPtr ar, TypePtr type,
                                      bool coerce) {
  m_func->inferTypes(ar);
  if (m_values) m_values->inferAndCheck(ar, Type::Some, false);
  if (m_vars) {
    // containing function's variable table (not closure function's)
    VariableTablePtr variables = getScope()->getVariables();
    for (int i = 0; i < m_vars->getCount(); i++) {
      ParameterExpressionPtr param =
        dynamic_pointer_cast<ParameterExpression>((*m_vars)[i]);
      string name = param->getName();
      if (param->isRef()) {
        variables->forceVariant(ar, name, VariableTable::AnyVars);
      }
    }
  }
  return Type::CreateObjectType("Closure");
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void ClosureExpression::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  m_func->outputPHPHeader(cg, ar);
  if (m_vars && m_vars->getCount()) {
    cg_printf(" use (");
    m_vars->outputPHP(cg, ar);
    cg_printf(")");
  }
  m_func->outputPHPBody(cg, ar);
}

void ClosureExpression::outputCPPImpl(CodeGenerator &cg,
                                      AnalysisResultPtr ar) {
  cg_printf("%sClosure((NEWOBJ(%sClosure)())->create(\"%s\", ",
            Option::SmartPtrPrefix, Option::ClassPrefix,
            m_func->getOriginalName().c_str());

  if (m_vars && m_vars->getCount()) {
    cg_printf("Array(ArrayInit(%d, false, true)", m_vars->getCount());
    for (int i = 0; i < m_vars->getCount(); i++) {
      ParameterExpressionPtr param =
        dynamic_pointer_cast<ParameterExpression>((*m_vars)[i]);
      ExpressionPtr value = (*m_values)[i];

      cg_printf(".set%s(\"%s\", ",
                param->isRef() ? "Ref" : "", param->getName().c_str());
      value->outputCPP(cg, ar);
      cg_printf(")");
    }
    cg_printf(".create())");
  } else {
    cg_printf("Array()");
  }

  cg_printf("))");
}
