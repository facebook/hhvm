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

#include <compiler/expression/include_expression.h>
#include <util/parser/hphp.tab.hpp>
#include <compiler/analysis/code_error.h>
#include <compiler/analysis/file_scope.h>
#include <compiler/analysis/function_scope.h>
#include <compiler/statement/statement_list.h>
#include <compiler/option.h>
#include <compiler/expression/expression_list.h>
#include <compiler/expression/binary_op_expression.h>
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

static string get_include_file_path(const string &source, string expText,
                                    bool documentRoot) {
  if (expText.size() <= 2) return "";

  char first = expText[0];
  char last = expText[expText.size() - 1];

  // (exp)
  if (first == '(' && last == ')') {
    expText = expText.substr(1, expText.size() - 2);
    return get_include_file_path(source, expText, documentRoot);
  }

  // 'string'
  if ((first == '\'' && last == '\'') || (first == '"' && last == '"')) {
    expText = expText.substr(1, expText.size() - 2);

    // absolute path
    if (!expText.empty() && expText[0] == '/') {
      return expText;
    }

    // relative path to document root
    if (documentRoot) {
      return expText;
    }

    struct stat sb;

    // relative path to containing file's directory
    ASSERT(source.size() > 1);
    size_t pos = source.rfind('/');
    string resolved;
    if (pos != string::npos) {
      resolved = source.substr(0, pos + 1) + expText;
      if (stat(resolved.c_str(), &sb) == 0) {
        return resolved;
      }
    }

    // if file cannot be found, resolve it using search paths
    for (unsigned int i = 0; i < Option::IncludeSearchPaths.size(); i++) {
      string filename = Option::IncludeSearchPaths[i] + "/" + expText;
      if (stat(filename.c_str(), &sb) == 0) {
        return filename;
      }
    }

    // try still use relative path to containing file's directory
    if (!resolved.empty()) {
      return resolved;
    }

    return expText;
  }

  // [IncludeRoot] . 'string'
  for (map<string, string>::const_iterator iter = Option::IncludeRoots.begin();
       iter != Option::IncludeRoots.end(); iter++) {
    string rootExp = iter->first + " . ";
    int rootLen = rootExp.size();
    if (expText.substr(0, rootLen) == rootExp &&
        (int)expText.length() > rootLen + 2 &&
        ((expText[rootLen] == '\'' && last == '\'') ||
         (expText[rootLen] == '"' && last == '"'))) {
      expText = expText.substr(rootLen + 1, expText.length() - rootLen - 2);

      string includeRoot = iter->second;
      if (!includeRoot.empty()) {
        if (includeRoot[0] == '/') includeRoot = includeRoot.substr(1);
        if (includeRoot.empty() ||
            includeRoot[includeRoot.size()-1] != '/') {
          includeRoot += "/";
        }
      }
      if (!expText.empty() && expText[0] == '/') {
        expText = expText.substr(1);
      }
      expText = includeRoot + expText;
      return expText;
    }
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
                                       ExpressionPtr fileExp,
                                       bool documentRoot) {
  string container = includeExp->getLocation()->file;
  string var, lit;
  parse_string_arg(fileExp, var, lit);
  if (!lit.empty()) {
    if (!var.empty()) {
      var += " . ";
    }
    var += "'" + lit + "'";
  }

  string included = get_include_file_path(container, var, documentRoot);
  included = Util::canonicalize(included);
  if (included.empty() || container == included) {
    if (!included.empty() && included.find(' ') == string::npos) {
      Compiler::Error(Compiler::BadPHPIncludeFile, includeExp);
    }
    return "";
  }
  return included;
}

void IncludeExpression::onParse(AnalysisResultConstPtr ar, FileScopePtr scope) {
  m_include = CheckInclude(shared_from_this(), m_exp, m_documentRoot);
  if (!m_include.empty()) ar->parseOnDemand(m_include);
}

///////////////////////////////////////////////////////////////////////////////
// static analysis functions

bool IncludeExpression::analyzeInclude(AnalysisResultConstPtr ar,
                                       const std::string &include) {
  ConstructPtr self = shared_from_this();
  FileScopePtr file = ar->findFileScope(include);
  if (!file) {
    if (!include.empty() && include.find(' ') == string::npos) {
      Compiler::Error(Compiler::PHPIncludeFileNotFound, self);
    }
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
      if (analyzeInclude(ar, m_include)) {
        FunctionScopePtr func = getFunctionScope();
        getFileScope()->addIncludeDependency(ar, m_include,
                                             func && func->isInlined());
      }
    }
  }
  if (!getScope()->inPseudoMain() && !m_privateScope) {
    VariableTablePtr var = getScope()->getVariables();
    var->setAttribute(VariableTable::ContainsLDynamicVariable);
    var->forceVariants(ar, VariableTable::AnyVars);
  }

  UnaryOpExpression::analyzeProgram(ar);
}

ExpressionPtr IncludeExpression::preOptimize(AnalysisResultConstPtr ar) {
  if (ar->getPhase() >= AnalysisResult::FirstPreOptimize) {
    if (m_include.empty()) {
      m_include = CheckInclude(shared_from_this(), m_exp, m_documentRoot);
      m_depsSet = false;
    }
    if (!m_depsSet && !m_include.empty()) {
      analyzeInclude(ar, m_include);
      m_depsSet = true;
    }
  }
  return ExpressionPtr();
}

ExpressionPtr IncludeExpression::postOptimize(AnalysisResultConstPtr ar) {
  if (!m_include.empty()) {
    if (!m_depsSet) {
      analyzeInclude(ar, m_include);
      m_depsSet = true;
    }
    FileScopePtr fs = ar->findFileScope(m_include);
    if (fs && fs->getPseudoMain()) {
      if (!Option::KeepStatementsWithNoEffect) {
        if (ExpressionPtr rep = fs->getEffectiveImpl(ar)) {
          recomputeEffects();
          return replaceValue(rep->clone());
        }
      }
      m_exp.reset();
    } else {
      m_include = "";
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
  if (!m_include.empty()) {
    FileScopePtr fs = ar->findFileScope(m_include);
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
