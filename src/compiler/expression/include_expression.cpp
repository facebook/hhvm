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

#include <compiler/expression/include_expression.h>
#include <util/parser/hphp.tab.hpp>
#include <compiler/analysis/dependency_graph.h>
#include <compiler/analysis/code_error.h>
#include <compiler/analysis/file_scope.h>
#include <compiler/analysis/function_scope.h>
#include <compiler/statement/statement_list.h>
#include <compiler/option.h>
#include <compiler/expression/expression_list.h>
#include <compiler/analysis/class_scope.h>
#include <compiler/parser/parser.h>
#include <compiler/analysis/variable_table.h>
#include <compiler/expression/scalar_expression.h>
#include <util/util.h>

using namespace HPHP;
using namespace std;
using namespace boost;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

IncludeExpression::IncludeExpression
(EXPRESSION_CONSTRUCTOR_PARAMETERS, ExpressionPtr exp, int op)
  : UnaryOpExpression(EXPRESSION_CONSTRUCTOR_PARAMETER_VALUES, exp, op, true),
    m_documentRoot(false), m_privateScope(false), m_depsSet(false) {
}

ExpressionPtr IncludeExpression::clone() {
  IncludeExpressionPtr exp(new IncludeExpression(*this));
  Expression::deepCopy(exp);
  exp->m_exp = Clone(m_exp);
  exp->m_depsSet = false;
  return exp;
}

///////////////////////////////////////////////////////////////////////////////
// parser functions

void IncludeExpression::onParse(AnalysisResultPtr ar, BlockScopePtr scope) {
  // See if we can get a string literal
  if (ExpressionPtr exp = ar->preOptimize(m_exp)) {
    m_exp = exp;
  }
  m_include = ar->getDependencyGraph()->add
    (DependencyGraph::KindOfPHPInclude, shared_from_this(), m_exp,
     m_documentRoot);
}

///////////////////////////////////////////////////////////////////////////////
// static analysis functions

std::string IncludeExpression::getCurrentInclude(AnalysisResultPtr ar) {
  return m_include;
}

void IncludeExpression::analyzeInclude(AnalysisResultPtr ar,
                                       const std::string &include) {
  ConstructPtr self = shared_from_this();
  FileScopePtr file = ar->findFileScope(include, ar->getPhase() <=
                                        AnalysisResult::AnalyzeInclude);
  if (!file && !include.empty()) {
    Compiler::Error(Compiler::PHPIncludeFileNotFound, self);
    return;
  }
  FunctionScopePtr func = getFunctionScope();
  getFileScope()->addIncludeDependency(ar, m_include,
                                       func && func->isInlined());
  if (func && file->getPseudoMain()) {
    file->getPseudoMain()->addUse(func, BlockScope::UseKindInclude);
  }
}

void IncludeExpression::analyzeProgram(AnalysisResultPtr ar) {
  string include = getCurrentInclude(ar);
  if (!include.empty()) {
    if (ar->getPhase() == AnalysisResult::AnalyzeInclude) {
      analyzeInclude(ar, include);
    }
  }
  if (!getScope()->inPseudoMain() &&
      !m_privateScope) {
    VariableTablePtr var = getScope()->getVariables();
    var->setAttribute(VariableTable::ContainsLDynamicVariable);
    var->forceVariants(ar, VariableTable::AnyVars);
  }

  UnaryOpExpression::analyzeProgram(ar);
}

ExpressionPtr IncludeExpression::preOptimize(AnalysisResultPtr ar) {
  if (ar->getPhase() >= AnalysisResult::FirstPreOptimize) {
    if (m_include.empty()) {
      m_include = ar->getDependencyGraph()->add
        (DependencyGraph::KindOfPHPInclude, shared_from_this(), m_exp,
         m_documentRoot);
      m_depsSet = false;
    }
    if (!m_depsSet && !m_include.empty()) {
      analyzeInclude(ar, m_include);
      m_depsSet = true;
    }
  }
  return ExpressionPtr();
}

ExpressionPtr IncludeExpression::postOptimize(AnalysisResultPtr ar) {
  if (!m_include.empty()) {
    if (!m_depsSet) {
      analyzeInclude(ar, m_include);
      m_depsSet = true;
    }
    FileScopePtr fs = ar->findFileScope(m_include, false);
    if (fs) {
      if (!Option::KeepStatementsWithNoEffect) {
        if (ExpressionPtr rep = fs->getEffectiveImpl(ar)) {
          recomputeEffects();
          return replaceValue(rep->clone());
        }
      }
      m_exp.reset();
    }
  }
  return ExpressionPtr();
}

TypePtr IncludeExpression::inferTypes(AnalysisResultPtr ar, TypePtr type,
                                      bool coerce) {
  return UnaryOpExpression::inferTypes(ar, type, coerce);
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void IncludeExpression::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {

  UnaryOpExpression::outputPHP(cg, ar);
}

void IncludeExpression::outputCPPImpl(CodeGenerator &cg,
                                      AnalysisResultPtr ar) {
  // Includes aren't really supported in system mode
  if (cg.getOutput() == CodeGenerator::SystemCPP) {
    cg_printf("true");
    return;
  }
  const char *vars = m_privateScope ?
    "lvar_ptr(LVariableTable())" : "variables";
  bool require = (m_op == T_REQUIRE || m_op == T_REQUIRE_ONCE);
  bool once = (m_op == T_INCLUDE_ONCE || m_op == T_REQUIRE_ONCE);
  if (!getCurrentInclude(ar).empty()) {
    FileScopePtr fs = ar->findFileScope(getCurrentInclude(ar), false);
    if (fs) {
      cg_printf("%s%s(%s, %s, %s)", Option::PseudoMainPrefix,
                fs->pseudoMainName().c_str(),
                once ? "true" : "false",
                vars, cg.getGlobals(ar));
      return;
    }
  }

  // include() and require() need containing file's directory
  string currentDir = "\"\"";
  if (m_loc && m_loc->file && *m_loc->file) {
    string file = m_loc->file;
    size_t pos = file.rfind('/');
    if (pos != string::npos) {
      currentDir = '"';
      currentDir += file.substr(0, pos + 1);
      currentDir += '"';
    }
  }

  // fallback to dynamic include
  cg_printf("%s(", require ? "require" : "include");
  m_exp->outputCPP(cg, ar);
  cg_printf(", %s, %s, %s)",
            once ? "true" : "false",
            vars,
            currentDir.c_str());
}
