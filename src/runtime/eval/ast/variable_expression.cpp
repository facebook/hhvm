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
  : LvalExpression(KindOfVariableExpression, EXPRESSION_PASS),
  m_name(name), m_idx(idx) {}

inline Variant &VariableExpression::getRef(VariableEnvironment &env) const {
  Variant *var = NULL;
  if (m_idx == -1 || !(var = env.getIdx(m_idx))) {
    CStrRef s = m_name->get(env);
    SuperGlobal sg;
    if (!m_name->getSuperGlobal(sg)) {
      sg = VariableIndex::isSuperGlobal(s);
    }
    var =  &env.getVar(s, sg);
    if (m_idx != -1) env.setIdx(m_idx, var);
  }
  return *var;
}

Variant &VariableExpression::getRefCheck(VariableEnvironment &env) const {
  Variant &var = getRef(env);
  /* note that 'if (!env.exists(str, name->hash()))' does not work
   * as undefined local variables are still in the (function) environment */
  if (!var.isInitialized()) {
    SET_LINE;
    raise_notice("Undefined variable: %s", m_name->get(env).c_str());
  }
  return var;
}

Variant VariableExpression::eval(VariableEnvironment &env) const {
  return getRefCheck(env);
}

Variant VariableExpression::evalExist(VariableEnvironment &env) const {
  return getRef(env);
}

Variant &VariableExpression::lval(VariableEnvironment &env) const {
  return getRef(env);
}

bool VariableExpression::weakLval(VariableEnvironment &env, Variant* &v) const
{
  v = &getRefCheck(env);
  return true;
}

bool VariableExpression::checkCompatibleAssignment(CVarRef left,
                                                   CVarRef right) {
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
  return lhs.assignVal(val);
}

Variant VariableExpression::setRef(VariableEnvironment &env, CVarRef val) const {
  Variant &lhs = lval(env);
  if (RuntimeOption::EnableStrict) {
    if (!checkCompatibleAssignment(lhs, val)) {
      throw_strict(TypeVariableChangeException(location_to_string(loc())),
                   StrictMode::StrictHardCore);
    }
  }
  return lhs.assignRef(val);
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

