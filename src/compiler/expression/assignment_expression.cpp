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

#include <compiler/expression/assignment_expression.h>
#include <compiler/expression/array_element_expression.h>
#include <compiler/expression/object_property_expression.h>
#include <compiler/analysis/code_error.h>
#include <compiler/expression/constant_expression.h>
#include <compiler/expression/simple_variable.h>
#include <compiler/analysis/block_scope.h>
#include <compiler/analysis/variable_table.h>
#include <compiler/analysis/constant_table.h>
#include <compiler/analysis/file_scope.h>
#include <compiler/expression/unary_op_expression.h>
#include <util/parser/hphp.tab.hpp>
#include <compiler/option.h>
#include <compiler/analysis/class_scope.h>
#include <compiler/analysis/function_scope.h>
#include <compiler/expression/scalar_expression.h>
#include <compiler/expression/expression_list.h>
#include <compiler/expression/simple_function_call.h>
#include <runtime/base/complex_types.h>
#include <runtime/base/builtin_functions.h>

using namespace HPHP;
using namespace std;
using namespace boost;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

AssignmentExpression::AssignmentExpression
(EXPRESSION_CONSTRUCTOR_PARAMETERS,
 ExpressionPtr variable, ExpressionPtr value, bool ref)
  : Expression(EXPRESSION_CONSTRUCTOR_PARAMETER_VALUES),
    m_variable(variable), m_value(value), m_ref(ref) {
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
    ConstantExpressionPtr exp =
      dynamic_pointer_cast<ConstantExpression>(m_variable);
    scope->getConstants()->add(exp->getName(), type, m_value, ar, m_variable);
  } else if (m_variable->is(Expression::KindOfSimpleVariable)) {
    SimpleVariablePtr var = dynamic_pointer_cast<SimpleVariable>(m_variable);
    scope->getVariables()->add(var->getName(), type, true, ar,
                               shared_from_this(), scope->getModifiers());
    var->clearContext(Declaration); // to avoid wrong CodeError
  } else {
    ASSERT(false); // parse phase shouldn't handle anything else
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
    if (m_variable->is(Expression::KindOfConstantExpression)) {
      ConstantExpressionPtr exp =
        dynamic_pointer_cast<ConstantExpression>(m_variable);
      if (!m_value->isScalar()) {
        getScope()->getConstants()->setDynamic(ar, exp->getName());
      }
    }
  } else if (ar->getPhase() == AnalysisResult::AnalyzeFinal) {
    CheckNeeded(m_variable, m_value);
  }
}

ConstructPtr AssignmentExpression::getNthKid(int n) const {
  switch (n) {
    case 0:
      return m_variable;
    case 1:
      return m_value;
    default:
      ASSERT(false);
      break;
  }
  return ConstructPtr();
}

int AssignmentExpression::getKidCount() const {
  return 2;
}

