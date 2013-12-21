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
#include "hphp/compiler/analysis/lambda_names.h"

#include <set>

#include "folly/ScopeGuard.h"

#include "hphp/compiler/analysis/variable_table.h"
#include "hphp/compiler/analysis/function_scope.h"
#include "hphp/compiler/statement/method_statement.h"
#include "hphp/compiler/statement/function_statement.h"
#include "hphp/compiler/expression/closure_expression.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

namespace {

struct NameScope {
  NameScope* const prev;
  VariableTablePtr const vars;
};

struct Walker {
  explicit Walker(AnalysisResultPtr ar)
    : m_curScope{nullptr}
    , m_ar{ar}
  {}

  void walk_functions(const FunctionContainer& funcCont) {
    for (auto& kv : funcCont.getFunctions()) {
      walk_function(kv.second);
    }
  }

private:
  void visit_closure(ClosureExpressionPtr ce) {
    auto const cfunc = ce->getClosureFunction();

    with_scope(
      cfunc->getScope()->getVariables(),
      [&] {
        walk_ast(cfunc->getStmts());
      }
    );

    if (ce->type() != ClosureType::Short) return;
    if (ce->captureState() == ClosureExpression::CaptureState::Known) {
      return;
    }

    auto const paramNames = ce->collectParamNames();
    std::set<std::string> mentioned;
    cfunc->getScope()->getVariables()->getNames(mentioned);

    std::set<std::string> toCapture;

    for (auto& m : mentioned) {
      if (paramNames.count(m)) continue;
      if (m == "this") {
        toCapture.insert("this");
        continue;
      }
      for (auto scope = m_curScope; scope; scope = scope->prev) {
        if (scope->vars->getSymbol(m)) {
          toCapture.insert(m);
          break;
        }
      }
    }

    if (cfunc->getFunctionScope()->containsThis()) {
      toCapture.insert("this");
    }

    ce->setCaptureList(m_ar, toCapture);
  }

  void walk_ast(ConstructPtr node) {
    if (!node) return;

    if (dynamic_pointer_cast<MethodStatement>(node)) {
      // Don't descend into nested non-closure functions, or functions
      // in the psuedo-main.
      return;
    }

    if (auto ce = dynamic_pointer_cast<ClosureExpression>(node)) {
      visit_closure(ce);
      return;
    }

    for (int i = 0; i < node->getKidCount(); ++i) {
      walk_ast(node->getNthKid(i));
    }
  }

  void walk_function(const FunctionScopePtr& fscope) {
    if (fscope->isClosure()) return;
    auto ms = dynamic_pointer_cast<MethodStatement>(fscope->getStmt());

    ConstructPtr node(ms->getStmts());
    with_scope(
      fscope->getVariables(),
      [&] {
        walk_ast(node);
      }
    );
  }

  template<class Func>
  void with_scope(const VariableTablePtr& scopeVars, Func func) {
    auto newScope = NameScope { m_curScope, scopeVars };
    m_curScope = &newScope;
    SCOPE_EXIT { m_curScope = m_curScope->prev; };
    func();
  }

private:
  NameScope* m_curScope;
  AnalysisResultPtr m_ar;
};

}

//////////////////////////////////////////////////////////////////////

void resolve_lambda_names(AnalysisResultPtr ar, const FileScopePtr& fscope) {
  Walker walker(ar);

  ClassScopePtrVec classScopes;
  fscope->getClassesFlattened(classScopes);
  for (auto& cls : classScopes) {
    walker.walk_functions(*cls);
  }
  walker.walk_functions(*fscope);
}

//////////////////////////////////////////////////////////////////////

}
