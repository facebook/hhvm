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

#include <runtime/eval/ast/variable_expression.h>
#include <runtime/eval/runtime/variable_environment.h>
#include <runtime/eval/ast/name.h>
#include <runtime/base/runtime_option.h>
#include <runtime/eval/strict_mode.h>

namespace HPHP {
namespace Eval {
///////////////////////////////////////////////////////////////////////////////

VariableExpression::VariableExpression(EXPRESSION_ARGS, NamePtr name,
                                       int idx /* = -1 */)
  : LvalExpression(EXPRESSION_PASS), m_name(name), m_idx(idx) { }

Variant &VariableExpression::getRef(VariableEnvironment &env,
                                    bool initNotice) const {
  Variant *var = NULL;
  String name;
  if (m_idx == -1 || !(var = env.getIdx(m_idx))) {
    name = m_name->get(env);
    var =  &env.get(name);
    if (m_idx != -1) env.setIdx(m_idx, var);
  }
  /* note that 'if (!env.exists(str, name->hash()))' does not work
   * as undefined local variables are still in the (function) environment */
  if (initNotice && !var->isInitialized()) {
    if (name.empty()) {
      name = m_name->get();
    }
    SET_LINE;
    raise_notice("Undefined variable: %s", name.c_str());
  }
  return *var;
}

Variant VariableExpression::eval(VariableEnvironment &env) const {
  return getRef(env, true);
}

Variant VariableExpression::evalExist(VariableEnvironment &env) const {
  return getRef(env, false);
}

Variant &VariableExpression::lval(VariableEnvironment &env) const {
  return getRef(env, false);
}

bool VariableExpression::weakLval(VariableEnvironment &env, Variant* &v) const
{
  v = &getRef(env, true);
  return true;
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
      throw_strict(TypeVariableChangeException(location_to_string(loc())),
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
      throw_strict(TypeVariableChangeException(location_to_string(loc())),
                   StrictMode::StrictHardCore);
    }
  }
  return LvalExpression::setOpVariant(lhs, op, rhs);
}

void VariableExpression::unset(VariableEnvironment &env) const {
  String name(m_name->get(env));
  env.unset(name, m_name->hash());
}

NamePtr VariableExpression::getName() const {
  return m_name;
}

void VariableExpression::dump(std::ostream &out) const {
  if (m_name->get().isNull()) {
    out << "${";
    m_name->dump(out);
    out << "}";
  } else {
    out << "$";
    m_name->dump(out);
  }
}

///////////////////////////////////////////////////////////////////////////////
}
}

