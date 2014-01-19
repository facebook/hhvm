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

#include "hphp/compiler/expression/object_property_expression.h"
#include "hphp/compiler/expression/scalar_expression.h"
#include "hphp/compiler/expression/expression_list.h"
#include "hphp/compiler/analysis/code_error.h"
#include "hphp/compiler/analysis/class_scope.h"
#include "hphp/compiler/analysis/function_scope.h"
#include "hphp/compiler/analysis/file_scope.h"
#include "hphp/compiler/analysis/variable_table.h"
#include "hphp/compiler/option.h"
#include "hphp/compiler/expression/simple_variable.h"
#include "hphp/util/hash.h"
#include "hphp/parser/hphp.tab.hpp"

using namespace HPHP;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

ObjectPropertyExpression::ObjectPropertyExpression
(EXPRESSION_CONSTRUCTOR_PARAMETERS,
 ExpressionPtr object, ExpressionPtr property)
  : Expression(
      EXPRESSION_CONSTRUCTOR_PARAMETER_VALUES(ObjectPropertyExpression)),
    LocalEffectsContainer(AccessorEffect),
    m_object(object), m_property(property), m_propSym(nullptr) {
  m_valid = false;
  m_propSymValid = false;
  m_object->setContext(Expression::ObjectContext);
  m_object->setContext(Expression::AccessContext);
}

ExpressionPtr ObjectPropertyExpression::clone() {
  ObjectPropertyExpressionPtr exp(new ObjectPropertyExpression(*this));
  Expression::deepCopy(exp);
  exp->m_object = Clone(m_object);
  exp->m_property = Clone(m_property);
  return exp;
}

///////////////////////////////////////////////////////////////////////////////
// parser functions

///////////////////////////////////////////////////////////////////////////////
// static analysis functions

bool ObjectPropertyExpression::isTemporary() const {
  return !m_valid && !(m_context & (LValue | RefValue | UnsetContext));
}

bool ObjectPropertyExpression::isNonPrivate(AnalysisResultPtr ar) {
  // To tell whether a property is declared as private in the context
  ClassScopePtr cls = getOriginalClass();
  if (!cls || !cls->getVariables()->hasNonStaticPrivate()) return true;
  if (m_property->getKindOf() != Expression::KindOfScalarExpression) {
    return false;
  }
  ScalarExpressionPtr name =
    dynamic_pointer_cast<ScalarExpression>(m_property);
  string propName = name->getLiteralString();
  if (propName.empty()) {
    return false;
  }
  Symbol *sym = cls->getVariables()->getSymbol(propName);
  if (!sym || sym->isStatic() || !sym->isPrivate()) return true;
  return false;
}

void ObjectPropertyExpression::setContext(Context context) {
  m_context |= context;
  switch (context) {
    case Expression::LValue:
      if (!hasContext(Expression::UnsetContext)) {
        m_object->setContext(Expression::LValue);
      }
      break;
    case Expression::DeepAssignmentLHS:
    case Expression::DeepOprLValue:
    case Expression::ExistContext:
    case Expression::UnsetContext:
    case Expression::DeepReference:
    case Expression::InvokeArgument:
      m_object->setContext(context);
      break;
    case Expression::RefValue:
    case Expression::RefParameter:
      m_object->setContext(DeepReference);
      break;
    default:
      break;
  }
  if (!m_valid &&
      (m_context & (LValue|RefValue)) &&
      !(m_context & AssignmentLHS)) {
    setLocalEffect(CreateEffect);
  }
  if (context == InvokeArgument) {
    setContext(NoLValueWrapper);
  }
}
void ObjectPropertyExpression::clearContext(Context context) {
  m_context &= ~context;
  switch (context) {
    case Expression::LValue:
    case Expression::DeepOprLValue:
    case Expression::DeepAssignmentLHS:
    case Expression::UnsetContext:
    case Expression::DeepReference:
    case Expression::InvokeArgument:
      m_object->clearContext(context);
      break;
    case Expression::RefValue:
    case Expression::RefParameter:
      m_object->clearContext(DeepReference);
      break;
    default:
      break;
  }

  if (!(m_context & (LValue|RefValue))) {
    clearLocalEffect(CreateEffect);
  }
  if (context == InvokeArgument) {
    clearContext(NoLValueWrapper);
  }
}

