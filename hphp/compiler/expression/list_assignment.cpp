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

#include "hphp/compiler/expression/list_assignment.h"
#include "hphp/compiler/expression/assignment_expression.h"
#include "hphp/compiler/expression/expression_list.h"
#include "hphp/compiler/analysis/file_scope.h"
#include "hphp/compiler/analysis/function_scope.h"
#include "hphp/compiler/expression/array_element_expression.h"
#include "hphp/compiler/expression/object_property_expression.h"
#include "hphp/compiler/expression/unary_op_expression.h"
#include "hphp/compiler/expression/binary_op_expression.h"
#include "hphp/parser/hphp.tab.hpp"

using namespace HPHP;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

/*
  Determine whether the rhs behaves normally, or abnormally.

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
  case Construct::KindOfSimpleVariable:
  case Construct::KindOfDynamicVariable:
  case Construct::KindOfArrayElementExpression:
  case Construct::KindOfObjectPropertyExpression:
  case Construct::KindOfStaticMemberExpression:
  case Construct::KindOfSimpleFunctionCall:
  case Construct::KindOfDynamicFunctionCall:
  case Construct::KindOfObjectMethodExpression:
  case Construct::KindOfNewObjectExpression:
  case Construct::KindOfAssignmentExpression:
  case Construct::KindOfExpressionList:
  case Construct::KindOfIncludeExpression:
  case Construct::KindOfYieldExpression:
  case Construct::KindOfAwaitExpression:
  case Construct::KindOfQueryExpression:
    return ListAssignment::Regular;

  case Construct::KindOfListAssignment:
    return GetRHSKind(static_pointer_cast<ListAssignment>(rhs)->getArray());

  case Construct::KindOfUnaryOpExpression: {
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

  case Construct::KindOfBinaryOpExpression: {
    BinaryOpExpressionPtr b(static_pointer_cast<BinaryOpExpression>(rhs));
    if (b->isAssignmentOp() ||
        b->getOp() == '+' ||
        b->getOp() == T_COLLECTION) {
      return ListAssignment::Regular;
    }
    return ListAssignment::Null;
  }
  case Construct::KindOfQOpExpression:
    return ListAssignment::Checked;

  // invalid context
  case Construct::KindOfExpression:
  case Construct::KindOfArrayPairExpression:
  case Construct::KindOfParameterExpression:
  case Construct::KindOfModifierExpression:
  case Construct::KindOfUserAttribute:
  case Construct::KindOfFromClause:
  case Construct::KindOfLetClause:
  case Construct::KindOfWhereClause:
  case Construct::KindOfSelectClause:
  case Construct::KindOfIntoClause:
  case Construct::KindOfJoinClause:
  case Construct::KindOfGroupClause:
  case Construct::KindOfOrderbyClause:
  case Construct::KindOfOrdering:
    always_assert(false);

  // non-arrays
  case Construct::KindOfScalarExpression:
  case Construct::KindOfConstantExpression:
  case Construct::KindOfClassConstantExpression:
  case Construct::KindOfEncapsListExpression:
  case Construct::KindOfClosureExpression:
    return ListAssignment::Null;

#define STATEMENT_CASE(x) case Construct::KindOf##x:
  DECLARE_STATEMENT_TYPES(STATEMENT_CASE) {
    always_assert(false);
  }
#undef STATEMENT_CASE
  }

  // unreachable for known expression kinds
  always_assert(false);
}

ListAssignment::ListAssignment
(EXPRESSION_CONSTRUCTOR_PARAMETERS,
 ExpressionListPtr variables, ExpressionPtr array, bool rhsFirst /* = false */)
  : Expression(EXPRESSION_CONSTRUCTOR_PARAMETER_VALUES(ListAssignment)),
    m_variables(variables), m_array(array), m_rhsKind(Regular),
    m_rhsFirst(rhsFirst) {
  setLValue();

  if (m_array) {
    m_rhsKind = GetRHSKind(m_array);
    if (m_array->is(KindOfSimpleVariable)) {
      m_array->setContext(LValue);
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
        if (exp->is(Construct::KindOfListAssignment)) {
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
  if (ar->getPhase() == AnalysisResult::AnalyzeFinal) {
    if (m_variables) {
      for (int i = 0; i < m_variables->getCount(); i++) {
        ExpressionPtr exp = (*m_variables)[i];
        if (exp) {
          if (!exp->is(Construct::KindOfListAssignment)) {
            CheckNeeded(exp, ExpressionPtr());
          }
        }
      }
    }
  }
}

ConstructPtr ListAssignment::getNthKid(int n) const {
  switch (m_rhsFirst ? 1 - n : n) {
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
  switch (m_rhsFirst ? 1 - n : n) {
    case 0:
      m_variables = dynamic_pointer_cast<ExpressionList>(cp);
      break;
    case 1:
      m_array = dynamic_pointer_cast<Expression>(cp);
      break;
    default:
      assert(false);
      break;
  }
}

///////////////////////////////////////////////////////////////////////////////

void ListAssignment::outputCodeModel(CodeGenerator &cg) {
  auto numProps = m_array != nullptr ? 3 : 2;
  cg.printObjectHeader("ListAssignmentExpression", numProps);
  cg.printPropertyHeader("variables");
  cg.printExpressionVector(m_variables);
  if (m_array != nullptr) {
    cg.printPropertyHeader("expression");
    m_array->outputCodeModel(cg);
  }
  cg.printPropertyHeader("sourceLocation");
  cg.printLocation(this);
  cg.printObjectFooter();
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
