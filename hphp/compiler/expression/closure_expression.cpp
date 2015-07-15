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
#include "hphp/compiler/expression/closure_expression.h"

#include <boost/make_shared.hpp>
#include <set>
#include <folly/ScopeGuard.h>

#include "hphp/compiler/expression/parameter_expression.h"
#include "hphp/compiler/expression/expression_list.h"
#include "hphp/compiler/expression/simple_variable.h"
#include "hphp/compiler/statement/function_statement.h"
#include "hphp/compiler/statement/static_statement.h"
#include "hphp/compiler/analysis/variable_table.h"
#include "hphp/compiler/analysis/function_scope.h"
#include "hphp/compiler/analysis/file_scope.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

ClosureExpression::ClosureExpression(
    EXPRESSION_CONSTRUCTOR_PARAMETERS,
    ClosureType type,
    FunctionStatementPtr func,
    ExpressionListPtr vars)
  : Expression(EXPRESSION_CONSTRUCTOR_PARAMETER_VALUES(ClosureExpression))
  , m_type(type)
  , m_func(func)
  , m_captureState(m_type == ClosureType::Short ? CaptureState::Unknown
                                               : CaptureState::Known)
{
  switch (m_type) {
  case ClosureType::Short:
    break;
  case ClosureType::Long:
    if (vars) initializeFromUseList(vars);
    break;
  }
}

void ClosureExpression::initializeFromUseList(ExpressionListPtr vars) {
  m_vars = ExpressionListPtr(
    new ExpressionList(vars->getScope(), vars->getRange()));

  // Because PHP is insane you can have a use variable with the same
  // name as a param name.
  // In that case, params win (which is different than zend but much easier)
  auto seenBefore = collectParamNames();

  for (int i = vars->getCount() - 1; i >= 0; i--) {
    ParameterExpressionPtr param(
      dynamic_pointer_cast<ParameterExpression>((*vars)[i]));
    assert(param);
    if (param->getName() == "this") {
      // "this" is automatically included.
      // Once we get rid of all the callsites, make this an error
      continue;
    }
    if (seenBefore.find(param->getName().c_str()) == seenBefore.end()) {
      seenBefore.insert(param->getName().c_str());
      m_vars->insertElement(param);
    }
  }

  initializeValuesFromVars();
}

void ClosureExpression::initializeValuesFromVars() {
  if (!m_vars) return;

  m_values = ExpressionListPtr
    (new ExpressionList(m_vars->getScope(), m_vars->getRange()));
  for (int i = 0; i < m_vars->getCount(); i++) {
    ParameterExpressionPtr param =
      dynamic_pointer_cast<ParameterExpression>((*m_vars)[i]);
    const string &name = param->getName();

    SimpleVariablePtr var(new SimpleVariable(param->getScope(),
                                             param->getRange(),
                                             name));
    if (param->isRef()) {
      var->setContext(RefValue);
    }
    m_values->addElement(var);
  }
  assert(m_vars->getCount() == m_values->getCount());
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
      m_values = dynamic_pointer_cast<ExpressionList>(cp);
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

  if (m_vars) analyzeVars(ar);

  FunctionScopeRawPtr container =
    getFunctionScope()->getContainingNonClosureFunction();
  if (container && container->isStatic()) {
    m_func->getModifiers()->add(T_STATIC);
  }
}

void ClosureExpression::analyzeVars(AnalysisResultPtr ar) {
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

void ClosureExpression::setCaptureList(
    AnalysisResultPtr ar,
    const std::set<std::string>& captureNames) {
  assert(m_captureState == CaptureState::Unknown);
  m_captureState = CaptureState::Known;

  bool usedThis = false;
  SCOPE_EXIT {
    /*
     * TODO: closures in a non-class scope should be neither static
     * nor non-static, but right now we don't really have this idea.
     *
     * This would allow not having to check for a $this or late bound
     * class in the closure object or on the ActRec when returning
     * from those closures.
     *
     * (We could also mark closures that don't use late static binding
     * with this flag to avoid checks on closures in member functions
     * when they use neither $this nor static::)
     */
    if (!usedThis) m_func->getModifiers()->add(T_STATIC);
  };

  if (captureNames.empty()) return;

  m_vars = ExpressionListPtr(
    new ExpressionList(getScope(), getRange()));

  for (auto const& name : captureNames) {
    if (name == "this") {
      usedThis = true;
      continue;
    }

    auto expr = std::make_shared<ParameterExpression>(
      BlockScopePtr(getScope()),
      getRange(),
      TypeAnnotationPtr(),
      true /* hhType */,
      name,
      false /* ref */,
      0 /* token modifier thing */,
      ExpressionPtr(),
      ExpressionPtr()
    );
    m_vars->insertElement(expr);
  }

  initializeValuesFromVars();
  analyzeVars(ar);
}

std::set<std::string> ClosureExpression::collectParamNames() const {
  std::set<std::string> ret;

  auto bodyParams = m_func->getParams();
  if (!bodyParams) return ret;

  int nParams = bodyParams->getCount();
  for (int i = 0; i < nParams; i++) {
    auto par = static_pointer_cast<ParameterExpression>((*bodyParams)[i]);
    ret.insert(par->getName());
  }
  return ret;
}

bool ClosureExpression::hasStaticLocals() {
  ConstructPtr cons(m_func);
  return hasStaticLocalsImpl(cons);
}

bool ClosureExpression::hasStaticLocalsImpl(ConstructPtr root) {
  if (!root) {
    return false;
  }
  if (root->getFunctionScope() != m_func->getFunctionScope()) {
    // new scope, new statics
    return false;
  }

  for (int i = 0; i < root->getKidCount(); i++) {
    ConstructPtr cons = root->getNthKid(i);
    if (StatementPtr s = dynamic_pointer_cast<Statement>(cons)) {
      if (s->is(Statement::KindOfStaticStatement)) {
        return true;
      }
    }
    if (hasStaticLocalsImpl(cons)) {
      return true;
    }
  }
  return false;
}

///////////////////////////////////////////////////////////////////////////////

void ClosureExpression::outputCodeModel(CodeGenerator &cg) {
  auto numProps = m_vars != nullptr && m_vars->getCount() > 0 ? 3 : 2;
  cg.printObjectHeader("ClosureExpression", numProps);
  cg.printPropertyHeader("ffunction");
  m_func->outputCodeModel(cg);
  if (m_vars != nullptr && m_vars->getCount() > 0) {
    cg.printPropertyHeader("capturedVariables");
    cg.printExpressionVector(m_vars);
  }
  cg.printPropertyHeader("sourceLocation");
  cg.printLocation(this);
  cg.printObjectFooter();
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

}
