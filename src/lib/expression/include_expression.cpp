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

#include <lib/expression/include_expression.h>
#include <lib/parser/hphp.tab.hpp>
#include <lib/analysis/dependency_graph.h>
#include <lib/analysis/code_error.h>
#include <lib/analysis/file_scope.h>
#include <lib/analysis/function_scope.h>
#include <lib/statement/statement_list.h>
#include <lib/option.h>
#include <lib/expression/expression_list.h>
#include <lib/analysis/class_scope.h>
#include <lib/parser/parser.h>
#include <lib/analysis/variable_table.h>
#include <lib/expression/scalar_expression.h>
#include <util/util.h>

using namespace HPHP;
using namespace std;
using namespace boost;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

IncludeExpression::IncludeExpression
(EXPRESSION_CONSTRUCTOR_PARAMETERS, ExpressionPtr exp, int op)
  : UnaryOpExpression(EXPRESSION_CONSTRUCTOR_PARAMETER_VALUES, exp, op, true),
    m_documentRoot(false) {
}

ExpressionPtr IncludeExpression::clone() {
  IncludeExpressionPtr exp(new IncludeExpression(*this));
  Expression::deepCopy(exp);
  return exp;
}

///////////////////////////////////////////////////////////////////////////////
// parser functions

void IncludeExpression::onParse(AnalysisResultPtr ar) {
  // See if we can get a string literal
  preOptimize(ar);
  m_include = ar->getDependencyGraph()->add
    (DependencyGraph::KindOfPHPInclude, shared_from_this(), m_exp,
     ar->getCodeError(), m_documentRoot);
}

///////////////////////////////////////////////////////////////////////////////
// static analysis functions

std::string IncludeExpression::getCurrentInclude(AnalysisResultPtr ar) {
  return m_include;
}

void IncludeExpression::analyzeInclude(AnalysisResultPtr ar,
                                       const std::string &include) {
  ASSERT(ar->getPhase() == AnalysisResult::AnalyzeInclude);
  ConstructPtr self = shared_from_this();
  FileScopePtr file = ar->findFileScope(include, true);
  if (!file) {
    ar->getCodeError()->record(self, CodeError::PHPIncludeFileNotFound, self);
    return;
  }
  if (include.find("lib/") != 0 ||
      include.find("/lib/") == string::npos) {
    ar->getCodeError()->record(self, CodeError::PHPIncludeFileNotInLib,
                               self, ConstructPtr(),
                               include.c_str());
  }

  if (!isFileLevel()) { // Not unsupported but potentially bad
    ar->getCodeError()->record(self, CodeError::UseDynamicInclude, self);
  }

  ar->getDependencyGraph()->add
    (DependencyGraph::KindOfProgramMaxInclude,
     ar->getName(), file->getName(), StatementPtr());
  ar->getDependencyGraph()->addParent
    (DependencyGraph::KindOfProgramMinInclude,
     ar->getName(), file->getName(), StatementPtr());
  FunctionScopePtr func = ar->getFunctionScope();
  ar->getFileScope()->addIncludeDependency(ar, m_include,
                                           func && func->isInlined());
}

void IncludeExpression::analyzeProgram(AnalysisResultPtr ar) {
  if (ar->isFirstPass()) {
    ConstructPtr self = shared_from_this();
    if (m_op == T_INCLUDE || m_op == T_REQUIRE) {
      ar->getCodeError()->record(self, CodeError::UseInclude, self);
    }
  }

  string include = getCurrentInclude(ar);
  if (!include.empty()) {
    if (ar->getPhase() == AnalysisResult::AnalyzeInclude) {
      analyzeInclude(ar, include);
    }
  }
  if (!ar->getScope()->inPseudoMain()) {
    VariableTablePtr var = ar->getScope()->getVariables();
    var->setAttribute(VariableTable::ContainsLDynamicVariable);
    var->forceVariants(ar);
  }

  UnaryOpExpression::analyzeProgram(ar);
}

ExpressionPtr IncludeExpression::preOptimize(AnalysisResultPtr ar) {
  return UnaryOpExpression::preOptimize(ar);
}

ExpressionPtr IncludeExpression::postOptimize(AnalysisResultPtr ar) {
  ar->postOptimize(m_exp);
  if (!Option::KeepStatementsWithNoEffect && !m_include.empty()) {
    FileScopePtr fs = ar->findFileScope(m_include, false);
    if (fs && !fs->hasImpl(ar)) {
      return ScalarExpressionPtr
               (new ScalarExpression(getLocation(),
                                     Expression::KindOfScalarExpression,
                                     1));
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
  bool linemap = outputLineMap(cg, ar);

  // Includes aren't really supported in system mode
  if (cg.getOutput() == CodeGenerator::SystemCPP) {
    cg.printf("true");
    if (linemap) cg.printf(")");
    return;
  }
  bool require = (m_op == T_REQUIRE || m_op == T_REQUIRE_ONCE);
  bool once = (m_op == T_INCLUDE_ONCE || m_op == T_REQUIRE_ONCE);
  if (!getCurrentInclude(ar).empty()) {
    FileScopePtr fs = ar->findFileScope(getCurrentInclude(ar), false);
    if (fs) {
      cg.printf("%s%s(%s, variables)", Option::PseudoMainPrefix,
                fs->pseudoMainName().c_str(), once ? "true" : "false");
      if (linemap) cg.printf(")");
      return;
    }
  }

  // fallback to dynamic include
  cg.printf("%s(", require ? "require" : "include");
  m_exp->outputCPP(cg, ar);
  cg.printf(", %s, variables)", once ? "true" : "false");
  if (linemap) cg.printf(")");
}
