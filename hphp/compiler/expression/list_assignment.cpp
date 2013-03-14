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

#include <compiler/expression/list_assignment.h>
#include <compiler/expression/assignment_expression.h>
#include <compiler/expression/expression_list.h>
#include <compiler/analysis/file_scope.h>
#include <compiler/analysis/function_scope.h>
#include <compiler/expression/array_element_expression.h>
#include <compiler/expression/object_property_expression.h>
#include <compiler/expression/unary_op_expression.h>
#include <compiler/expression/binary_op_expression.h>
#include <util/parser/hphp.tab.hpp>

using namespace HPHP;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

/*
  Determine whether the rhs behaves normall, or abnormally.

  1) If the expression is the silence operator, recurse on the inner expression.
  2) If the expression is a list assignment expression, recurse on the
     RHS of the expression.
  3) If the expression is one of the following, then E behaves normally:
  Simple/Dynamic variable (including $this and superglobals)
  Array element expression
  Property expression
  Static variable expression
  Function call expression
  Preinc/predec expression (but not postinc/postdec)
  Assignment expression
  Assignment op expression
  Binding assignment expression
  Include/require expression
  Eval expression
  Array expression
  Array cast expression
  4) For all other expressions, E behaves abnormally. This includes:
  All binary operator expressions
  All unary operator expressions except silence and preinc/predec
  Scalar expression of type null, bool, int, double, or string
  Qop expression (?:)
  Constant expression
  Class constant expression
  Isset or empty expression
  Exit expression
  Instanceof expression
*/
static ListAssignment::RHSKind GetRHSKind(ExpressionPtr rhs) {
  switch (rhs->getKindOf()) {
    case Expression::KindOfSimpleVariable:
    case Expression::KindOfDynamicVariable:
    case Expression::KindOfArrayElementExpression:
    case Expression::KindOfObjectPropertyExpression:
    case Expression::KindOfStaticMemberExpression:
    case Expression::KindOfSimpleFunctionCall:
    case Expression::KindOfDynamicFunctionCall:
    case Expression::KindOfObjectMethodExpression:
    case Expression::KindOfNewObjectExpression:
    case Expression::KindOfAssignmentExpression:
    case Expression::KindOfExpressionList:
    case Expression::KindOfIncludeExpression:
      return ListAssignment::Regular;

    case Expression::KindOfListAssignment:
      return GetRHSKind(static_pointer_cast<ListAssignment>(rhs)->getArray());

    case Expression::KindOfUnaryOpExpression: {
      UnaryOpExpressionPtr u(static_pointer_cast<UnaryOpExpression>(rhs));
      switch (u->getOp()) {
        case '@':
          return GetRHSKind(u->getExpression());
        case T_INC:
        case T_DEC:
          return u->getFront() ?
            ListAssignment::Regular : ListAssignment::Checked;
        case T_EVAL:
        case T_ARRAY:
        case T_ARRAY_CAST:
          return ListAssignment::Regular;
        default:
          return ListAssignment::Null;
      }
      break;
    }

    case Expression::KindOfBinaryOpExpression: {
      BinaryOpExpressionPtr b(static_pointer_cast<BinaryOpExpression>(rhs));
      return b->isAssignmentOp() || b->getOp() == '+' ?
        ListAssignment::Regular : ListAssignment::Null;
    }
    case Expression::KindOfQOpExpression:
      return ListAssignment::Checked;

    // invalid context
    case Expression::KindOfArrayPairExpression:
    case Expression::KindOfParameterExpression:
    case Expression::KindOfModifierExpression:
    case Expression::KindOfUserAttribute:
      always_assert(false);

    // non-arrays
    case Expression::KindOfScalarExpression:
    case Expression::KindOfConstantExpression:
    case Expression::KindOfClassConstantExpression:
    case Expression::KindOfEncapsListExpression:
    case Expression::KindOfClosureExpression:
      return ListAssignment::Null;
  }

  // unreachable for known expression kinds
  always_assert(false);
}

