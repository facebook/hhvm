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

#include "hphp/compiler/expression/include_expression.h"
#include <map>
#include "hphp/parser/hphp.tab.hpp"
#include "hphp/compiler/analysis/code_error.h"
#include "hphp/compiler/analysis/file_scope.h"
#include "hphp/compiler/analysis/function_scope.h"
#include "hphp/compiler/statement/statement_list.h"
#include "hphp/compiler/option.h"
#include "hphp/compiler/expression/expression_list.h"
#include "hphp/compiler/expression/binary_op_expression.h"
#include "hphp/compiler/analysis/class_scope.h"
#include "hphp/compiler/parser/parser.h"
#include "hphp/compiler/analysis/variable_table.h"
#include "hphp/compiler/expression/scalar_expression.h"
#include "hphp/runtime/base/file-util.h"

using namespace HPHP;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

IncludeExpression::IncludeExpression
(EXPRESSION_CONSTRUCTOR_PARAMETERS, ExpressionPtr exp, int op)
  : UnaryOpExpression(
      EXPRESSION_CONSTRUCTOR_PARAMETER_VALUES(IncludeExpression),
      exp, op, true),
    m_documentRoot(false), m_depsSet(false) {
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

static string get_include_file_path(const string &source,
                                    const string &var, const string &lit,
                                    bool documentRoot) {
  if (var.empty()) {
    // absolute path
    if (!lit.empty() && lit[0] == '/') {
      return lit;
    }

    // relative path to document root
    if (documentRoot) {
      return lit;
    }

    struct stat sb;
    // relative path to containing file's directory
    if (source.empty() && (stat(lit.c_str(), &sb) == 0)) {
      return lit;
    }

    size_t pos = source.rfind('/');
    string resolved;
    if (pos != string::npos) {
      resolved = source.substr(0, pos + 1) + lit;
      if (stat(resolved.c_str(), &sb) == 0) {
        return resolved;
      }
    }

    // if file cannot be found, resolve it using search paths
    for (unsigned int i = 0; i < Option::IncludeSearchPaths.size(); i++) {
      string filename = Option::IncludeSearchPaths[i] + "/" + lit;
      struct stat sb;
      if (stat(filename.c_str(), &sb) == 0) {
        return filename;
      }
    }

    // try still use relative path to containing file's directory
    if (!resolved.empty()) {
      return resolved;
    }

    return lit;
  }

  // [IncludeRoot] . 'string'
  std::map<string, string>::const_iterator iter =
    Option::IncludeRoots.find(var);

  if (iter != Option::IncludeRoots.end()) {
    string includeRoot = iter->second;
    if (!includeRoot.empty()) {
      if (includeRoot[0] == '/') includeRoot = includeRoot.substr(1);
      if (includeRoot.empty() ||
          includeRoot[includeRoot.size()-1] != '/') {
        includeRoot += "/";
      }
    }
    if (!lit.empty() && lit[0] == '/') {
      includeRoot += lit.substr(1);
    } else {
      includeRoot += lit;
    }
    return includeRoot;
  }

  return "";
}

static void parse_string_arg(ExpressionPtr exp, string &var, string &lit) {
  if (exp->is(Expression::KindOfUnaryOpExpression)) {
    UnaryOpExpressionPtr u(static_pointer_cast<UnaryOpExpression>(exp));
    if (u->getOp() == '(') {
      parse_string_arg(u->getExpression(), var, lit);
      return;
    }
  } else if (exp->is(Expression::KindOfBinaryOpExpression)) {
    BinaryOpExpressionPtr b(static_pointer_cast<BinaryOpExpression>(exp));
    if (b->getOp() == '.') {
      string v, l;
      parse_string_arg(b->getExp2(), v, l);
      if (v.empty()) {
        parse_string_arg(b->getExp1(), var, lit);
        lit += l;
        return;
      }
    }
  }
  if (exp->isLiteralString()) {
    var = "";
    lit = exp->getLiteralString();
    return;
  }
  var = exp->getText();
  lit = "";
  return;
}

string IncludeExpression::CheckInclude(ConstructPtr includeExp,
                                       FileScopePtr scope,
                                       ExpressionPtr fileExp,
                                       bool &documentRoot) {
  string container = scope->getName();
  string var, lit;
  parse_string_arg(fileExp, var, lit);
  if (lit.empty()) return lit;

  if (var == "__DIR__") {
    var = "";
    // get_include_file_path will check relative to the current file's dir
    // as long as the first char isn't a /
    if (lit[0] == '/') {
      lit = lit.substr(1);
    }
  }

  string included = get_include_file_path(container, var, lit, documentRoot);
  if (!included.empty()) {
    if (included == container) {
      Compiler::Error(Compiler::BadPHPIncludeFile, includeExp);
    }
    included = FileUtil::canonicalize(included).toCppString();
    if (!var.empty()) documentRoot = true;
  }
  return included;
}

void IncludeExpression::onParse(AnalysisResultConstPtr ar, FileScopePtr scope) {
  /* m_documentRoot is a bitfield */
  bool dr = m_documentRoot;
  m_include = CheckInclude(shared_from_this(), scope, m_exp, dr);
  m_documentRoot = dr;
  if (!m_include.empty()) ar->parseOnDemand(m_include);
}

///////////////////////////////////////////////////////////////////////////////
// static analysis functions

FileScopeRawPtr IncludeExpression::getIncludedFile(
  AnalysisResultConstPtr ar) const {
  if (m_include.empty()) return FileScopeRawPtr();
  return ar->findFileScope(m_include);
}

std::string IncludeExpression::includePath() const {
  return m_include;
}

bool IncludeExpression::isReqLit() const {
  return !m_include.empty() &&
    m_op == T_REQUIRE_ONCE && isDocumentRoot();
}

bool IncludeExpression::analyzeInclude(AnalysisResultConstPtr ar,
                                       const std::string &include) {
  ConstructPtr self = shared_from_this();
  FileScopePtr file = ar->findFileScope(include);
  if (!file) {
    Compiler::Error(Compiler::PHPIncludeFileNotFound, self);
    return false;
  }

  FunctionScopePtr func = getFunctionScope();
  if (func && file->getPseudoMain()) {
    file->getPseudoMain()->addUse(func, BlockScope::UseKindInclude);
  }
  return true;
}

void IncludeExpression::analyzeProgram(AnalysisResultPtr ar) {
  if (!m_include.empty()) {
    if (ar->getPhase() == AnalysisResult::AnalyzeAll ||
        ar->getPhase() == AnalysisResult::AnalyzeFinal) {
      analyzeInclude(ar, m_include);
    }
  }

  VariableTablePtr var = getScope()->getVariables();
  var->setAttribute(VariableTable::ContainsLDynamicVariable);

  UnaryOpExpression::analyzeProgram(ar);
}

ExpressionPtr IncludeExpression::preOptimize(AnalysisResultConstPtr ar) {
  if (ar->getPhase() >= AnalysisResult::FirstPreOptimize) {
    if (m_include.empty()) {
      bool dr = m_documentRoot;
      m_include = CheckInclude(shared_from_this(), getFileScope(), m_exp, dr);
      m_documentRoot = dr;
      m_depsSet = false;
    }
    if (!m_depsSet && !m_include.empty()) {
      analyzeInclude(ar, m_include);
      m_depsSet = true;
    }
  }
  return ExpressionPtr();
}

///////////////////////////////////////////////////////////////////////////////

void IncludeExpression::outputCodeModel(CodeGenerator &cg) {
  UnaryOpExpression::outputCodeModel(cg);
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void IncludeExpression::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  UnaryOpExpression::outputPHP(cg, ar);
}
