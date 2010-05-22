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

#include <runtime/eval/ast/list_assignment_expression.h>

namespace HPHP {
namespace Eval {
///////////////////////////////////////////////////////////////////////////////

ListElement::ListElement(CONSTRUCT_ARGS) : Construct(CONSTRUCT_PASS) {}

LvalListElement::LvalListElement(CONSTRUCT_ARGS, LvalExpressionPtr lval)
  : ListElement(CONSTRUCT_PASS), m_lval(lval) {}

void LvalListElement::set(VariableEnvironment &env, CVarRef val) const {
  if (m_lval) m_lval->set(env, val);
}

void LvalListElement::dump() const {
  if (m_lval) m_lval->dump();
}

SubListElement::SubListElement(CONSTRUCT_ARGS,
                               const std::vector<ListElementPtr> &elems)
  : ListElement(CONSTRUCT_PASS), m_elems(elems) {}

void SubListElement::set(VariableEnvironment &env, CVarRef val) const {
  for (int i = m_elems.size() - 1; i >= 0; i--) {
    const ListElementPtr &le = m_elems[i];
    if (le) {
      le->set(env, val.rvalAt(i));
    }
  }
}

void SubListElement::dump() const {
  Construct::dumpVector(m_elems, ", ");
}

ListAssignmentExpression::ListAssignmentExpression(EXPRESSION_ARGS,
                                                   ListElementPtr lhs,
                                                   ExpressionPtr rhs)
  : Expression(EXPRESSION_PASS), m_lhs(lhs), m_rhs(rhs) {}

Variant ListAssignmentExpression::eval(VariableEnvironment &env) const {
  Variant rhs(m_rhs->eval(env));
  m_lhs->set(env, rhs.is(KindOfArray) ? rhs : null_variant);
  return rhs;
}

void ListAssignmentExpression::dump() const {
  printf("list(");
  m_lhs->dump();
  printf(") = ");
  m_rhs->dump();
}

///////////////////////////////////////////////////////////////////////////////
}
}