void ObjectPropertyExpression::analyzeProgram(AnalysisResultPtr ar) {
  m_object->analyzeProgram(ar);
  m_property->analyzeProgram(ar);
  if (ar->getPhase() == AnalysisResult::AnalyzeFinal) {
    if (m_valid && !hasLocalEffect(UnknownEffect) &&
        !m_object->isThis() &&
        (!m_object->is(KindOfSimpleVariable) ||
         !static_pointer_cast<SimpleVariable>(m_object)->isGuarded())) {
      setLocalEffect(DiagnosticEffect);
    }
  }
}

ConstructPtr ObjectPropertyExpression::getNthKid(int n) const {
  switch (n) {
    case 0:
      return m_object;
    case 1:
      return m_property;
    default:
      assert(false);
      break;
  }
  return ConstructPtr();
}

int ObjectPropertyExpression::getKidCount() const {
  return 2;
}

void ObjectPropertyExpression::setNthKid(int n, ConstructPtr cp) {
  switch (n) {
    case 0:
      m_object = dynamic_pointer_cast<Expression>(cp);
      break;
    case 1:
      m_property = dynamic_pointer_cast<Expression>(cp);
      break;
    default:
      assert(false);
      break;
  }
}

TypePtr ObjectPropertyExpression::inferTypes(AnalysisResultPtr ar,
                                             TypePtr type, bool coerce) {
  m_valid = false;

  ConstructPtr self = shared_from_this();
  TypePtr objectType = m_object->inferAndCheck(ar, Type::Some, false);

  if (!m_property->is(Expression::KindOfScalarExpression)) {
    m_property->inferAndCheck(ar, Type::String, false);
    // we also lost track of which class variable an expression is about, hence
    // any type inference could be wrong. Instead, we just force variants on
    // all class variables.
    if (m_context & (LValue | RefValue)) {
      ar->forceClassVariants(getOriginalClass(), false, true);
    }
    return Type::Variant; // we have to use a variant to hold dynamic value
  }

  ScalarExpressionPtr exp = dynamic_pointer_cast<ScalarExpression>(m_property);
  const string &name = exp->getLiteralString();
  if (name.empty()) {
    m_property->inferAndCheck(ar, Type::String, false);
    if (m_context & (LValue | RefValue)) {
      ar->forceClassVariants(getOriginalClass(), false, true);
    }
    return Type::Variant; // we have to use a variant to hold dynamic value
  }

  m_property->inferAndCheck(ar, Type::String, false);

  ClassScopePtr cls;
  if (objectType && !objectType->getName().empty()) {
    // what object-> has told us
    cls = ar->findExactClass(shared_from_this(), objectType->getName());
  } else {
    if ((m_context & LValue) && objectType &&
        !objectType->is(Type::KindOfObject) &&
        !objectType->is(Type::KindOfVariant) &&
        !objectType->is(Type::KindOfSome) &&
        !objectType->is(Type::KindOfAny)) {
      m_object->inferAndCheck(ar, Type::Object, true);
    }
  }

  if (!cls) {
    if (m_context & (LValue | RefValue | DeepReference | UnsetContext)) {
      ar->forceClassVariants(name, getOriginalClass(), false, true);
    }
    return Type::Variant;
  }

  // resolved to this class
  if (m_context & RefValue) {
    type = Type::Variant;
    coerce = true;
  }

  // use $this inside a static function
  if (m_object->isThis()) {
    FunctionScopePtr func = m_object->getOriginalFunction();
    if (!func || func->isStatic()) {
      if (getScope()->isFirstPass()) {
        Compiler::Error(Compiler::MissingObjectContext, self);
      }
      m_actualType = Type::Variant;
      return m_actualType;
    }
  }

  assert(cls);
  if (!m_propSym || cls != m_objectClass.lock()) {
    m_objectClass = cls;
    ClassScopePtr parent;
    m_propSym = cls->findProperty(parent, name, ar);
    if (m_propSym) {
      if (!parent) {
        parent = cls;
      }
      m_symOwner = parent;
      always_assert(m_propSym->isPresent());
      m_propSymValid =
        (!m_propSym->isPrivate() || getOriginalClass() == parent) &&
        !m_propSym->isStatic();

      if (m_propSymValid) {
        m_symOwner->addUse(getScope(),
                           BlockScope::GetNonStaticRefUseKind(
                             m_propSym->getHash()));
      }
    }
  }

  TypePtr ret;
  if (m_propSymValid && (!cls->derivesFromRedeclaring() ||
                         m_propSym->isPrivate())) {
    always_assert(m_symOwner);
    TypePtr t(m_propSym->getType());
    if (t && t->is(Type::KindOfVariant)) {
      // only check property if we could possibly do some work
      ret = t;
    } else {
      if (coerce && type->is(Type::KindOfAutoSequence) &&
          (!t || t->is(Type::KindOfVoid) ||
           t->is(Type::KindOfSome) || t->is(Type::KindOfArray))) {
        type = Type::Array;
      }
      assert(getScope()->is(BlockScope::FunctionScope));
      GET_LOCK(m_symOwner);
      ret = m_symOwner->checkProperty(getScope(), m_propSym, type, coerce, ar);
    }
    always_assert(m_object->getActualType() &&
           m_object->getActualType()->isSpecificObject());
    m_valid = true;
    return ret;
  } else {
    m_actualType = Type::Variant;
    return m_actualType;
  }
}

