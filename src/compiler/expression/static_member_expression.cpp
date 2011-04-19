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

#include <compiler/expression/static_member_expression.h>
#include <compiler/expression/simple_variable.h>
#include <compiler/expression/dynamic_variable.h>
#include <compiler/expression/scalar_expression.h>
#include <compiler/analysis/class_scope.h>
#include <compiler/analysis/variable_table.h>
#include <compiler/analysis/code_error.h>
#include <compiler/analysis/function_scope.h>
#include <util/util.h>
#include <util/hash.h>
#include <util/parser/hphp.tab.hpp>
#include <compiler/option.h>

using namespace HPHP;
using namespace std;
using namespace boost;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

StaticMemberExpression::StaticMemberExpression
(EXPRESSION_CONSTRUCTOR_PARAMETERS,
 ExpressionPtr classExp, ExpressionPtr exp)
  : Expression(EXPRESSION_CONSTRUCTOR_PARAMETER_VALUES),
    StaticClassName(classExp), m_exp(exp), m_valid(false),
    m_dynamicClass(false) {
  if (exp->is(KindOfSimpleVariable)) {
    SimpleVariablePtr s(dynamic_pointer_cast<SimpleVariable>(exp));
    m_exp = ExpressionPtr
      (new ScalarExpression(getScope(), getLocation(),
                            KindOfScalarExpression,
                            T_STRING, s->getName(), true));

  } else {
    assert(exp->is(KindOfDynamicVariable));
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

  sym = NULL;
  m_resolvedClass = resolveClass();
  if (!m_resolvedClass) return isRedeclared();

  if (m_resolvedClass->derivesFromRedeclaring()) {
    m_dynamicClass = true;
  }

  if (m_dynamicClass) return true;

  if (!name.empty()) {
    ClassScopePtr parent = m_resolvedClass;
    sym = m_resolvedClass->findProperty(parent, name, ar, shared_from_this());
    if (sym && sym->isStatic()) {
      m_resolvedClass = parent;
    } else {
      sym = NULL;
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
            ar->getPhase() == AnalysisResult::AnalyzeAll) {
          Compiler::Error(Compiler::UseUndeclaredVariable, shared_from_this());
        }
      }
    }
    addUserClass(ar, m_className);
  }
  m_exp->analyzeProgram(ar);
  if (ar->getPhase() >= AnalysisResult::AnalyzeFinal) {
    FunctionScopePtr fs = getFunctionScope();
    if (fs) fs->setNeedsCheckMem();
  }
}

ConstructPtr StaticMemberExpression::getNthKid(int n) const {
  switch (n) {
    case 0:
      return m_class;
    case 1:
      return m_exp;
    default:
      ASSERT(false);
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
      m_class = boost::dynamic_pointer_cast<Expression>(cp);
      break;
    case 1:
      m_exp = boost::dynamic_pointer_cast<Expression>(cp);
      break;
    default:
      ASSERT(false);
      break;
  }
}

ExpressionPtr StaticMemberExpression::preOptimize(AnalysisResultConstPtr ar) {
  if (m_class) updateClassName();
  return ExpressionPtr();
}

ExpressionPtr StaticMemberExpression::postOptimize(AnalysisResultConstPtr ar) {
  Symbol *sym = NULL;
  if (m_class) updateClassName();
  if (!m_class && m_resolvedClass && m_valid &&
      m_exp->is(Expression::KindOfScalarExpression)) {
    ClassScopePtr cls = ar->findExactClass(shared_from_this(), m_className);
    if (cls && (!cls->isVolatile() || isPresent())) {
      ScalarExpressionPtr var = dynamic_pointer_cast<ScalarExpression>(m_exp);
      const std::string &name = var->getString();

      sym = cls->findProperty(cls, name, ar, shared_from_this());
      if (sym && !sym->isIndirectAltered() && sym->isStatic()) {
        ConstructPtr init = sym->getClassInitVal();
        if (init) {
          ExpressionPtr rep = dynamic_pointer_cast<Expression>(init);
          if (rep->isScalar()) {
            return replaceValue(Clone(rep, getScope()));
          }
        }
      }
    }
  }
  return ExpressionPtr();
}

