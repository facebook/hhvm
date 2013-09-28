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

#include "hphp/compiler/expression/assignment_expression.h"
#include "hphp/compiler/expression/array_element_expression.h"
#include "hphp/compiler/expression/object_property_expression.h"
#include "hphp/compiler/analysis/code_error.h"
#include "hphp/compiler/expression/constant_expression.h"
#include "hphp/compiler/expression/simple_variable.h"
#include "hphp/compiler/analysis/block_scope.h"
#include "hphp/compiler/analysis/variable_table.h"
#include "hphp/compiler/analysis/constant_table.h"
#include "hphp/compiler/analysis/file_scope.h"
#include "hphp/compiler/expression/unary_op_expression.h"
#include "hphp/parser/hphp.tab.hpp"
#include "hphp/compiler/option.h"
#include "hphp/compiler/analysis/class_scope.h"
#include "hphp/compiler/analysis/function_scope.h"
#include "hphp/compiler/expression/scalar_expression.h"
#include "hphp/compiler/expression/expression_list.h"
#include "hphp/compiler/expression/simple_function_call.h"
#include "hphp/runtime/base/complex-types.h"
#include "hphp/runtime/base/builtin-functions.h"

using namespace HPHP;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

AssignmentExpression::AssignmentExpression
(EXPRESSION_CONSTRUCTOR_PARAMETERS,
 ExpressionPtr variable, ExpressionPtr value, bool ref,
 bool rhsFirst /* = false */)
  : Expression(EXPRESSION_CONSTRUCTOR_PARAMETER_VALUES(AssignmentExpression)),
    m_variable(variable), m_value(value), m_ref(ref), m_rhsFirst(rhsFirst) {
  assert(!m_ref || !m_rhsFirst);
  m_variable->setContext(Expression::DeepAssignmentLHS);
  m_variable->setContext(Expression::AssignmentLHS);
  m_variable->setContext(Expression::LValue);
  m_variable->setContext(Expression::NoLValueWrapper);
  m_value->setContext(Expression::AssignmentRHS);
  if (ref) {
    m_variable->setContext(Expression::RefAssignmentLHS);
    m_value->setContext(Expression::RefValue);

    // we have &new special case that's handled in this class
    m_value->setContext(Expression::NoRefWrapper);
  }
}

ExpressionPtr AssignmentExpression::clone() {
  AssignmentExpressionPtr exp(new AssignmentExpression(*this));
  Expression::deepCopy(exp);
  exp->m_variable = Clone(m_variable);
  exp->m_value = Clone(m_value);
  return exp;
}

///////////////////////////////////////////////////////////////////////////////
// parser functions

void AssignmentExpression::onParseRecur(AnalysisResultConstPtr ar,
                                        ClassScopePtr scope) {
  // This is that much we can do during parse phase.
  TypePtr type;
  if (m_value->is(Expression::KindOfScalarExpression)) {
    type = static_pointer_cast<ScalarExpression>(m_value)->inferenceImpl(
      ar, Type::Some, false);
  } else if (m_value->is(Expression::KindOfUnaryOpExpression)) {
    UnaryOpExpressionPtr uexp =
      dynamic_pointer_cast<UnaryOpExpression>(m_value);
    if (uexp->getOp() == T_ARRAY) {
      type = Type::Array;
    }
  }
  if (!type) type = Type::Some;

  if (m_variable->is(Expression::KindOfConstantExpression)) {
    // ...as in ClassConstant statement
    // We are handling this one here, not in ClassConstant, purely because
    // we need "value" to store in constant table.
    if (type->is(Type::KindOfArray)) {
      parseTimeFatal(Compiler::NoError,
                     "Arrays are not allowed in class constants");
    }
    ConstantExpressionPtr exp =
      dynamic_pointer_cast<ConstantExpression>(m_variable);
    scope->getConstants()->add(exp->getName(), type, m_value, ar, m_variable);
  } else if (m_variable->is(Expression::KindOfSimpleVariable)) {
    SimpleVariablePtr var = dynamic_pointer_cast<SimpleVariable>(m_variable);
    scope->getVariables()->add(var->getName(), type, true, ar,
                               shared_from_this(), scope->getModifiers());
    var->clearContext(Declaration); // to avoid wrong CodeError
  } else {
    assert(false); // parse phase shouldn't handle anything else
  }
}

///////////////////////////////////////////////////////////////////////////////
// static analysis functions

int AssignmentExpression::getLocalEffects() const {
  return AssignEffect;
}

void AssignmentExpression::analyzeProgram(AnalysisResultPtr ar) {
  m_variable->analyzeProgram(ar);
  m_value->analyzeProgram(ar);
  if (ar->getPhase() == AnalysisResult::AnalyzeAll) {
    if (m_ref && m_variable->is(Expression::KindOfSimpleVariable)) {
      SimpleVariablePtr var =
        dynamic_pointer_cast<SimpleVariable>(m_variable);
      const std::string &name = var->getName();
      VariableTablePtr variables = getScope()->getVariables();
      variables->addUsed(name);
    }
  } else if (ar->getPhase() == AnalysisResult::AnalyzeFinal) {
    if (m_variable->is(Expression::KindOfConstantExpression)) {
      ConstantExpressionPtr exp =
        dynamic_pointer_cast<ConstantExpression>(m_variable);
      if (!m_value->isScalar()) {
        getScope()->getConstants()->setDynamic(ar, exp->getName(), false);
      }
    } else {
      CheckNeeded(m_variable, m_value);
    }
  }
}