void AssignmentExpression::setNthKid(int n, ConstructPtr cp) {
  switch (n) {
    case 0:
      m_variable = boost::dynamic_pointer_cast<Expression>(cp);
      break;
    case 1:
      m_value = boost::dynamic_pointer_cast<Expression>(cp);
      break;
    default:
      ASSERT(false);
      break;
  }
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
                              KindOfExpressionList,
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
                getScope(), getLocation(), KindOfAssignmentExpression,
                m_variable->replaceValue(Clone(ae->getVariable())),
                makeScalarExpression(ar, v), false));
            if (!isUnused()) {
              ExpressionListPtr el(
                new ExpressionList(
                  getScope(), getLocation(), KindOfExpressionList,
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

static void wrapValue(CodeGenerator &cg, AnalysisResultPtr ar,
                      ExpressionPtr exp, bool ref, bool array, bool varnr) {
  bool close = false;
  if (ref) {
    cg_printf("ref(");
    close = true;
  } else if (array && !exp->hasCPPTemp() &&
             !exp->isTemporary() && !exp->isScalar() &&
             exp->getActualType() && !exp->getActualType()->isPrimitive() &&
             exp->getActualType()->getKindOf() != Type::KindOfString) {
    cg_printf("wrap_variant(");
    close = true;
  } else if (varnr && exp->getCPPType()->isExactType()) {
    cg_printf("VarNR(");
    close = true;
  }
  exp->outputCPP(cg, ar);
  if (close) cg_printf(")");
}

void AssignmentExpression::preOutputStash(CodeGenerator &cg,
                                          AnalysisResultPtr ar, int state) {
  if (hasCPPTemp()) return;
  if (m_value->hasCPPTemp() && Type::SameType(getType(), m_value->getType())) {
    setUnused(true);
    outputCPP(cg, ar);
    cg_printf(";\n");
    m_cppTemp = m_value->cppTemp();
    return;
  }
  return Expression::preOutputStash(cg, ar, state);
}

bool AssignmentExpression::preOutputCPP(CodeGenerator &cg, AnalysisResultPtr ar,
                                        int state) {
  if (m_variable->is(Expression::KindOfArrayElementExpression)) {
    ExpressionPtr exp = m_value;
    ExpressionPtr vv(
      static_pointer_cast<ArrayElementExpression>(m_variable)->getVariable());
    if ((vv->is(KindOfArrayElementExpression) ||
         vv->is(KindOfObjectPropertyExpression)) &&
        (vv->getContainedEffects() && (CreateEffect|AccessorEffect))) {
      /*
        We are in a case such as
          $a->b['c'] = ...;
          $a['b']['c'] = ...;
        Where evaluating m_variable may modify $a. Unless we can prove that
        the rhs is not referring to the same thing as $a, we must generate
        a temporary for it (note that we could do better with the following
        checks).
      */
      if (!(m_ref && exp->isRefable()) &&
          !exp->isTemporary() && !exp->isScalar() &&
          exp->getActualType() && !exp->getActualType()->isPrimitive() &&
          exp->getActualType()->getKindOf() != Type::KindOfString) {
        state |= Expression::StashAll;
      }
    }
  }

  return Expression::preOutputCPP(cg, ar, state);
}

bool AssignmentExpression::SpecialAssignment(CodeGenerator &cg,
                                             AnalysisResultPtr ar,
                                             ExpressionPtr lval,
                                             ExpressionPtr rval,
                                             const char *rvalStr, bool ref) {
  if (lval->is(KindOfArrayElementExpression)) {
    ArrayElementExpressionPtr exp =
      dynamic_pointer_cast<ArrayElementExpression>(lval);
    if (!exp->isSuperGlobal() && !exp->isDynamicGlobal()) {
      exp->getVariable()->outputCPP(cg, ar);
      if (exp->getOffset()) {
        cg_printf(".set(");
        exp->getOffset()->outputCPP(cg, ar);
        cg_printf(", (");
      } else {
        cg_printf(".append((");
      }
      if (rval) {
        wrapValue(cg, ar, rval, ref,
                  (exp->getVariable()->is(KindOfArrayElementExpression) ||
                   exp->getVariable()->is(KindOfObjectPropertyExpression)) &&
                  (exp->getVariable()->getContainedEffects() &&
                   (CreateEffect|AccessorEffect)), true);
      } else {
        cg_printf(ref ? "ref(%s)" : "%s", rvalStr);
      }
      cg_printf(")");
      ExpressionPtr off = exp->getOffset();
      if (off) {
        ScalarExpressionPtr sc =
          dynamic_pointer_cast<ScalarExpression>(off);
        if (sc) {
          if (sc->isLiteralString()) {
            String s(sc->getLiteralString());
            int64 n;
            if (!s.get()->isStrictlyInteger(n)) {
              cg_printf(", true"); // skip toKey() at run time
            }
          }
        }
      }
      cg_printf(")");
      return true;
    }
  } else if (lval->is(KindOfObjectPropertyExpression)) {
    ObjectPropertyExpressionPtr var(
      dynamic_pointer_cast<ObjectPropertyExpression>(lval));
    if (!var->isValid()) {
      bool nonPrivate = var->isNonPrivate(ar);
      var->outputCPPObject(cg, ar);
      if (nonPrivate) {
        cg_printf("o_setPublic(");
      } else {
        cg_printf("o_set(");
      }
      var->outputCPPProperty(cg, ar);
      cg_printf(", %s", ref ? "ref(" : "");
      if (rval) {
        rval->outputCPP(cg, ar);
      } else {
        cg_printf(ref ? "ref(%s)" : "%s", rvalStr);
      }
      if (nonPrivate) {
        cg_printf("%s)", ref ? ")" : "");
      }
      else {
        cg_printf("%s%s)",
                  ref ? ")" : "",
                  lval->originalClassName(cg, true).c_str());
      }
      return true;
    }
  }
  return false;
}

void AssignmentExpression::outputCPPImpl(CodeGenerator &cg,
                                         AnalysisResultPtr ar) {
  bool ref = (m_ref && m_value->isRefable());

  bool setNull = false;

  if (SpecialAssignment(cg, ar, m_variable, m_value, NULL, ref)) {
    return;
  }

  if (m_variable->is(Expression::KindOfSimpleVariable) &&
      m_value->is(Expression::KindOfConstantExpression)) {
    ConstantExpressionPtr exp =
      dynamic_pointer_cast<ConstantExpression>(m_value);
    if (exp->isNull()) setNull = true;
  }

  bool wrapped = true;
  if (setNull) {
    cg_printf("setNull(");
    m_variable->outputCPP(cg, ar);
  } else {
    if (!m_variable->getCPPType()->isExactType() &&
        !(m_value->hasCPPTemp() ?
          m_value->getType() : m_value->getCPPType())->isExactType()) {
      m_variable->outputCPP(cg, ar);
      cg_printf(".assign%s(", ref ? "Ref" : "Val");
      wrapped = true;
      ref = false;
    } else {
      if ((wrapped = !isUnused())) {
        cg_printf("(");
      }
      m_variable->outputCPP(cg, ar);
      cg_printf(" = ");
    }

    wrapValue(cg, ar, m_value, ref, false, false);
  }
  if (wrapped) {
    cg_printf(")");
  }
}
