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

#include <runtime/eval/ast/object_property_expression.h>
#include <runtime/eval/ast/name.h>
#include <runtime/eval/ast/variable_expression.h>
#include <runtime/eval/runtime/variable_environment.h>
#include <util/parser/hphp.tab.hpp>

namespace HPHP {
namespace Eval {
///////////////////////////////////////////////////////////////////////////////

ObjectPropertyExpression::ObjectPropertyExpression(EXPRESSION_ARGS,
                                                   ExpressionPtr obj,
                                                   NamePtr name)
  : LvalExpression(KindOfObjectPropertyExpression, EXPRESSION_PASS),
  m_obj(obj), m_name(name) {
  m_reverseOrder = m_obj->isKindOf(KindOfVariableExpression);
}

Expression *ObjectPropertyExpression::optimize(VariableEnvironment &env) {
  Eval::optimize(env, m_obj);
  Eval::optimize(env, m_name);
  if (dynamic_cast<StringName *>(m_name.get())) {
    if (m_obj->isKindOf(Expression::KindOfThisExpression)) {
      return new ThisStringPropertyExpression(
        static_cast<StringName *>(m_name.get())->get(), loc());
    }
    if (m_obj->isKindOf(Expression::KindOfVariableExpression)) {
      return new VariableStringPropertyExpression
        (static_cast<VariableExpression *>(m_obj.get()),
        static_cast<StringName *>(m_name.get())->get(), loc());
    }
    return new ObjectStringPropertyExpression
      (m_obj.get(), static_cast<StringName *>(m_name.get())->get(), loc());
  }
  return NULL;
}

Variant ObjectPropertyExpression::eval(VariableEnvironment &env) const {
  String name;
  Variant obj;
  if (m_reverseOrder) {
    name = m_name->get(env);
    obj = m_obj->eval(env);
  } else {
    obj = m_obj->eval(env);
    name = m_name->get(env);
  }
  SET_LINE;
  if (!g_context->getDebuggerBypassCheck()) {
    return obj.o_get(name);
  }
  Variant v = obj.o_get(name, false);
  if (!v.isNull()) return v;
  CStrRef context = obj.isObject() ?
                      obj.getObjectData()->o_getClassName() :
                      null_string;
  return obj.o_get(name, false, context);
}

Variant ObjectPropertyExpression::evalExist(VariableEnvironment &env) const {
  if (m_reverseOrder) {
    String name(m_name->get(env));
    Variant obj(m_obj->evalExist(env));
    SET_LINE;
    return obj.o_get(name, false);
  }
  Variant obj(m_obj->evalExist(env));
  String name(m_name->get(env));
  SET_LINE;
  return obj.o_get(name, false);
}

Variant &ObjectPropertyExpression::lval(VariableEnvironment &env) const {
  // How annoying. Sometimes object is an lval and should be treated as such
  // and sometimes it isn't eg method call.
  const LvalExpression *lobj = m_obj->toLval();
  Variant &proxy = get_globals()->__lvalProxy;
  if (lobj) {
    Variant &lv = lobj->lval(env);
    String name(m_name->get(env));
    SET_LINE;
    return lv.o_lval(name, proxy);
  } else {
    Variant obj(m_obj->eval(env));
    String name(m_name->get(env));
    SET_LINE;
    proxy = ref(obj.o_lval(name, proxy));
    return proxy;
  }
}

bool ObjectPropertyExpression::weakLval(VariableEnvironment &env,
                                        Variant* &v) const {
  Variant *obj;

  const LvalExpression *lobj = m_obj->toLval();
  if (lobj) {
    bool ok = lobj->weakLval(env, obj);
    String name(m_name->get(env));
    if (!ok || !SET_LINE_EXPR) return false;
    Variant tmp;
    v = &obj->o_unsetLval(name, tmp);
    return v != &tmp;
  }

  m_obj->eval(env);
  m_name->get(env);
  return false;
}

Variant ObjectPropertyExpression::set(VariableEnvironment &env, CVarRef val)
  const {
  const LvalExpression *lobj = m_obj->toLval();
  if (lobj) {
    Variant &lv = lobj->lval(env);
    String name(m_name->get(env));
    SET_LINE;
    lv.o_set(name, val);
  } else {
    Variant obj(m_obj->eval(env));
    String name(m_name->get(env));
    SET_LINE;
    obj.o_set(name, val);
  }
  return val;
}

Variant ObjectPropertyExpression::setRef(VariableEnvironment &env, CVarRef val)
  const {
  const LvalExpression *lobj = m_obj->toLval();
  if (lobj) {
    Variant &lv = lobj->lval(env);
    String name(m_name->get(env));
    SET_LINE;
    lv.o_setRef(name, val);
  } else {
    Variant obj(m_obj->eval(env));
    String name(m_name->get(env));
    SET_LINE;
    obj.o_setRef(name, val);
  }
  return val;
}

Variant ObjectPropertyExpression::setOp(VariableEnvironment &env, int op,
                                        CVarRef rhs) const {
  Variant *vobj;
  Variant obj;
  const LvalExpression *lobj = m_obj->toLval();
  if (lobj) {
    vobj = &lobj->lval(env);
  } else {
    obj = m_obj->eval(env);
    vobj = &obj;
  }
  String name(m_name->get(env));
  SET_LINE;
  switch (op) {
    case T_PLUS_EQUAL:
      return vobj->o_assign_op<Variant, T_PLUS_EQUAL>(name, rhs);
    case T_MINUS_EQUAL:
      return vobj->o_assign_op<Variant, T_MINUS_EQUAL>(name, rhs);
    case T_MUL_EQUAL:
      return vobj->o_assign_op<Variant, T_MUL_EQUAL>(name, rhs);
    case T_DIV_EQUAL:
      return vobj->o_assign_op<Variant, T_DIV_EQUAL>(name, rhs);
    case T_CONCAT_EQUAL:
      return vobj->o_assign_op<Variant, T_CONCAT_EQUAL>(name, rhs);
    case T_MOD_EQUAL:
      return vobj->o_assign_op<Variant, T_MOD_EQUAL>(name, rhs);
    case T_AND_EQUAL:
      return vobj->o_assign_op<Variant, T_AND_EQUAL>(name, rhs);
    case T_OR_EQUAL:
      return vobj->o_assign_op<Variant, T_OR_EQUAL>(name, rhs);
    case T_XOR_EQUAL:
      return vobj->o_assign_op<Variant, T_XOR_EQUAL>(name, rhs);
    case T_SL_EQUAL:
      return vobj->o_assign_op<Variant, T_SL_EQUAL>(name, rhs);
    case T_SR_EQUAL:
      return vobj->o_assign_op<Variant, T_SR_EQUAL>(name, rhs);
    case T_INC:
      return vobj->o_assign_op<Variant, T_INC>(name, rhs);
    case T_DEC:
      return vobj->o_assign_op<Variant, T_DEC>(name, rhs);
    default:
      ASSERT(false);
  }
  return rhs;
}

bool ObjectPropertyExpression::exist(VariableEnvironment &env, int op) const {
  Variant obj;
  String name;
  if (m_reverseOrder) {
    obj = m_obj->evalExist(env);
    name = m_name->get(env);
  } else {
    name = m_name->get(env);
    obj = m_obj->evalExist(env);
  }
  SET_LINE;
  if (op == T_ISSET) {
    return obj.o_isset(name);
  } else {
    return obj.o_empty(name);
  }
}

void ObjectPropertyExpression::unset(VariableEnvironment &env) const {
  const LvalExpression *lobj = m_obj->toLval();
  Variant *obj;
  if (lobj && lobj->weakLval(env, obj)) {
    String name(m_name->get(env));
    obj->o_unset(name);
  } else {
    Variant obj(m_obj->evalExist(env));
    String name(m_name->get(env));
    obj.o_unset(name);
  }
}

NamePtr ObjectPropertyExpression::getProperty() const {
  return m_name;
}

void ObjectPropertyExpression::dump(std::ostream &out) const {
  m_obj->dump(out);
  out << "->";
  m_name->dump(out);
}

ThisStringPropertyExpression::ThisStringPropertyExpression(CStrRef name,
  const Location *loc) :
  LvalExpression(KindOfThisStringPropertyExpression, loc), m_name(name) {}

Variant ThisStringPropertyExpression::eval(VariableEnvironment &env) const {
  const Variant *op = &env.currentObject();
  SET_LINE;
  if (!g_context->getDebuggerBypassCheck()) {
    return op->o_get(m_name);
  }
  Variant v = op->o_get(m_name, false);
  if (!v.isNull()) return v;
  CStrRef context = op->isObject() ?
                    op->getObjectData()->o_getClassName() :
                    null_string;
  return op->o_get(m_name, false, context);
}

Variant ThisStringPropertyExpression::evalExist(VariableEnvironment &env)
  const {
  SET_LINE;
  return env.currentObject().o_get(m_name, false);
}

Variant &ThisStringPropertyExpression::lval(VariableEnvironment &env) const {
  Variant &proxy = get_globals()->__lvalProxy;
  if (!env.currentObject().is(KindOfObject)) {
    SET_LINE;
    raise_error("Using $this when not in an object context");
  }
  SET_LINE;
  return env.currentObject().o_lval(m_name, proxy);
}

bool ThisStringPropertyExpression::weakLval(VariableEnvironment &env,
                                        Variant* &v) const {
  Variant *obj = &env.currentObject();

  if (!obj->is(KindOfObject)) {
    SET_LINE;
    raise_error("Using $this when not in an object context");
    return false;
  }
  if (!SET_LINE_EXPR) return false;
  Variant tmp;
  v = &obj->o_unsetLval(m_name, tmp);
  return v != &tmp;
}

Variant ThisStringPropertyExpression::set(VariableEnvironment &env,
  CVarRef val) const {
  Variant &lv = env.currentObject();
  SET_LINE;
  lv.o_set(m_name, val);
  return val;
}

Variant ThisStringPropertyExpression::setRef(VariableEnvironment &env,
  CVarRef val) const {
  Variant &lv = env.currentObject();
  SET_LINE;
  lv.o_setRef(m_name, val);
  return val;
}

Variant ThisStringPropertyExpression::setOp(VariableEnvironment &env,
  int op, CVarRef rhs) const {
  Variant *vobj = &env.currentObject();

  if (!vobj->is(KindOfObject)) {
    SET_LINE;
    raise_error("Using $this when not in an object context");
  }
  SET_LINE;
  switch (op) {
    case T_PLUS_EQUAL:
      return vobj->o_assign_op<Variant, T_PLUS_EQUAL>(m_name, rhs);
    case T_MINUS_EQUAL:
      return vobj->o_assign_op<Variant, T_MINUS_EQUAL>(m_name, rhs);
    case T_MUL_EQUAL:
      return vobj->o_assign_op<Variant, T_MUL_EQUAL>(m_name, rhs);
    case T_DIV_EQUAL:
      return vobj->o_assign_op<Variant, T_DIV_EQUAL>(m_name, rhs);
    case T_CONCAT_EQUAL:
      return vobj->o_assign_op<Variant, T_CONCAT_EQUAL>(m_name, rhs);
    case T_MOD_EQUAL:
      return vobj->o_assign_op<Variant, T_MOD_EQUAL>(m_name, rhs);
    case T_AND_EQUAL:
      return vobj->o_assign_op<Variant, T_AND_EQUAL>(m_name, rhs);
    case T_OR_EQUAL:
      return vobj->o_assign_op<Variant, T_OR_EQUAL>(m_name, rhs);
    case T_XOR_EQUAL:
      return vobj->o_assign_op<Variant, T_XOR_EQUAL>(m_name, rhs);
    case T_SL_EQUAL:
      return vobj->o_assign_op<Variant, T_SL_EQUAL>(m_name, rhs);
    case T_SR_EQUAL:
      return vobj->o_assign_op<Variant, T_SR_EQUAL>(m_name, rhs);
    case T_INC:
      return vobj->o_assign_op<Variant, T_INC>(m_name, rhs);
    case T_DEC:
      return vobj->o_assign_op<Variant, T_DEC>(m_name, rhs);
    default:
      ASSERT(false);
  }
  return rhs;
}

bool ThisStringPropertyExpression::exist(VariableEnvironment &env, int op)
  const {
  Variant *obj = &env.currentObject();
  SET_LINE;
  if (op == T_ISSET) {
    return obj->o_isset(m_name);
  } else {
    return obj->o_empty(m_name);
  }
}

void ThisStringPropertyExpression::unset(VariableEnvironment &env) const {
  Variant *obj = &env.currentObject();

  if (!obj->is(KindOfObject)) {
    SET_LINE_VOID;
    raise_error("Using $this when not in an object context");
  }
  obj->o_unset(m_name);
}

void ThisStringPropertyExpression::dump(std::ostream &out) const {
  out << "$this->";
  out << m_name.data();
}

VariableStringPropertyExpression::VariableStringPropertyExpression(
  VariableExpressionPtr obj, CStrRef name, const Location *loc) :
  LvalExpression(KindOfVariableStringPropertyExpression, loc),
  m_obj(obj), m_name(name) {}

Variant VariableStringPropertyExpression::eval(VariableEnvironment &env) const {
  CVarRef obj = m_obj->getRef(env);
  SET_LINE;
  if (!g_context->getDebuggerBypassCheck()) {
    return obj.o_get(m_name);
  }
  Variant v = obj.o_get(m_name, false);
  if (!v.isNull()) return v;
  CStrRef context = obj.isObject() ?
                    obj.getObjectData()->o_getClassName() :
                    null_string;
  return obj.o_get(m_name, false, context);
}

Variant VariableStringPropertyExpression::evalExist(VariableEnvironment &env)
  const {
  CVarRef lv = m_obj->getRef(env);
  SET_LINE;
  return lv.o_get(m_name, false);
}

Variant &VariableStringPropertyExpression::lval(VariableEnvironment &env)
  const {
  Variant &proxy = get_globals()->__lvalProxy;
  Variant &lv = m_obj->getRef(env);
  SET_LINE;
  return lv.o_lval(m_name, proxy);
}

bool VariableStringPropertyExpression::weakLval(VariableEnvironment &env,
                                        Variant* &v) const {
  Variant *obj = &m_obj->getRefCheck(env);
  if (!SET_LINE_EXPR) return false;
  Variant tmp;
  v = &obj->o_unsetLval(m_name, tmp);
  return v != &tmp;
}

Variant VariableStringPropertyExpression::set(VariableEnvironment &env,
  CVarRef val) const {
  Variant &lv = m_obj->getRef(env);
  SET_LINE;
  lv.o_set(m_name, val);
  return val;
}

Variant VariableStringPropertyExpression::setRef(VariableEnvironment &env,
  CVarRef val) const {
  Variant &lv = m_obj->getRef(env);
  SET_LINE;
  lv.o_setRef(m_name, val);
  return val;
}

Variant VariableStringPropertyExpression::setOp(VariableEnvironment &env,
  int op, CVarRef rhs) const {
  Variant *vobj = &m_obj->getRef(env);
  SET_LINE;
  switch (op) {
    case T_PLUS_EQUAL:
      return vobj->o_assign_op<Variant, T_PLUS_EQUAL>(m_name, rhs);
    case T_MINUS_EQUAL:
      return vobj->o_assign_op<Variant, T_MINUS_EQUAL>(m_name, rhs);
    case T_MUL_EQUAL:
      return vobj->o_assign_op<Variant, T_MUL_EQUAL>(m_name, rhs);
    case T_DIV_EQUAL:
      return vobj->o_assign_op<Variant, T_DIV_EQUAL>(m_name, rhs);
    case T_CONCAT_EQUAL:
      return vobj->o_assign_op<Variant, T_CONCAT_EQUAL>(m_name, rhs);
    case T_MOD_EQUAL:
      return vobj->o_assign_op<Variant, T_MOD_EQUAL>(m_name, rhs);
    case T_AND_EQUAL:
      return vobj->o_assign_op<Variant, T_AND_EQUAL>(m_name, rhs);
    case T_OR_EQUAL:
      return vobj->o_assign_op<Variant, T_OR_EQUAL>(m_name, rhs);
    case T_XOR_EQUAL:
      return vobj->o_assign_op<Variant, T_XOR_EQUAL>(m_name, rhs);
    case T_SL_EQUAL:
      return vobj->o_assign_op<Variant, T_SL_EQUAL>(m_name, rhs);
    case T_SR_EQUAL:
      return vobj->o_assign_op<Variant, T_SR_EQUAL>(m_name, rhs);
    case T_INC:
      return vobj->o_assign_op<Variant, T_INC>(m_name, rhs);
    case T_DEC:
      return vobj->o_assign_op<Variant, T_DEC>(m_name, rhs);
    default:
      ASSERT(false);
  }
  return rhs;
}

bool VariableStringPropertyExpression::exist(VariableEnvironment &env, int op)
  const {
  CVarRef obj = m_obj->getRef(env);
  SET_LINE;
  if (op == T_ISSET) {
    return obj.o_isset(m_name);
  } else {
    return obj.o_empty(m_name);
  }
}

void VariableStringPropertyExpression::unset(VariableEnvironment &env) const {
  m_obj->getRef(env).o_unset(m_name);
}

void VariableStringPropertyExpression::dump(std::ostream &out) const {
  m_obj->dump(out);
  out << "->";
  out << m_name.data();
}

ObjectStringPropertyExpression::ObjectStringPropertyExpression(
  ExpressionPtr obj, CStrRef name, const Location *loc) :
  LvalExpression(KindOfObjectStringPropertyExpression, loc),
  m_obj(obj), m_name(name) {}

Variant ObjectStringPropertyExpression::eval(VariableEnvironment &env) const {
  CVarRef obj = m_obj->eval(env);
  SET_LINE;
  if (!g_context->getDebuggerBypassCheck()) {
    return obj.o_get(m_name);
  }
  Variant v = obj.o_get(m_name, false);
  if (!v.isNull()) return v;
  CStrRef context = obj.isObject() ?
                    obj.getObjectData()->o_getClassName() :
                    null_string;
  return obj.o_get(m_name, false, context);
}

Variant ObjectStringPropertyExpression::evalExist(
  VariableEnvironment &env) const {
  Variant obj(m_obj->evalExist(env));
  SET_LINE;
  return obj.o_get(m_name, false);
}

Variant &ObjectStringPropertyExpression::lval(VariableEnvironment &env) const {
  const LvalExpression *lobj = m_obj->toLval();
  Variant &proxy = get_globals()->__lvalProxy;
  if (lobj) {
    Variant &lv = lobj->lval(env);
    SET_LINE;
    return lv.o_lval(m_name, proxy);
  } else {
    Variant obj(m_obj->eval(env));
    SET_LINE;
    proxy = ref(obj.o_lval(m_name, proxy));
    return proxy;
  }
}

bool ObjectStringPropertyExpression::weakLval(VariableEnvironment &env,
                                              Variant* &v) const {
  Variant *obj;

  const LvalExpression *lobj = m_obj->toLval();
  if (lobj) {
    bool ok = lobj->weakLval(env, obj);
    if (!ok || !SET_LINE_EXPR) return false;
    Variant tmp;
    v = &obj->o_unsetLval(m_name, tmp);
    return v != &tmp;
  }

  m_obj->eval(env);
  return false;
}

Variant ObjectStringPropertyExpression::set(VariableEnvironment &env, CVarRef val)
  const {
  const LvalExpression *lobj = m_obj->toLval();
  if (lobj) {
    Variant &lv = lobj->lval(env);
    SET_LINE;
    lv.o_set(m_name, val);
  } else {
    Variant obj(m_obj->eval(env));
    SET_LINE;
    obj.o_set(m_name, val);
  }
  return val;
}

Variant ObjectStringPropertyExpression::setRef(
  VariableEnvironment &env, CVarRef val) const {
  const LvalExpression *lobj = m_obj->toLval();
  if (lobj) {
    Variant &lv = lobj->lval(env);
    SET_LINE;
    lv.o_setRef(m_name, val);
  } else {
    Variant obj(m_obj->eval(env));
    SET_LINE;
    obj.o_setRef(m_name, val);
  }
  return val;
}

Variant ObjectStringPropertyExpression::setOp(
  VariableEnvironment &env, int op, CVarRef rhs) const {
  Variant *vobj;
  Variant obj;
  const LvalExpression *lobj = m_obj->toLval();
  if (lobj) {
    vobj = &lobj->lval(env);
  } else {
    obj = m_obj->eval(env);
    vobj = &obj;
  }
  SET_LINE;
  switch (op) {
    case T_PLUS_EQUAL:
      return vobj->o_assign_op<Variant, T_PLUS_EQUAL>(m_name, rhs);
    case T_MINUS_EQUAL:
      return vobj->o_assign_op<Variant, T_MINUS_EQUAL>(m_name, rhs);
    case T_MUL_EQUAL:
      return vobj->o_assign_op<Variant, T_MUL_EQUAL>(m_name, rhs);
    case T_DIV_EQUAL:
      return vobj->o_assign_op<Variant, T_DIV_EQUAL>(m_name, rhs);
    case T_CONCAT_EQUAL:
      return vobj->o_assign_op<Variant, T_CONCAT_EQUAL>(m_name, rhs);
    case T_MOD_EQUAL:
      return vobj->o_assign_op<Variant, T_MOD_EQUAL>(m_name, rhs);
    case T_AND_EQUAL:
      return vobj->o_assign_op<Variant, T_AND_EQUAL>(m_name, rhs);
    case T_OR_EQUAL:
      return vobj->o_assign_op<Variant, T_OR_EQUAL>(m_name, rhs);
    case T_XOR_EQUAL:
      return vobj->o_assign_op<Variant, T_XOR_EQUAL>(m_name, rhs);
    case T_SL_EQUAL:
      return vobj->o_assign_op<Variant, T_SL_EQUAL>(m_name, rhs);
    case T_SR_EQUAL:
      return vobj->o_assign_op<Variant, T_SR_EQUAL>(m_name, rhs);
    case T_INC:
      return vobj->o_assign_op<Variant, T_INC>(m_name, rhs);
    case T_DEC:
      return vobj->o_assign_op<Variant, T_DEC>(m_name, rhs);
    default:
      ASSERT(false);
  }
  return rhs;
}

bool ObjectStringPropertyExpression::exist(
  VariableEnvironment &env, int op) const {
  Variant obj;
  obj = m_obj->evalExist(env);
  SET_LINE;
  if (op == T_ISSET) {
    return obj.o_isset(m_name);
  } else {
    return obj.o_empty(m_name);
  }
}

void ObjectStringPropertyExpression::unset(VariableEnvironment &env) const {
  const LvalExpression *lobj = m_obj->toLval();
  Variant *obj;
  if (lobj && lobj->weakLval(env, obj)) {
    obj->o_unset(m_name);
  } else {
    Variant obj(m_obj->evalExist(env));
    obj.o_unset(m_name);
  }
}

void ObjectStringPropertyExpression::dump(std::ostream &out) const {
  m_obj->dump(out);
  out << "->";
  out << m_name.data();
}

///////////////////////////////////////////////////////////////////////////////
}
}