ConstructPtr AssignmentExpression::getNthKid(int n) const {
  switch (m_rhsFirst ? 1 - n : n) {
    case 0:
      return m_variable;
    case 1:
      return m_value;
    default:
      assert(false);
      break;
  }
  return ConstructPtr();
}

int AssignmentExpression::getKidCount() const {
  return 2;
}

void AssignmentExpression::setNthKid(int n, ConstructPtr cp) {
  switch (m_rhsFirst ? 1 - n : n) {
    case 0:
      m_variable = dynamic_pointer_cast<Expression>(cp);
      break;
    case 1:
      m_value = dynamic_pointer_cast<Expression>(cp);
      break;
    default:
      assert(false);
      break;
  }
}

bool AssignmentExpression::isSimpleGlobalAssign(StringData **name,
                                                TypedValue *tv) const {
  if (!m_variable->is(KindOfArrayElementExpression)) return false;
  ArrayElementExpressionPtr ae(
    static_pointer_cast<ArrayElementExpression>(m_variable));
  if (!ae->isSuperGlobal() || ae->isDynamicGlobal()) return false;
  Variant v;
  if (!m_value->getScalarValue(v) || v.is(KindOfArray)) return false;
  if (name) {
    *name = makeStaticString(ae->getGlobalName());
  }
  if (tv) {
    if (v.isString()) {
      v = makeStaticString(v.toCStrRef().get());
    }
    *tv = *v.asTypedValue();
  }
  return true;
}

ExpressionPtr AssignmentExpression::optimize(AnalysisResultConstPtr ar) {
  if (m_variable->is(Expression::KindOfSimpleVariable)) {
    SimpleVariablePtr var =
      dynamic_pointer_cast<SimpleVariable>(m_variable);
    if (var->checkUnused() &&
        !CheckNeeded(var, m_value)) {
      if (m_value->getContainedEffects() != getContainedEffects()) {
        recomputeEffects();
      }
      return replaceValue(m_value);
    }
  }
  return ExpressionPtr();
}

ExpressionPtr AssignmentExpression::preOptimize(AnalysisResultConstPtr ar) {
  if (Option::EliminateDeadCode &&
      ar->getPhase() >= AnalysisResult::FirstPreOptimize) {
    // otherwise used & needed flags may not be up to date yet
    ExpressionPtr rep = optimize(ar);
    if (rep) return rep;
  }
  if (m_variable->getContainedEffects() & ~(CreateEffect|AccessorEffect)) {
    return ExpressionPtr();
  }
  ExpressionPtr val = m_value;
  while (val) {
    if (val->is(KindOfExpressionList)) {
      ExpressionListPtr el(static_pointer_cast<ExpressionList>(val));
      val = el->listValue();
      continue;
    }
    if (val->is(KindOfAssignmentExpression)) {
      val = static_pointer_cast<AssignmentExpression>(val)->m_value;
      continue;
    }
    break;
  }
  if (val && val->isScalar()) {
    if (val != m_value) {
      ExpressionListPtr rep(new ExpressionList(
                              getScope(), getLocation(),
                              ExpressionList::ListKindWrapped));
      rep->addElement(m_value);
      m_value = val->clone();
      rep->addElement(static_pointer_cast<Expression>(shared_from_this()));
      return replaceValue(rep);
    }
    if (!m_ref && m_variable->is(KindOfArrayElementExpression)) {
      ArrayElementExpressionPtr ae(
        static_pointer_cast<ArrayElementExpression>(m_variable));
      ExpressionPtr avar(ae->getVariable());
      ExpressionPtr aoff(ae->getOffset());
      if (!aoff || aoff->isScalar()) {
        avar = avar->getCanonLVal();
        while (avar) {
          if (avar->isScalar()) {
            Variant v,o,r;
            if (!avar->getScalarValue(v)) break;
            if (!val->getScalarValue(r)) break;
            try {
              g_context->setThrowAllErrors(true);
              if (aoff) {
                if (!aoff->getScalarValue(o)) break;
                if (v.isString()) {
                  if (!o.isInteger() ||
                      o.toInt64Val() < 0 ||
                      o.toInt64Val() >= v.toCStrRef().size()) {
                    // warnings should be raised...
                    break;
                  }
                }
                v.set(o, r);
              } else {
                v.append(r);
              }
              g_context->setThrowAllErrors(false);
            } catch (...) {
              break;
            }
            ExpressionPtr rep(
              new AssignmentExpression(
                getScope(), getLocation(),
                m_variable->replaceValue(Clone(ae->getVariable())),
                makeScalarExpression(ar, v), false));
            if (!isUnused()) {
              ExpressionListPtr el(
                new ExpressionList(
                  getScope(), getLocation(),
                  ExpressionList::ListKindWrapped));
              el->addElement(rep);
              el->addElement(val);
              rep = el;
            }
            return replaceValue(rep);
          }
          avar = avar->getCanonPtr();
        }
        g_context->setThrowAllErrors(false);
      }
    }
  }
  return ExpressionPtr();
}

ExpressionPtr AssignmentExpression::postOptimize(AnalysisResultConstPtr ar) {
  return optimize(ar);
}

TypePtr AssignmentExpression::inferTypes(AnalysisResultPtr ar, TypePtr type,
                                         bool coerce) {

  return inferAssignmentTypes(ar, type, coerce, m_variable, m_value);
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void AssignmentExpression::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  m_variable->outputPHP(cg, ar);
  cg_printf(" = ");
  if (m_ref) cg_printf("&");
  m_value->outputPHP(cg, ar);
}
