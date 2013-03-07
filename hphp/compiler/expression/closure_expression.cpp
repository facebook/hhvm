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

#include <compiler/expression/closure_expression.h>
#include <compiler/expression/parameter_expression.h>
#include <compiler/expression/expression_list.h>
#include <compiler/expression/simple_variable.h>
#include <compiler/statement/function_statement.h>
#include <compiler/analysis/variable_table.h>
#include <compiler/analysis/function_scope.h>
#include <compiler/analysis/file_scope.h>

using namespace HPHP;

TypePtr ClosureExpression::s_ClosureType =
  Type::CreateObjectType("closure"); // needs lower case

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

ClosureExpression::ClosureExpression
(EXPRESSION_CONSTRUCTOR_PARAMETERS, FunctionStatementPtr func,
 ExpressionListPtr vars)
    : Expression(EXPRESSION_CONSTRUCTOR_PARAMETER_VALUES(ClosureExpression)),
      m_func(func) {

  if (vars) {
    m_vars = ExpressionListPtr
      (new ExpressionList(vars->getScope(), vars->getLocation()));
    // push the vars in reverse order, not retaining duplicates
    std::set<string> seenBefore;
    for (int i = vars->getCount() - 1; i >= 0; i--) {
      ParameterExpressionPtr param(
        dynamic_pointer_cast<ParameterExpression>((*vars)[i]));
      assert(param);
      if (seenBefore.find(param->getName().c_str()) == seenBefore.end()) {
        seenBefore.insert(param->getName().c_str());
        m_vars->insertElement(param);
      }
    }

    if (m_vars) {
      m_values = ExpressionListPtr
        (new ExpressionList(m_vars->getScope(), m_vars->getLocation()));
      for (int i = 0; i < m_vars->getCount(); i++) {
        ParameterExpressionPtr param =
          dynamic_pointer_cast<ParameterExpression>((*m_vars)[i]);
        const string &name = param->getName();

        SimpleVariablePtr var(new SimpleVariable(param->getScope(),
                                                 param->getLocation(),
                                                 name));
        if (param->isRef()) {
          var->setContext(RefValue);
        }
        m_values->addElement(var);
      }
      assert(m_vars->getCount() == m_values->getCount());
    }
  }
}

ExpressionPtr ClosureExpression::clone() {
  ClosureExpressionPtr exp(new ClosureExpression(*this));
  Expression::deepCopy(exp);

  // don't clone the function statement or the vars, since
  // they are shared with the function scope
  exp->m_func = m_func;
  exp->m_vars = m_vars;

  exp->m_values = Clone(m_values);
  return exp;
}

ConstructPtr ClosureExpression::getNthKid(int n) const {
  switch (n) {
    case 0:
      return m_values;
    default:
      assert(false);
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
      assert(false);
      break;
  }
}

///////////////////////////////////////////////////////////////////////////////
// static analysis functions

void ClosureExpression::analyzeProgram(AnalysisResultPtr ar) {
  m_func->analyzeProgram(ar);
  if (m_vars) {
    m_values->analyzeProgram(ar);

    if (ar->getPhase() == AnalysisResult::AnalyzeAll) {
      getFunctionScope()->addUse(m_func->getFunctionScope(),
                                 BlockScope::UseKindClosure);
      m_func->getFunctionScope()->setClosureVars(m_vars);

      // closure function's variable table (not containing function's)
      VariableTablePtr variables = m_func->getFunctionScope()->getVariables();
      VariableTablePtr containing = getFunctionScope()->getVariables();
      for (int i = 0; i < m_vars->getCount(); i++) {
        ParameterExpressionPtr param =
          dynamic_pointer_cast<ParameterExpression>((*m_vars)[i]);
        const string &name = param->getName();
        {
          Symbol *containingSym = containing->addDeclaredSymbol(name, param);
          containingSym->setPassClosureVar();

          Symbol *sym = variables->addDeclaredSymbol(name, param);
          sym->setClosureVar();
          sym->setDeclaration(ConstructPtr());
          if (param->isRef()) {
            sym->setRefClosureVar();
            sym->setUsed();
          } else {
            sym->clearRefClosureVar();
            sym->clearUsed();
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
        const string &name = param->getName();

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
  if (m_vars) {
    assert(m_values && m_values->getCount() == m_vars->getCount());

    // containing function's variable table (not closure function's)
    VariableTablePtr variables = getScope()->getVariables();

    // closure function's variable table
    VariableTablePtr cvariables = m_func->getFunctionScope()->getVariables();

    // force all reference use vars into variant for this function scope
    for (int i = 0; i < m_vars->getCount(); i++) {
      ParameterExpressionPtr param =
        dynamic_pointer_cast<ParameterExpression>((*m_vars)[i]);
      const string &name = param->getName();
      if (param->isRef()) {
        variables->forceVariant(ar, name, VariableTable::AnyVars);
      }
    }

    // infer the types of the values
    m_values->inferAndCheck(ar, Type::Some, false);

    // coerce the types inferred from m_values into m_vars
    for (int i = 0; i < m_vars->getCount(); i++) {
      ExpressionPtr value = (*m_values)[i];
      ParameterExpressionPtr var =
        dynamic_pointer_cast<ParameterExpression>((*m_vars)[i]);
      assert(!var->getExpectedType());
      assert(!var->getImplementedType());
      if (var->isRef()) {
        var->setActualType(Type::Variant);
      } else {
        TypePtr origVarType(var->getActualType() ?
                            var->getActualType() : Type::Some);
        var->setActualType(Type::Coerce(ar, origVarType, value->getType()));
      }
    }

    {
      // this lock isn't technically needed for thread-safety, since
      // the dependencies are all set up. however, the lock assertions
      // will fail if we don't acquire it.
      GET_LOCK(m_func->getFunctionScope());

      // bootstrap the closure function's variable table with
      // the types from m_vars
      for (int i = 0; i < m_vars->getCount(); i++) {
        ParameterExpressionPtr param =
          dynamic_pointer_cast<ParameterExpression>((*m_vars)[i]);
        const string &name = param->getName();
        cvariables->addParamLike(name, param->getType(), ar,
                                 shared_from_this(),
                                 getScope()->isFirstPass());
      }
    }
  }
  return s_ClosureType;
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
