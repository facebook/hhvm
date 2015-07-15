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
 ExpressionPtr object, ExpressionPtr property, PropAccessType propAccessType)
  : Expression(
      EXPRESSION_CONSTRUCTOR_PARAMETER_VALUES(ObjectPropertyExpression)),
    LocalEffectsContainer(AccessorEffect),
    m_object(object), m_property(property), m_propSym(nullptr) {
  m_valid = false;
  m_propSymValid = false;
  m_object->setContext(Expression::ObjectContext);
  m_object->setContext(Expression::AccessContext);
  m_nullsafe = (propAccessType == PropAccessType::NullSafe);
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

bool ObjectPropertyExpression::isNonPrivate(AnalysisResultPtr ar) {
  // To tell whether a property is declared as private in the context
  ClassScopePtr cls = getClassScope();
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
  cg.printLocation(this);
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