static bool AssignmentCouldSet(ExpressionListPtr vars, ExpressionPtr var) {
  for (int i = 0; i < vars->getCount(); i++) {
    ExpressionPtr v = (*vars)[i];
    if (!v) continue;
    if (v->is(Expression::KindOfSimpleVariable) &&
        v->canonCompare(var)) {
      return true;
    }
    if (v->is(Expression::KindOfDynamicVariable)) return true;
    if (v->is(Expression::KindOfListAssignment) &&
        AssignmentCouldSet(static_pointer_cast<ListAssignment>(v)->
                           getVariables(), var)) {
      return true;
    }
  }
  return false;
}

ListAssignment::ListAssignment
(EXPRESSION_CONSTRUCTOR_PARAMETERS,
 ExpressionListPtr variables, ExpressionPtr array)
  : Expression(EXPRESSION_CONSTRUCTOR_PARAMETER_VALUES(ListAssignment)),
    m_variables(variables), m_array(array), m_rhsKind(Regular) {
  setLValue();

  if (m_array) {
    m_rhsKind = GetRHSKind(m_array);
    if (m_array->is(KindOfSimpleVariable)) {
      if (AssignmentCouldSet(m_variables, m_array)) {
        m_array->setContext(LValue);
      }
    }
  }
}

ExpressionPtr ListAssignment::clone() {
  ListAssignmentPtr exp(new ListAssignment(*this));
  Expression::deepCopy(exp);
  exp->m_variables = Clone(m_variables);
  exp->m_array = Clone(m_array);
  return exp;
}

void ListAssignment::setLValue() {
  if (m_variables) {
    for (int i = 0; i < m_variables->getCount(); i++) {
      ExpressionPtr exp = (*m_variables)[i];
      if (exp) {
        if (exp->is(Expression::KindOfListAssignment)) {
          ListAssignmentPtr sublist =
            dynamic_pointer_cast<ListAssignment>(exp);
          sublist->setLValue();
        } else {
          // Magic contexts I took from assignment expression
          exp->setContext(Expression::DeepAssignmentLHS);
          exp->setContext(Expression::AssignmentLHS);
          exp->setContext(Expression::LValue);
          exp->setContext(Expression::NoLValueWrapper);
        }
      }
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// parser functions

///////////////////////////////////////////////////////////////////////////////
// static analysis functions

void ListAssignment::analyzeProgram(AnalysisResultPtr ar) {
  if (m_variables) m_variables->analyzeProgram(ar);
  if (m_array) m_array->analyzeProgram(ar);
  FunctionScopePtr func = getFunctionScope();
  if (func) func->disableInline();
  if (ar->getPhase() == AnalysisResult::AnalyzeFinal) {
    if (m_variables) {
      for (int i = 0; i < m_variables->getCount(); i++) {
        ExpressionPtr exp = (*m_variables)[i];
        if (exp) {
          if (!exp->is(Expression::KindOfListAssignment)) {
            CheckNeeded(exp, ExpressionPtr());
          }
        }
      }
    }
  }
}

ConstructPtr ListAssignment::getNthKid(int n) const {
  switch (n) {
    case 0:
      return m_variables;
    case 1:
      return m_array;
    default:
      assert(false);
      break;
  }
  return ConstructPtr();
}

int ListAssignment::getKidCount() const {
  return 2;
}

void ListAssignment::setNthKid(int n, ConstructPtr cp) {
  switch (n) {
    case 0:
      m_variables = boost::dynamic_pointer_cast<ExpressionList>(cp);
      break;
    case 1:
      m_array = boost::dynamic_pointer_cast<Expression>(cp);
      break;
    default:
      assert(false);
      break;
  }
}

TypePtr ListAssignment::inferTypes(AnalysisResultPtr ar, TypePtr type,
                                   bool coerce) {
  if (m_variables) {
    for (int i = m_variables->getCount(); i--; ) {
      ExpressionPtr exp = (*m_variables)[i];
      if (exp) {
        if (exp->is(Expression::KindOfListAssignment)) {
          exp->inferAndCheck(ar, Type::Any, false);
        } else {
          inferAssignmentTypes(ar, Type::Variant, true, exp);
        }
      }
    }
  }

  if (!m_array) return TypePtr();
  return m_array->inferAndCheck(ar, Type::Variant, false);
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void ListAssignment::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  cg_printf("list(");
  if (m_variables) m_variables->outputPHP(cg, ar);
  if (m_array) {
    cg_printf(") = ");
    m_array->outputPHP(cg, ar);
  } else {
    cg_printf(")");
  }
}
