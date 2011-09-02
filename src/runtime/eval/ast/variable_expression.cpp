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
#include <runtime/eval/ast/expression.h>
#include <runtime/eval/ast/variable_expression.h>
#include <runtime/eval/ast/name.h>
#include <runtime/base/runtime_option.h>
#include <runtime/eval/strict_mode.h>

namespace HPHP {
namespace Eval {
///////////////////////////////////////////////////////////////////////////////

static StaticString s_this("this");

VariableExpression::VariableExpression(EXPRESSION_ARGS, NamePtr name,
                                       int idx /* = -1 */)
  : LvalExpression(KindOfVariableExpression, EXPRESSION_PASS),
  m_name(name), m_idx(idx) {}

bool VariableExpression::isSuperGlobal() const {
  SuperGlobal sg;
  return m_name->getSuperGlobal(sg);
}

Variant &VariableExpression::getRefHelper(
  VariableEnvironment &env) const {
  CStrRef s = m_name->get(env);
  SuperGlobal sg;
  if (!m_name->getSuperGlobal(sg)) {
    sg = VariableIndex::isSuperGlobal(s);
  }
  if (m_idx != -1 && sg == SgNormal &&
      env.isKindOf(VariableEnvironment::KindOfFuncScopeVariableEnvironment)) {
    FuncScopeVariableEnvironment *fenv =
      static_cast<FuncScopeVariableEnvironment *>(&env);
    ASSERT(fenv->getIdx(m_idx) == NULL);
    AssocList &alist = fenv->getAssocList();
    ASSERT(!alist.getPtr(s));
    Variant *var = &alist.append(s);
    fenv->setIdx(m_idx, var);
    return *var;
  }
  Variant *var =  &env.getVar(s, sg);
  if (m_idx != -1) env.setIdx(m_idx, var);
  return *var;
}

void VariableExpression::raiseUndefined(VariableEnvironment &env) const {
  SET_LINE_VOID;
  raise_notice("Undefined variable: %s", m_name->get(env).c_str());
}

Variant VariableExpression::eval(VariableEnvironment &env) const {
  return getRefCheck(env);
}

Variant *ThisVariableExpression::getThis(VariableEnvironment &env) const {
  if (!env.currentObject().isNull()) {
    Variant *var = NULL;
    ASSERT(m_idx != -1);
    if (var = env.getIdx(m_idx)) return var;
    var = &env.getVar(s_this, SgNormal);
    var->assign(env.currentObject());
    return var;
  }
  return NULL;
}

Variant ThisVariableExpression::eval(VariableEnvironment &env) const {
  if (Variant *var = getThis(env)) return *var;
  return getRefCheck(env);
}

Variant VariableExpression::evalExist(VariableEnvironment &env) const {
  return getRef(env);
}

Variant ThisVariableExpression::evalExist(VariableEnvironment &env) const {
  if (Variant *var = getThis(env)) return *var;
  return getRef(env);
}

Variant &VariableExpression::lval(VariableEnvironment &env) const {
  return getRef(env);
}

Variant &ThisVariableExpression::lval(VariableEnvironment &env) const {
  if (Variant *var = getThis(env)) return *var;
  return getRef(env);
}

bool VariableExpression::weakLval(VariableEnvironment &env, Variant* &v)
  const {
  v = &getRefCheck(env);
  return true;
}

bool ThisVariableExpression::weakLval(VariableEnvironment &env, Variant* &v)
  const {
  if (v = getThis(env)) return true;
  v = &getRefCheck(env);
  return true;
}

bool VariableExpression::CheckCompatibleAssignment(CVarRef left, CVarRef right) {
  DataType lhs = left.getType();
  if (lhs == KindOfUninit || lhs == KindOfNull) {
    // everything is ok with NULL
    return true;
  }
  DataType rhs = right.getType();
  switch (lhs) {
  case KindOfBoolean: return rhs == KindOfBoolean;
  case KindOfDouble:  return rhs == KindOfDouble;
  case KindOfArray:   return rhs == KindOfArray;
  case KindOfObject:  return rhs == KindOfObject;

  case KindOfStaticString:
  case KindOfString:
    return rhs == KindOfStaticString || rhs == KindOfString;

  case KindOfInt32:
  case KindOfInt64:
    return rhs == KindOfInt32 || rhs == KindOfInt64;
  default:
    ASSERT(false);
  }
  return true;
}

Variant VariableExpression::set(VariableEnvironment &env, CVarRef val) const {
  Variant &lhs = lval(env);
  if (RuntimeOption::EnableStrict) {
    if (!CheckCompatibleAssignment(lhs, val)) {
      throw_strict(TypeVariableChangeException(location_to_string(loc())),
                   StrictMode::StrictHardCore);
    }
  }
  return lhs.assignVal(val);
}

Variant VariableExpression::setRef(VariableEnvironment &env, CVarRef val)
  const {
  Variant &lhs = lval(env);
  if (RuntimeOption::EnableStrict) {
    if (!CheckCompatibleAssignment(lhs, val)) {
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
    if (!CheckCompatibleAssignment(lhs, rhs)) {
      throw_strict(TypeVariableChangeException(location_to_string(loc())),
                   StrictMode::StrictHardCore);
    }
  }
  return LvalExpression::setOpVariant(lhs, op, rhs);
}

void VariableExpression::unset(VariableEnvironment &env) const {
  HPHP::unset(getRef(env));
}

void ThisVariableExpression::unset(VariableEnvironment &env) const {
  if (Variant *var = getThis(env)) {
    HPHP::unset(*var);
    return;
  }
  HPHP::unset(getRefCheck(env));
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

