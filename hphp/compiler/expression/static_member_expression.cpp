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

#include "hphp/compiler/expression/static_member_expression.h"
#include "hphp/compiler/expression/simple_variable.h"
#include "hphp/compiler/expression/dynamic_variable.h"
#include "hphp/compiler/expression/scalar_expression.h"
#include "hphp/compiler/analysis/class_scope.h"
#include "hphp/compiler/analysis/variable_table.h"
#include "hphp/compiler/analysis/code_error.h"
#include "hphp/compiler/analysis/function_scope.h"
#include "hphp/util/text-util.h"
#include "hphp/util/hash.h"
#include "hphp/parser/hphp.tab.hpp"
#include "hphp/compiler/option.h"

using namespace HPHP;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

StaticMemberExpression::StaticMemberExpression
(EXPRESSION_CONSTRUCTOR_PARAMETERS,
 ExpressionPtr classExp, ExpressionPtr exp)
  : Expression(EXPRESSION_CONSTRUCTOR_PARAMETER_VALUES(StaticMemberExpression)),
    StaticClassName(classExp), m_exp(exp), m_valid(false),
    m_dynamicClass(false) {
  if (exp->is(KindOfSimpleVariable)) {
    SimpleVariablePtr s(dynamic_pointer_cast<SimpleVariable>(exp));
    m_exp = ExpressionPtr
      (new ScalarExpression(getScope(), getRange(),
                            T_STRING, s->getName(), true));

  } else {
    always_assert(exp->is(KindOfDynamicVariable));
    m_exp = dynamic_pointer_cast<DynamicVariable>(exp)->getSubExpression();
  }
}

ExpressionPtr StaticMemberExpression::clone() {
  StaticMemberExpressionPtr exp(new StaticMemberExpression(*this));
  Expression::deepCopy(exp);
  if (m_class) exp->m_class = m_class->clone();
  exp->m_exp = m_exp->clone();
  return exp;
}

///////////////////////////////////////////////////////////////////////////////
// parser functions

///////////////////////////////////////////////////////////////////////////////
// static analysis functions

bool StaticMemberExpression::findMember(AnalysisResultPtr ar, string &name,
                                        Symbol *&sym) {
  if (m_exp->is(Expression::KindOfScalarExpression)) {
    ScalarExpressionPtr var = dynamic_pointer_cast<ScalarExpression>(m_exp);
    name = var->getString();
  }

  if (m_class) return false;

  sym = nullptr;
  m_resolvedClass = resolveClass();
  if (!m_resolvedClass) return isRedeclared();

  if (m_resolvedClass->derivesFromRedeclaring() == Derivation::Redeclaring) {
    m_dynamicClass = true;
  }

  if (m_dynamicClass) return true;

  if (!name.empty()) {
    ClassScopePtr parent = m_resolvedClass;
    sym = m_resolvedClass->findProperty(parent, name, ar);
    if (sym && sym->isStatic()) {
      m_resolvedClass = parent;
    } else {
      sym = nullptr;
    }
  }

  return true;
}

void StaticMemberExpression::analyzeProgram(AnalysisResultPtr ar) {
  if (m_class) {
    m_class->analyzeProgram(ar);
  } else if (ar->getPhase() >= AnalysisResult::AnalyzeAll) {
    Symbol *sym;
    string name;
    if (findMember(ar, name, sym)) {
      if (m_resolvedClass) {
        m_resolvedClass->addUse(getScope(), BlockScope::UseKindStaticRef);
        if (!sym && !m_dynamicClass && !name.empty() &&
            ar->getPhase() == AnalysisResult::AnalyzeFinal &&
            !m_resolvedClass->isTrait()) {
          Compiler::Error(Compiler::UseUndeclaredVariable, shared_from_this());
        }
      }
    }
  }
  m_exp->analyzeProgram(ar);
}

ConstructPtr StaticMemberExpression::getNthKid(int n) const {
  switch (n) {
    case 0:
      return m_class;
    case 1:
      return m_exp;
    default:
      assert(false);
      break;
  }
  return ConstructPtr();
}

int StaticMemberExpression::getKidCount() const {
  return 2;
}

void StaticMemberExpression::setNthKid(int n, ConstructPtr cp) {
  switch (n) {
    case 0:
      m_class = dynamic_pointer_cast<Expression>(cp);
      break;
    case 1:
      m_exp = dynamic_pointer_cast<Expression>(cp);
      break;
    default:
      assert(false);
      break;
  }
}

ExpressionPtr StaticMemberExpression::preOptimize(AnalysisResultConstPtr ar) {
  if (m_class) updateClassName();
  return ExpressionPtr();
}

///////////////////////////////////////////////////////////////////////////////

void StaticMemberExpression::outputCodeModel(CodeGenerator &cg) {
  cg.printObjectHeader("ClassPropertyExpression", 3);
  StaticClassName::outputCodeModel(cg);
  if (m_exp->is(Expression::KindOfScalarExpression)) {
    cg.printPropertyHeader("propertyName");
    ScalarExpressionPtr var = dynamic_pointer_cast<ScalarExpression>(m_exp);
    cg.printValue(var->getString());
  } else {
    cg.printPropertyHeader("propertyExpression");
    m_exp->outputCodeModel(cg);
  }
  cg.printPropertyHeader("sourceLocation");
  cg.printLocation(this);
  cg.printObjectFooter();
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void StaticMemberExpression::outputPHP(CodeGenerator &cg,
                                       AnalysisResultPtr ar) {
  StaticClassName::outputPHP(cg, ar);

  cg_printf("::$");
  bool needsClose = false;
  switch (m_exp->getKindOf()) {
  case KindOfScalarExpression:
    {
      ScalarExpressionPtr var = dynamic_pointer_cast<ScalarExpression>(m_exp);
      cg_printf("%s", var->getString().c_str());
      return;
    }
  case KindOfSimpleVariable:
  case KindOfDynamicVariable:
    break;
  default:
    cg_printf("{");
    needsClose = true;
  }
  m_exp->outputPHP(cg, ar);
  if (needsClose) {
    cg_printf("}");
  }
}
