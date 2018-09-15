/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#include <set>
#include <folly/ScopeGuard.h>

#include "hphp/compiler/expression/parameter_expression.h"
#include "hphp/compiler/expression/expression_list.h"
#include "hphp/compiler/expression/simple_variable.h"
#include "hphp/compiler/statement/function_statement.h"
#include "hphp/compiler/statement/static_statement.h"
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
    auto param = dynamic_pointer_cast<ParameterExpression>((*vars)[i]);
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
    auto param = dynamic_pointer_cast<ParameterExpression>((*m_vars)[i]);
    auto const& name = param->getName();

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

void ClosureExpression::processLambdas(
  AnalysisResultConstRawPtr ar,
  CompactVector<ClosureExpressionRawPtr>&& lambdas) {
  for (auto const ce : lambdas) {
    ce->processLambda(ar);
  }
  lambdas.clear();
}

void ClosureExpression::processLambda(AnalysisResultConstRawPtr ar) {
  if (m_captureState == CaptureState::Unknown) {
    assert(m_type == ClosureType::Short);
    auto const closureFuncScope = m_func->getFunctionScope();

    auto const paramNames = collectParamNames();
    auto const& mentioned = closureFuncScope->getLocals();

    std::set<std::string> toCapture;

    for (auto& m : mentioned) {
      if (paramNames.count(m)) continue;
      if (m == "this") {
        toCapture.insert("this");
        continue;
      }
      auto scope = closureFuncScope;
      do {
        auto const prev = scope;
        scope = prev->getOuterScope()->getContainingFunction();
        always_assert(scope);
        always_assert(!FileScope::getCurrent() ||
                      scope->getContainingFile() == FileScope::getCurrent());
        if (scope->hasLocal(m)) {
          toCapture.insert(m);
          break;
        }
      } while (scope->isLambdaClosure());
    }

    if (closureFuncScope->containsThis()) {
      toCapture.insert("this");
    }

    setCaptureList(ar, toCapture);
  }
}

void ClosureExpression::analyzeProgram(AnalysisResultConstRawPtr ar) {
  always_assert(getFileScope() == FileScope::getCurrent());

  auto const sameScope = m_func->getFileScope() == FileScope::getCurrent();
  if (sameScope) {
    // Closures in flattened traits could come from another file.
    // Only let the owner analyze them
    ar->analyzeProgram(m_func);

    if (m_captureState == CaptureState::Unknown) {
      assert(m_type == ClosureType::Short);
      FileScope::getCurrent()->addLambda(ClosureExpressionRawPtr{this});
    }
  } else {
    if (m_captureState == CaptureState::Unknown) {
      assert(m_type == ClosureType::Short);
      ar->lock()->addClonedLambda(ClosureExpressionRawPtr{this});
    }
  }

  if (m_vars && ar->getPhase() == AnalysisResult::AnalyzeAll) {
    if (sameScope) analyzeVarsForClosure(ar);
    analyzeVarsForClosureExpression(ar);
  }

  if (!sameScope || m_func->getModifiers()->isStatic()) return;
  auto const funcScope = getFunctionScope();
  auto const container = funcScope->getContainingNonClosureFunction();
  if (container && container->isStatic()) {
    m_func->getModifiers()->add(T_STATIC);
  } else {
    auto const closureFuncScope = m_func->getFunctionScope();
    if (m_type != ClosureType::Short ||
        closureFuncScope->containsThis()) {
      funcScope->setContainsThis();
    }
  }
}

void ClosureExpression::analyzeVarsForClosure(
  AnalysisResultConstRawPtr /*ar*/) {
  // closure function (not containing function)
  auto const func = m_func->getFunctionScope();
  for (auto const& var : *m_vars) {
    auto const param = dynamic_pointer_cast<ParameterExpression>(var);
    func->addLocal(param->getName());
  }
}

void ClosureExpression::analyzeVarsForClosureExpression(
  AnalysisResultConstRawPtr /*ar*/) {
  auto const containing = getFunctionScope();
  for (auto const& var : *m_vars) {
    auto const param = dynamic_pointer_cast<ParameterExpression>(var);
    containing->addLocal(param->getName());
  }
}

void ClosureExpression::setCaptureList(
    AnalysisResultConstRawPtr ar,
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
    if (!usedThis && !m_func->getModifiers()->isStatic()) {
      m_func->getModifiers()->add(T_STATIC);
    }
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
      ParamMode::In,
      0 /* token modifier thing */,
      ExpressionPtr(),
      ExpressionPtr()
    );
    m_vars->insertElement(expr);
  }

  initializeValuesFromVars();
  analyzeVarsForClosure(ar);
  analyzeVarsForClosureExpression(ar);
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
    auto cons = root->getNthKid(i);
    if (auto s = dynamic_pointer_cast<Statement>(cons)) {
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
