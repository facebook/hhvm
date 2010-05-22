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

#include <runtime/eval/ast/array_element_expression.h>
#include <runtime/base/builtin_functions.h>
#include <runtime/eval/ast/variable_expression.h>
#include <runtime/eval/ast/static_member_expression.h>
#include <runtime/eval/ast/name.h>
#include <runtime/eval/parser/hphp.tab.hpp>

namespace HPHP {
namespace Eval {
using namespace std;
///////////////////////////////////////////////////////////////////////////////

ArrayElementExpression::ArrayElementExpression(EXPRESSION_ARGS,
                                               LvalExpressionPtr arr,
                                               ExpressionPtr idx)
  : LvalExpression(EXPRESSION_PASS), m_arr(arr), m_idx(idx) {}

Variant ArrayElementExpression::eval(VariableEnvironment &env) const {
  if (!m_idx) {
    SET_LINE;
    throw InvalidOperandException("Cannot use [] in read context");
  }
  Variant arr(m_arr->eval(env));
  Variant idx(m_idx->eval(env));
  SET_LINE;
  return arr.rvalAt(idx, -1, true);
}

Variant ArrayElementExpression::evalExist(VariableEnvironment &env) const {
  if (!m_idx) {
    SET_LINE;
    throw InvalidOperandException("Cannot use [] in isset/empty context");
  }
  Variant arr(m_arr->evalExist(env));
  Variant idx(m_idx->eval(env));
  SET_LINE;
  return arr.rvalAt(idx);
}

bool ArrayElementExpression::exist(VariableEnvironment &env, int op) const {
  if (!m_idx) {
    SET_LINE;
    throw InvalidOperandException("Cannot use [] in read context");
  }
  Variant arr(m_arr->evalExist(env));
  Variant idx(m_idx->eval(env));
  SET_LINE;
  if (op == T_ISSET) {
    return HPHP::isset(arr, idx);
  }
  ASSERT(op == T_EMPTY);
  return HPHP::empty(arr, idx);
}

Variant &ArrayElementExpression::lval(VariableEnvironment &env) const {
  Variant &arr = m_arr->lval(env);
  if (m_idx) {
    Variant idx(m_idx->eval(env));
    SET_LINE;
    return arr.lvalAt(idx);
  } else {
    SET_LINE;
    return arr.lvalAt();
  }
}

bool ArrayElementExpression::weakLval(VariableEnvironment &env,
                                      Variant *&v) const {
  if (!m_idx) {
    SET_LINE;
    throw InvalidOperandException("Cannot unset array append");
  }
  Variant *arr;
  if (m_arr->weakLval(env, arr)) {
    Variant idx(m_idx->eval(env));
    if (!arr->is(KindOfArray)) {
      return false;
    }
    SET_LINE;
    if (arr->getArrayData()->exists(idx)) {
      v = &arr->lvalAt(idx);
      return true;
    }
  }
  return false;
}

Variant ArrayElementExpression::refval(VariableEnvironment &env,
                                       bool strict /* = true */) const {
  if (m_idx) {
    Variant arr(m_arr->refval(env, strict));
    Variant idx(m_idx->eval(env));
    SET_LINE;
    return ref(arr.refvalAt(idx));
  } else {
    SET_LINE;
    return ref(lval(env));
  }
}

Variant ArrayElementExpression::set(VariableEnvironment &env, CVarRef val)
  const {
  Variant &arr = m_arr->lval(env);
  if (m_idx) {
    Variant idx(m_idx->eval(env));
    SET_LINE;
    return arr.set(idx, val);
  } else {
    SET_LINE;
    return arr.append(val);
  }
}

void ArrayElementExpression::unset(VariableEnvironment &env) const {
  if (!m_idx) {
    throw InvalidOperandException("Cannot unset array append");
  }
  Variant *arr;
  if (m_arr->weakLval(env, arr)) {
    Variant idx(m_idx->eval(env));
    SET_LINE;
    arr->weakRemove(idx);
  }
}

void ArrayElementExpression::sinkStaticMember(Parser *parser,
                                              const NamePtr &className) {
  ArrayElementExpressionPtr arr = m_arr->cast<ArrayElementExpression>();
  if (arr) {
    arr->sinkStaticMember(parser, className);
    return;
  }

  VariableExpressionPtr var = m_arr->cast<VariableExpression>();
  if (var) {
    m_arr =
      StaticMemberExpressionPtr(new StaticMemberExpression(parser,
                                                           className,
                                                           var->getName()));
    return;
  }
  ASSERT(false);
}

void ArrayElementExpression::dump() const {
  m_arr->dump();
  printf("[");
  if (m_idx) {
    m_idx->dump();
  }
  printf("]");
}

///////////////////////////////////////////////////////////////////////////////
}
}

