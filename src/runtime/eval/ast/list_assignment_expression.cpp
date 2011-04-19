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

#include <runtime/eval/ast/list_assignment_expression.h>
#include <runtime/ext/ext_variable.h>
#include <runtime/eval/ast/variable_expression.h>
#include <runtime/eval/ast/temp_expression_list.h>
#include <runtime/eval/ast/array_element_expression.h>
#include <runtime/eval/ast/include_expression.h>
#include <runtime/eval/ast/unary_op_expression.h>
#include <runtime/eval/ast/object_property_expression.h>
#include <runtime/eval/ast/assignment_op_expression.h>
#include <runtime/eval/ast/assignment_ref_expression.h>
#include <runtime/eval/ast/inc_op_expression.h>
#include <runtime/eval/ast/function_call_expression.h>
#include <util/parser/hphp.tab.hpp>

namespace HPHP {
namespace Eval {
///////////////////////////////////////////////////////////////////////////////

ListElement::ListElement(CONSTRUCT_ARGS) : Construct(CONSTRUCT_PASS) {}

LvalListElement::LvalListElement(CONSTRUCT_ARGS, LvalExpressionPtr lval)
  : ListElement(CONSTRUCT_PASS), m_lval(lval) {}

void LvalListElement::collectOffsets(TempExpressionListPtr texp) {
  if (m_lval) {
    TempExpressionListPtr exp = m_lval->unsafe_cast<TempExpressionList>();
    if (exp) {
      texp->takeOffsets(exp);
    }
  }
}

void LvalListElement::set(VariableEnvironment &env, CVarRef val) const {
  if (m_lval) m_lval->set(env, val);
}

void LvalListElement::dump(std::ostream &out) const {
  if (m_lval) m_lval->dump(out);
}

SubListElement::SubListElement(CONSTRUCT_ARGS,
                               const std::vector<ListElementPtr> &elems)
  : ListElement(CONSTRUCT_PASS), m_elems(elems) {}

void SubListElement::collectOffsets(TempExpressionListPtr texp) {
  for (unsigned int i = 0; i < m_elems.size(); i++) {
    ListElementPtr &le = m_elems[i];
    if (le) {
      le->collectOffsets(texp);
    }
  }
}

void SubListElement::set(VariableEnvironment &env, CVarRef val) const {
  for (int i = m_elems.size() - 1; i >= 0; i--) {
    const ListElementPtr &le = m_elems[i];
    if (le) {
      le->set(env, val[i]);
    }
  }
}

void SubListElement::dump(std::ostream &out) const {
  out << "list(";
  Construct::dumpVector(out, m_elems);
  out << ")";
}

static bool IsAbnormal(ExpressionPtr rhs) {
  if (rhs->is<VariableExpression>() ||
      rhs->is<ArrayElementExpression>() ||
      rhs->is<ObjectPropertyExpression>() ||
      rhs->is<FunctionCallExpression>() ||
      rhs->is<IncludeExpression>() ||
      rhs->is<AssignmentOpExpression>() ||
      rhs->is<AssignmentRefExpression>()) {
    return false;
  }

  if (IncOpExpression *op = rhs->cast<IncOpExpression>()) {
    return !op->front();
  }

  if (UnaryOpExpression *op = rhs->cast<UnaryOpExpression>()) {
    if (op->getOp() == '@') return IsAbnormal(op->getExpression());
    if (op->getOp() == T_EVAL) return false;
    return true;
  }

  if (ListAssignmentExpression *la = rhs->cast<ListAssignmentExpression>()) {
    return IsAbnormal(la->getArray());
  }

  if (TempExpressionList *t = rhs->cast<TempExpressionList>()) {
    return IsAbnormal(t->getLastExp().get());
  }

  return true;
}

ListAssignmentExpression::ListAssignmentExpression(EXPRESSION_ARGS,
                                                   ListElementPtr lhs,
                                                   ExpressionPtr rhs)
  : Expression(EXPRESSION_PASS), m_lhs(lhs), m_rhs(rhs) {
  m_abnormal = IsAbnormal(rhs);
}

Variant ListAssignmentExpression::eval(VariableEnvironment &env) const {
  const VariableExpression *v = m_rhs->cast<VariableExpression>();
  if (v) {
    // Rhs has to be taken as lval if a variable in case there are references
    // to that variable on the lhs.
    CVarRef rhs(v->lval(env));
    Variant tmp(ref(rhs));
    m_lhs->set(env, tmp);
    return rhs;
  } else {
    Variant rhs(m_rhs->eval(env));
    m_lhs->set(env, !m_abnormal || rhs.is(KindOfArray) ? rhs : null_variant);
    return rhs;
  }
}

void ListAssignmentExpression::dump(std::ostream &out) const {
  m_lhs->dump(out);
  out << " = ";
  m_rhs->dump(out);
}

///////////////////////////////////////////////////////////////////////////////
}
}

