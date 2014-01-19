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

#include "hphp/compiler/expression/static_member_expression.h"
#include "hphp/compiler/expression/simple_variable.h"
#include "hphp/compiler/expression/dynamic_variable.h"
#include "hphp/compiler/expression/scalar_expression.h"
#include "hphp/compiler/analysis/class_scope.h"
#include "hphp/compiler/analysis/variable_table.h"
#include "hphp/compiler/analysis/code_error.h"
#include "hphp/compiler/analysis/function_scope.h"
#include "hphp/util/util.h"
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
      (new ScalarExpression(getScope(), getLocation(),
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

  if (m_resolvedClass->derivesFromRedeclaring()) {
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
    addUserClass(ar, m_className);
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

ExpressionPtr StaticMemberExpression::postOptimize(AnalysisResultConstPtr ar) {
  Symbol *sym = nullptr;
  if (m_class) updateClassName();
  if (m_class || !m_resolvedClass || !m_valid ||
      !m_exp->is(Expression::KindOfScalarExpression)) {
    return ExpressionPtr();
  }

  ClassScopePtr cls = ar->findExactClass(shared_from_this(), m_className);
  if (!cls || (cls->isVolatile() && !isPresent())) {
    return ExpressionPtr();
  }

  ScalarExpressionPtr var = dynamic_pointer_cast<ScalarExpression>(m_exp);
  const std::string &name = var->getString();

  sym = cls->findProperty(cls, name, ar);
  if (sym && !sym->isIndirectAltered() && sym->isStatic()) {
    if (sym->isPrivate() ? cls == getClassScope() :
        sym->isProtected() ?
        getClassScope() && getClassScope()->derivesFrom(ar, cls->getName(),
                                                        true, false) :
        true) {
      ConstructPtr init = sym->getClassInitVal();
      if (init) {
        ExpressionPtr rep = dynamic_pointer_cast<Expression>(init);
        if (rep->isScalar()) {
          ExpressionPtr repClone = Clone(rep, getScope());
          if (!repClone->getActualType()) {
            repClone->setActualType(getActualType());
          }
          return replaceValue(repClone);
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
  assert(getScope()->is(BlockScope::FunctionScope));
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
        ar->forceClassVariants(name, getOriginalClass(), true, true);
      } else {
        ar->forceClassVariants(getOriginalClass(), true, true);
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
      ClassScopeRawPtr cscope = getClassScope();
      if (!cscope ||
          !cscope->isTrait() ||
          (!isSelf() && !isParent())) {
        Compiler::Error(Compiler::UnknownClass, self);
      }
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
        sym = clsr->findProperty(clsr, name, ar);
        if (sym && sym->isStatic()) {
          {
            GET_LOCK(clsr);
            if (modified) {
              sym->setType(ar, getScope(), Type::Variant, true);
              sym->setIndirectAltered();
            } else {
              clsr->checkProperty(getScope(), sym, type, coerce, ar);
            }
          }
          found = true;
        }
      }
      tp = Type::Variant;
      sym = nullptr;
    } else if (sym) {
      assert(m_resolvedClass);
      {
        GET_LOCK(m_resolvedClass);
        tp = m_resolvedClass->checkProperty(getScope(), sym, type, coerce, ar);
      }
      found = true;
      if (modified) {
        // concurrent modifications here are OK because:
        // 1) you never clear the bit (you only set it to true)
        // 2) the value isn't read in type inference
        sym->setIndirectAltered();
      }
    } else {
      GET_LOCK(m_resolvedClass);
      m_resolvedClass->getVariables()->
        forceVariant(ar, name,
                     m_resolvedClass == getOriginalClass() ?
                     VariableTable::AnyStaticVars :
                     VariableTable::NonPrivateStaticVars);
      tp = Type::Variant;
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
          GET_LOCK(clsr);
          clsr->getVariables()->forceVariants(ar, mask);
        }
      } else {
        int mask = m_resolvedClass == getOriginalClass() ?
          VariableTable::AnyStaticVars : VariableTable::NonPrivateStaticVars;
        GET_LOCK(m_resolvedClass);
        m_resolvedClass->getVariables()->forceVariants(ar, mask);
      }
    }
  }

  // we have to use a variant to hold dynamic value
  m_implementedType.reset();
  return Type::Variant;
}

unsigned StaticMemberExpression::getCanonHash() const {
  int64_t val = Expression::getCanonHash() +
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

void StaticMemberExpression::outputCodeModel(CodeGenerator &cg) {
  cg.printObjectHeader("ClassPropertyExpression", 3);
  cg.printPropertyHeader("className");
  StaticClassName::outputCodeModel(cg);
  if (m_exp->is(Expression::KindOfScalarExpression)) {
    cg.printPropertyHeader("propertyName");
  } else {
    cg.printPropertyHeader("propertyExpression");
  }
  m_exp->outputCodeModel(cg);
  cg.printPropertyHeader("sourceLocation");
  cg.printLocation(this->getLocation());
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
