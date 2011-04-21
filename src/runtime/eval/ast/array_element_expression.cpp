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

#include <runtime/eval/ast/array_element_expression.h>
#include <runtime/base/builtin_functions.h>
#include <runtime/eval/ast/variable_expression.h>
#include <runtime/eval/ast/static_member_expression.h>
#include <runtime/eval/ast/name.h>
#include <util/parser/hphp.tab.hpp>

namespace HPHP {
namespace Eval {
using namespace std;
///////////////////////////////////////////////////////////////////////////////

ArrayElementExpression::ArrayElementExpression(EXPRESSION_ARGS,
                                               ExpressionPtr arr,
                                               ExpressionPtr idx)
  : LvalExpression(EXPRESSION_PASS), m_arr(arr), m_idx(idx) {
  m_reverseOrder = m_idx && m_arr->is<VariableExpression>();
}

Variant ArrayElementExpression::eval(VariableEnvironment &env) const {
  if (!m_idx) {
    SET_LINE;
    throw InvalidOperandException("Cannot use [] in read context");
  }
  Variant arr, idx;
  if (m_reverseOrder) {
    idx = m_idx->eval(env);
    arr = m_arr->eval(env);
  } else {
    arr = m_arr->eval(env);
    idx = m_idx->eval(env);
  }
  SET_LINE;
  return arr.rvalAt(idx, AccessFlags::Error);
}

Variant ArrayElementExpression::evalExist(VariableEnvironment &env) const {
  if (!m_idx) {
    SET_LINE;
    throw InvalidOperandException("Cannot use [] in isset/empty context");
  }
  Variant arr, idx;
  if (m_reverseOrder) {
    idx = m_idx->eval(env);
    arr = m_arr->evalExist(env);
  } else {
    arr = m_arr->evalExist(env);
    idx = m_idx->eval(env);
  }
  SET_LINE;
  return arr.rvalAt(idx);
}

bool ArrayElementExpression::exist(VariableEnvironment &env, int op) const {
  if (!m_idx) {
    SET_LINE;
    throw InvalidOperandException("Cannot use [] in read context");
  }
  Variant arr, idx;
  if (m_reverseOrder) {
    idx = m_idx->eval(env);
    arr = m_arr->evalExist(env);
  } else {
    arr = m_arr->evalExist(env);
    idx = m_idx->eval(env);
  }
  SET_LINE;
  if (op == T_ISSET) {
    return HPHP::isset(arr, idx);
  }
  ASSERT(op == T_EMPTY);
  return HPHP::empty(arr, idx);
}

Variant &ArrayElementExpression::lval(VariableEnvironment &env) const {
  const LvalExpression *larr = m_arr->toLval();
  ASSERT(larr);
  if (!larr) {
    throw InvalidOperandException("Cannot take l-value with function return");
  }
  Variant &arr = larr->lval(env);
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
  const LvalExpression *larr = m_arr->toLval();
  if (!larr) {
    throw InvalidOperandException("Cannot take l-value with function return");
  }
  Variant *arr;
  bool ok = larr->weakLval(env, arr);
  Variant idx(m_idx->eval(env));
  if (!ok || !arr->is(KindOfArray)) {
    return false;
  }
  SET_LINE;
  if (arr->toArray().exists(idx)) {
    v = &arr->lvalAt(idx);
    return true;
  }
  return false;
}

Variant ArrayElementExpression::refval(VariableEnvironment &env,
                                       int strict /* = 2 */) const {
  if (m_idx) {
    Variant arr(m_arr->refval(env, strict));
    Variant idx(m_idx->eval(env));
    SET_LINE;
    return strongBind(arr.refvalAt(idx));
  } else {
    SET_LINE;
    return strongBind(lval(env));
  }
}

Variant ArrayElementExpression::set(VariableEnvironment &env, CVarRef val)
  const {
  const LvalExpression *larr = m_arr->toLval();
  if (!larr) {
    throw InvalidOperandException("Cannot take l-value with function return");
  }
  Variant &arr = larr->lval(env);
  if (m_idx) {
    Variant idx(m_idx->eval(env));
    SET_LINE;
    return arr.set(idx, val);
  } else {
    SET_LINE;
    return arr.append(val);
  }
}

Variant ArrayElementExpression::setRef(VariableEnvironment &env, CVarRef val)
  const {
  const LvalExpression *larr = m_arr->toLval();
  if (!larr) {
    throw InvalidOperandException("Cannot take l-value with function return");
  }
  Variant &arr = larr->lval(env);
  if (m_idx) {
    Variant idx(m_idx->eval(env));
    SET_LINE;
    return arr.setRef(idx, val);
  } else {
    SET_LINE;
    return arr.appendRef(val);
  }
}

void ArrayElementExpression::unset(VariableEnvironment &env) const {
  if (!m_idx) {
    throw InvalidOperandException("Cannot unset array append");
  }
  const LvalExpression *larr = m_arr->toLval();
  if (!larr) {
    throw InvalidOperandException("Cannot take l-value with function return");
  }
  Variant *arr;
  if (larr->weakLval(env, arr)) {
    Variant idx(m_idx->eval(env));
    SET_LINE_VOID;
    arr->weakRemove(idx);
  } else {
    m_idx->eval(env);
  }
}

void ArrayElementExpression::sinkStaticMember(Parser *parser,
                                              const NamePtr &className) {
  ArrayElementExpressionPtr arr = m_arr->unsafe_cast<ArrayElementExpression>();
  if (arr) {
    arr->sinkStaticMember(parser, className);
    return;
  }

  VariableExpressionPtr var = m_arr->unsafe_cast<VariableExpression>();
  if (var) {
    m_arr =
      StaticMemberExpressionPtr(new StaticMemberExpression(parser,
                                                           className,
                                                           var->getName()));
    return;
  }
  ASSERT(false);
}

void ArrayElementExpression::dump(std::ostream &out) const {
  m_arr->dump(out);
  out << "[";
  if (m_idx) {
    m_idx->dump(out);
  }
  out << "]";
}

///////////////////////////////////////////////////////////////////////////////
}
}

