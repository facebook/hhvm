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

#include "hphp/compiler/expression/simple_variable.h"
#include "hphp/compiler/analysis/function_scope.h"
#include "hphp/compiler/analysis/class_scope.h"
#include "hphp/compiler/option.h"
#include "hphp/compiler/builtin_symbols.h"
#include "hphp/compiler/expression/scalar_expression.h"
#include "hphp/parser/hphp.tab.hpp"
#include "hphp/parser/parser.h"

using namespace HPHP;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

SimpleVariable::SimpleVariable
(EXPRESSION_CONSTRUCTOR_PARAMETERS,
 const std::string &name,
 const std::string &docComment /* = "" */)
  : Expression(EXPRESSION_CONSTRUCTOR_PARAMETER_VALUES(SimpleVariable)),
    m_name(name), m_docComment(docComment),
    m_this(false), m_globals(false),
    m_superGlobal(false), m_alwaysStash(false) {
}

ExpressionPtr SimpleVariable::clone() {
  SimpleVariablePtr exp(new SimpleVariable(*this));
  Expression::deepCopy(exp);
  return exp;
}

///////////////////////////////////////////////////////////////////////////////
// parser functions

///////////////////////////////////////////////////////////////////////////////
// static analysis functions

void SimpleVariable::setContext(Context context) {
  m_context |= context;
  if (m_this) {
    bool ref = context & (RefValue | RefAssignmentLHS);
    bool unset = ((context & Expression::UnsetContext) &&
      (context & Expression::LValue));
    if (ref || unset) {
      if (FunctionScopePtr func = getFunctionScope()) {
        func->setContainsBareThis(true, true);
      }
    }
  }
}

void SimpleVariable::analyzeProgram(AnalysisResultConstRawPtr ar) {
  m_superGlobal = BuiltinSymbols::IsSuperGlobal(m_name);

  if (m_name == "GLOBALS") {
    m_globals = true;
  } else {
    if (auto const func = getFunctionScope()) {
      func->addLocal(m_name);

      if (ar->getPhase() == AnalysisResult::AnalyzeAll) {
        if (m_name == "this" && func->mayContainThis()) {
          func->setContainsThis();
          m_this = true;
          if (!hasContext(ObjectContext)) {
            bool unset = hasAllContext(UnsetContext | LValue);
            func->setContainsBareThis(
              true,
              hasAnyContext(RefValue | RefAssignmentLHS) || unset);
          }
        }
      }
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void SimpleVariable::outputPHP(CodeGenerator& cg, AnalysisResultPtr /*ar*/) {
  cg_printf("$%s", m_name.c_str());
}