/**
 * static_member can only be one of these two forms:
 *
 *   T::$member
 *   T::$$member or T::${$member}, where $member can be an arbitrary expression
 *   The former is represented by a ScalarExpression with value "member",
 *   the latter by the expression $member.
 */
TypePtr StaticMemberExpression::inferTypes(AnalysisResultPtr ar,
                                           TypePtr type, bool coerce) {
  ConstructPtr self = shared_from_this();

  bool modified = m_context & (LValue | RefValue | UnsetContext | RefParameter);
  if (m_context & (RefValue|RefParameter)) {
    coerce = true;
    type = Type::Variant;
  }
  if (m_class) {
    if (modified) {
      if (m_exp->is(Expression::KindOfScalarExpression)) {
        ScalarExpressionPtr var = dynamic_pointer_cast<ScalarExpression>(m_exp);
        const std::string &name = var->getString();
        ar->forceClassVariants(name, getOriginalClass(), true);
      } else {
        ar->forceClassVariants(getOriginalClass(), true);
      }
    }
    m_class->inferAndCheck(ar, Type::Any, false);
    m_exp->inferAndCheck(ar, Type::String, false);
    return Type::Variant;
  }

  m_exp->inferAndCheck(ar, Type::String, false);
  m_valid = true;

  Symbol *sym;
  string name;
  m_valid = findMember(ar, name, sym);
  if (!m_valid) {
    if (getScope()->isFirstPass()) {
      Compiler::Error(Compiler::UnknownClass, self);
    }
  } else if (m_resolvedClass) {
    m_resolvedClass->addUse(getScope(), BlockScope::UseKindStaticRef);
  }

  VariableTablePtr variables = getScope()->getVariables();
  variables->setAttribute(VariableTable::NeedGlobalPointer);

  if (!name.empty()) {
    if (!m_resolvedClass && !isRedeclared()) {
      m_implementedType.reset();
      return Type::Variant;
    }
    bool found = false;
    TypePtr tp;
    if (isRedeclared()) {
      BOOST_FOREACH(ClassScopePtr clsr,
                    ar->findRedeclaredClasses(m_className)) {
        sym = clsr->findProperty(clsr, name, ar, self);
        if (sym && sym->isStatic()) {
          clsr->checkProperty(sym, type, coerce, ar);
          found = true;
        }
      }
      tp = Type::Variant;
      sym = NULL;
    } else if (sym) {
      tp = m_resolvedClass->checkProperty(sym, type, coerce, ar);
      found = true;
      if (modified) {
        sym->setIndirectAltered();
      }
    } else {
      tp = Type::Variant;
    }
    if (!found && getScope()->isFirstPass()) {
      Compiler::Error(Compiler::UseUndeclaredVariable, self);
    }
    m_valid = found || isRedeclared() || m_dynamicClass;
    m_implementedType.reset();
    return tp;
  }

  if (m_resolvedClass || isRedeclared()) {
    if (modified) {
      if (isRedeclared()) {
        BOOST_FOREACH(ClassScopePtr clsr,
                      ar->findRedeclaredClasses(m_className)) {
          int mask = clsr == getOriginalClass() ?
            VariableTable::AnyStaticVars : VariableTable::NonPrivateStaticVars;
          clsr->getVariables()->forceVariants(ar, mask);
        }
      } else {
        int mask = m_resolvedClass == getOriginalClass() ?
          VariableTable::AnyStaticVars : VariableTable::NonPrivateStaticVars;
        m_resolvedClass->getVariables()->forceVariants(ar, mask);
      }
    }
  }

  // we have to use a variant to hold dynamic value
  m_implementedType.reset();
  return Type::Variant;
}

unsigned StaticMemberExpression::getCanonHash() const {
  int64 val = Expression::getCanonHash() +
    hash_string(Util::toLower(m_className).c_str(), m_className.size());
  return ~unsigned(val) ^ unsigned(val >> 32);
}

