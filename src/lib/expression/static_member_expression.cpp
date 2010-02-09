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

#include <lib/expression/static_member_expression.h>
#include <lib/expression/simple_variable.h>
#include <lib/expression/dynamic_variable.h>
#include <lib/expression/scalar_expression.h>
#include <lib/analysis/class_scope.h>
#include <lib/analysis/variable_table.h>
#include <lib/analysis/code_error.h>
#include <util/util.h>
#include <lib/analysis/dependency_graph.h>
#include <lib/option.h>

using namespace HPHP;
using namespace std;
using namespace boost;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

StaticMemberExpression::StaticMemberExpression
(EXPRESSION_CONSTRUCTOR_PARAMETERS,
 const std::string &className, ExpressionPtr exp)
  : Expression(EXPRESSION_CONSTRUCTOR_PARAMETER_VALUES),
    m_exp(exp), m_valid(false), m_dynamicClass(false), m_redeclared(false) {
  m_className = Util::toLower(className);
}

ExpressionPtr StaticMemberExpression::clone() {
  StaticMemberExpressionPtr exp(new StaticMemberExpression(*this));
  Expression::deepCopy(exp);
  return exp;
}

///////////////////////////////////////////////////////////////////////////////
// parser functions

///////////////////////////////////////////////////////////////////////////////
// static analysis functions

void StaticMemberExpression::analyzeProgram(AnalysisResultPtr ar) {
  addUserClass(ar, m_className);
  m_exp->analyzeProgram(ar);
}

ConstructPtr StaticMemberExpression::getNthKid(int n) const {
  switch (n) {
    case 0:
      return m_exp;
    default:
      return ConstructPtr();
  }
  ASSERT(0);
}

int StaticMemberExpression::getKidCount() const {
  return 1;
}

int StaticMemberExpression::setNthKid(int n, ConstructPtr cp) {
  switch (n) {
    case 0:
      m_exp = boost::dynamic_pointer_cast<Expression>(cp);
      return 1;
    default:
      return 0;
  }
  ASSERT(0);
}

ExpressionPtr StaticMemberExpression::preOptimize(AnalysisResultPtr ar) {
  ar->preOptimize(m_exp);
  return ExpressionPtr();
}

ExpressionPtr StaticMemberExpression::postOptimize(AnalysisResultPtr ar) {
  ar->postOptimize(m_exp);
  return ExpressionPtr();
}

/**
 * static_member can only be one of these two forms:
 *
 *   T::$member
 *   T::$$member or T::${$member}, where $member can be an arbitrary expression
 */
TypePtr StaticMemberExpression::inferTypes(AnalysisResultPtr ar,
                                           TypePtr type, bool coerce) {
  ConstructPtr self = shared_from_this();

  ClassScopePtr cls = ar->resolveClass(m_className);
  m_valid = true;
  if (!cls) {
    if (!cls && ar->isFirstPass()) {
      ar->getCodeError()->record(self, CodeError::UnknownClass, self);
    }
    m_valid = false;
  }

  VariableTablePtr variables = ar->getScope()->getVariables();
  variables->setAttribute(VariableTable::NeedGlobalPointer);
  if (cls) {
    if (cls->isRedeclaring()) {
      m_redeclared = true;
    }
    if (cls->derivesFromRedeclaring()) {
      m_dynamicClass = true;
    }
  }

  if (m_exp->is(Expression::KindOfSimpleVariable)) {
    if (!cls) {
      return type;
    }
    SimpleVariablePtr var = dynamic_pointer_cast<SimpleVariable>(m_exp);
    const std::string &name = var->getName();
    int p;
    TypePtr tp;
    if (m_redeclared) {
      BOOST_FOREACH(ClassScopePtr clsr, ar->findClasses(m_className)) {
        clsr->checkStatic(name, type, coerce, ar, self, p);
      }
      tp = Type::Variant;
    } else {
      tp = cls->checkStatic(name, type, coerce, ar, self, p);
    }
    if (ar->isFirstPass() && p && !(p & VariableTable::VariableStatic)) {
      ar->getCodeError()->record(self, CodeError::MissingObjectContext, self);
    }

    if (cls->hasProperty(name) || m_redeclared) {
      m_resolvedClassName = m_className;
    } else {
      ClassScopePtr parent = cls->getVariables()->findParent(ar, name);
      ASSERT(parent);
      m_resolvedClassName = parent->getName();
    }
    return tp;
  } else {
    cls->getVariables()->forceVariants(ar);
  }

  m_exp->inferAndCheck(ar, Type::String, false);
  return Type::Variant; // we have to use a variant to hold dynamic value
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void StaticMemberExpression::outputPHP(CodeGenerator &cg,
                                       AnalysisResultPtr ar) {
  cg.printf("%s::", m_className.c_str());
  m_exp->outputPHP(cg, ar);
}

void StaticMemberExpression::outputCPPImpl(CodeGenerator &cg,
                                           AnalysisResultPtr ar) {
  if (!m_valid) {
    cg.printf("throw_fatal(\"unknown class %s\")", m_className.c_str());
    return;
  }

  if (m_exp->is(Expression::KindOfSimpleVariable) && !m_redeclared &&
      !m_dynamicClass) {
    ASSERT(!m_resolvedClassName.empty());

    ClassScopePtr cls = ar->findClass(m_resolvedClassName);
    ASSERT(cls);
    SimpleVariablePtr var = dynamic_pointer_cast<SimpleVariable>(m_exp);
    if (cls->needLazyStaticInitializer()) {
      cg.printf("%s%s::lazy_initializer(g)->%s%s%s%s",
                Option::ClassPrefix, m_resolvedClassName.c_str(),
                Option::StaticPropertyPrefix, m_resolvedClassName.c_str(),
                Option::IdPrefix.c_str(), var->getName().c_str());
    } else {
      cg.printf("g->%s%s%s%s", Option::StaticPropertyPrefix,
                m_resolvedClassName.c_str(), Option::IdPrefix.c_str(),
                var->getName().c_str());
    }
  } else {
    if (m_context & (LValue | RefValue)) {
      if (m_redeclared) {
        cg.printf("g->%s%s->%slval(", Option::ClassStaticsObjectPrefix,
                  m_className.c_str(),Option::ObjectStaticPrefix);
      } else {
        cg.printf("%s%s::%slval(", Option::ClassPrefix, m_className.c_str(),
                  Option::ObjectStaticPrefix);
      }
    } else {
      if (m_redeclared) {
        cg.printf("g->%s%s->%sget(", Option::ClassStaticsObjectPrefix,
                  m_className.c_str(), Option::ObjectStaticPrefix);
      } else {
        cg.printf("%s%s::%sget(", Option::ClassPrefix, m_className.c_str(),
                  Option::ObjectStaticPrefix);
      }
    }
    if (m_exp->is(Expression::KindOfSimpleVariable)) {
      SimpleVariablePtr var = dynamic_pointer_cast<SimpleVariable>(m_exp);
      cg.printf("\"%s\"", var->getName().c_str());
    } else if (m_exp->is(Expression::KindOfDynamicVariable)) {
      DynamicVariablePtr var = dynamic_pointer_cast<DynamicVariable>(m_exp);
      var->getSubExpression()->outputCPP(cg, ar);
    } else {
      m_exp->outputCPP(cg, ar);
    }
    cg.printf(", -1)");
  }
}
