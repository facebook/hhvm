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

#include <cpp/eval/ast/variable_expression.h>
#include <cpp/eval/runtime/variable_environment.h>
#include <cpp/eval/ast/name.h>
#include <cpp/base/runtime_option.h>
#include <cpp/eval/strict_mode.h>

namespace HPHP {
namespace Eval {
///////////////////////////////////////////////////////////////////////////////

VariableExpression::VariableExpression(EXPRESSION_ARGS, NamePtr name,
                                       int idx /* = -1 */)
  : LvalExpression(EXPRESSION_PASS), m_name(name), m_idx(idx) {}

Variant VariableExpression::eval(VariableEnvironment &env) const {
  Variant &lhs = lval(env);
  if (RuntimeOption::EnableStrict) {
    /* note that 'if (!env.exists(str, name->hash()))' does not work
     * as undefined local variables are still in the (function) environment */
    if (!lhs.isInitialized()) {
      throw_strict(UseOfUndefinedVarException(this->loc()->toString()),
                   StrictMode::StrictBasic,
                   true);
    }
  }
  return lhs;
}

bool checkCompatibleAssignment(const Variant &left, const Variant &right) {
  bool ok = true;
  if (left.isNull()) {
    // everything is ok with NULL
  } else if (left.isBoolean()) {
    ok = right.isBoolean();
  } else if (left.isNumeric()) {
    ok = right.isNumeric();
  } else if (left.isString()) {
    ok = right.isString();
  } else if (left.isArray()) {
    ok = right.isArray();
  } else if (left.isObject()) {
    //TODO? check class of right derives from class of left ?
    ok = right.isObject();
  } else {
    ASSERT(false);
  }
  return ok;
}

Variant VariableExpression::set(VariableEnvironment &env, CVarRef val) const {
  Variant &lhs = lval(env);
  if (RuntimeOption::EnableStrict) {
    if (!checkCompatibleAssignment(lhs, val)) {
      throw_strict(TypeVariableChangeException(this->loc()->toString()),
                   StrictMode::StrictHardCore);
    }
  }
  return lhs = val;
}

Variant VariableExpression::setOp(VariableEnvironment &env, int op, CVarRef rhs)
  const {
  Variant &lhs = lval(env);
  if (RuntimeOption::EnableStrict) {
    if (!checkCompatibleAssignment(lhs, rhs)) {
      throw_strict(TypeVariableChangeException(this->loc()->toString()),
                   StrictMode::StrictHardCore);
    }
  }
  return LvalExpression::setOpVariant(lhs, op, rhs);
}

Variant &VariableExpression::lval(VariableEnvironment &env) const {
  if (m_idx != -1) {
    return env.getIdx(m_idx);
  }
  String name = m_name->get(env);
  return env.get(name, m_name->hash());
}

void VariableExpression::unset(VariableEnvironment &env) const {
  String name = m_name->get(env);
  env.unset(name, m_name->hash());
}

NamePtr VariableExpression::getName() const {
  return m_name;
}

void VariableExpression::dump() const {
  printf("$");
  m_name->dump();
}

///////////////////////////////////////////////////////////////////////////////
}
}