ExpressionPtr
ObjectPropertyExpression::postOptimize(AnalysisResultConstPtr ar) {
  bool changed = false;
  if (m_objectClass && hasLocalEffect(AccessorEffect)) {
    int prop = hasContext(AssignmentLHS) ?
      ClassScope::MayHaveUnknownPropSetter :
      hasContext(ExistContext) ?
        ClassScope::MayHaveUnknownPropTester :
        hasContext(UnsetContext) && hasContext(LValue) ?
          ClassScope::MayHavePropUnsetter :
          ClassScope::MayHaveUnknownPropGetter;
    if ((m_context & (AssignmentLHS|OprLValue)) ||
        !m_objectClass->implementsAccessor(prop)) {
      clearLocalEffect(AccessorEffect);
      changed = true;
    }
  }
  if (m_valid &&
      (hasLocalEffect(AccessorEffect) || hasLocalEffect(CreateEffect))) {
    clearLocalEffect(AccessorEffect);
    clearLocalEffect(CreateEffect);
    changed = true;
  }
  return changed ?
    dynamic_pointer_cast<Expression>(shared_from_this()) :
    ExpressionPtr();
}

///////////////////////////////////////////////////////////////////////////////

void ObjectPropertyExpression::outputCodeModel(CodeGenerator &cg) {
  cg.printObjectHeader("ObjectPropertyExpression", 3);
  cg.printPropertyHeader("object");
  m_object->outputCodeModel(cg);
  if (m_property->is(Expression::KindOfScalarExpression)) {
    cg.printPropertyHeader("propertyName");
  } else {
    cg.printPropertyHeader("propertyExpression");
  }
  m_property->outputCodeModel(cg);
  cg.printPropertyHeader("sourceLocation");
  cg.printLocation(this->getLocation());
  cg.printObjectFooter();
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void ObjectPropertyExpression::outputPHP(CodeGenerator &cg,
                                         AnalysisResultPtr ar) {
  m_object->outputPHP(cg, ar);
  cg_printf("->");
  if (m_property->getKindOf() == Expression::KindOfScalarExpression) {
    m_property->outputPHP(cg, ar);
  } else {
    cg_printf("{");
    m_property->outputPHP(cg, ar);
    cg_printf("}");
  }
}