bool StaticMemberExpression::canonCompare(ExpressionPtr e) const {
  if (!Expression::canonCompare(e)) return false;
  StaticMemberExpressionPtr s =
    static_pointer_cast<StaticMemberExpression>(e);
  return m_className == s->m_className;
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

void StaticMemberExpression::preOutputStash(CodeGenerator &cg,
                                            AnalysisResultPtr ar,
                                            int state)
{
  if (getContext() & (LValue|RefValue|RefParameter)) return;
  Expression::preOutputStash(cg, ar, state);
}

void StaticMemberExpression::outputCPPImpl(CodeGenerator &cg,
                                           AnalysisResultPtr ar) {
  if (m_class) {
    const char *func_suffix = "";
    if (m_context & (LValue | RefValue | UnsetContext)) {
      func_suffix = "_lval";
    }

    if (!hasContext(UnsetContext) || !hasContext(LValue)) {
      cg_printf("get_static_property%s(", func_suffix);
    } else {
      cg_printf("throw_fatal_unset_static_property(");
    }

    if (m_class->is(KindOfScalarExpression)) {
      ASSERT(strcasecmp(dynamic_pointer_cast<ScalarExpression>(m_class)->
                        getString().c_str(), "static") == 0);
      cg_printf("FrameInjection::GetStaticClassName(fi.getThreadInfo())"
                ".data()");
    } else {
      cg_printf("get_static_class_name(");
      m_class->outputCPP(cg, ar);
      cg_printf(").data()");
    }

    cg_printf(", toString(");
    m_exp->outputCPP(cg, ar);
    cg_printf(").data())");
    return;
  }

  if (hasContext(UnsetContext) && hasContext(LValue)) {
    cg_printf("throw_fatal_unset_static_property(\"%s\"",
              m_origClassName.c_str());

    cg_printf(", toString(");
    m_exp->outputCPP(cg, ar);
    cg_printf(").data())");
    return;
  }

  if (!m_valid) {
    if (!m_resolvedClass && !isRedeclared()) {
      cg_printf("throw_fatal(\"unknown class %s\")", m_origClassName.c_str());
    } else {
      cg_printf("throw_fatal(\"Access to undeclared static property: ");
      outputPHP(cg, ar);
      cg_printf("\")");
    }
    return;
  }

  bool volatileCheck = false;
  if (!isPresent() &&
      ((m_resolvedClass && m_resolvedClass->isVolatile()) ||
       isRedeclared())) {
    volatileCheck = true;
    ClassScope::OutputVolatileCheckBegin(cg, ar, getScope(),
                                         m_origClassName);
  }

  if (m_exp->is(Expression::KindOfScalarExpression) && !isRedeclared() &&
      !m_dynamicClass) {

    ASSERT(m_resolvedClass);
    ScalarExpressionPtr var = dynamic_pointer_cast<ScalarExpression>(m_exp);
    string clsId = m_resolvedClass->getId(cg);
    if (m_resolvedClass->needLazyStaticInitializer()) {
      cg_printf("%s%s::lazy_initializer(g)->%s%s%s%s",
                Option::ClassPrefix, clsId.c_str(),
                Option::StaticPropertyPrefix, clsId.c_str(),
                Option::IdPrefix.c_str(),
                cg.formatLabel(var->getString()).c_str());
    } else {
      cg_printf("g->%s%s%s%s", Option::StaticPropertyPrefix, clsId.c_str(),
                Option::IdPrefix.c_str(),
                cg.formatLabel(var->getString()).c_str());
    }
  } else {
    if (m_context & (LValue | RefValue | UnsetContext)) {
      if (isRedeclared()) {
        cg_printf("g->%s%s->%slval(", Option::ClassStaticsObjectPrefix,
                  cg.formatLabel(m_className).c_str(),
                  Option::ObjectStaticPrefix);
      } else {
        cg_printf("%s%s::%slval(", Option::ClassPrefix,
                  m_resolvedClass->getId(cg).c_str(),
                  Option::ObjectStaticPrefix);
      }
    } else {
      if (isRedeclared()) {
        cg_printf("g->%s%s->%sget(", Option::ClassStaticsObjectPrefix,
                  cg.formatLabel(m_className).c_str(),
                  Option::ObjectStaticPrefix);
      } else {
        cg_printf("%s%s::%sget(", Option::ClassPrefix,
                  m_resolvedClass->getId(cg).c_str(),
                  Option::ObjectStaticPrefix);
      }
    }
    m_exp->outputCPP(cg, ar);
    cg_printf(")");
  }

  if (volatileCheck) {
    ClassScope::OutputVolatileCheckEnd(cg);
  }
}
