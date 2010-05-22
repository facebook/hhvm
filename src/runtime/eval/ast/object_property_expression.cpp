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

#include <runtime/eval/ast/object_property_expression.h>
#include <runtime/eval/ast/name.h>
#include <runtime/eval/runtime/variable_environment.h>
#include <runtime/eval/parser/hphp.tab.hpp>

namespace HPHP {
namespace Eval {
///////////////////////////////////////////////////////////////////////////////

ObjectPropertyExpression::ObjectPropertyExpression(EXPRESSION_ARGS,
                                                   ExpressionPtr obj,
                                                   NamePtr name)
  : LvalExpression(EXPRESSION_PASS), m_obj(obj), m_name(name) {
}

Variant ObjectPropertyExpression::eval(VariableEnvironment &env) const {
  Variant obj(m_obj->eval(env));
  String name(m_name->get(env));
  SET_LINE;
  return obj.o_get(name, m_name->hash());
}

Variant ObjectPropertyExpression::evalExist(VariableEnvironment &env) const {
  Variant obj(m_obj->evalExist(env));
  String name(m_name->get(env));
  SET_LINE;
  return obj.o_get(name, m_name->hash(), false);
}

Variant &ObjectPropertyExpression::lval(VariableEnvironment &env) const {
  // How annoying. Sometimes object is an lval and should be treated as such
  // and sometimes it isn't eg method call.
  const LvalExpression *lobj = m_obj->toLval();
  if (lobj) {
    Variant &lv = lobj->lval(env);
    String name(m_name->get(env));
    SET_LINE;
    return HPHP::lval(lv.o_lval(name, m_name->hash()));
  } else {
    Variant obj(m_obj->eval(env));
    String name(m_name->get(env));
    SET_LINE;
    return HPHP::lval(obj.o_lval(name, m_name->hash()));
  }
}

bool ObjectPropertyExpression::weakLval(VariableEnvironment &env,
                                        Variant* &v) const {
  Variant vobj(m_obj->eval(env));
  if (vobj.is(KindOfObject)) {
    Object obj(vobj);
    String name(m_name->get(env));
    SET_LINE;
    if (obj->o_exists(name, m_name->hash())) {
      v = &HPHP::lval(obj.o_lval(name, m_name->hash()));
      return true;
    }
  }
  return false;
}


Variant ObjectPropertyExpression::set(VariableEnvironment &env, CVarRef val)
  const {
  const LvalExpression *lobj = m_obj->toLval();
  if (lobj) {
    Variant &lv = lobj->lval(env);
    String name(m_name->get(env));
    SET_LINE;
    lv.o_lval(name, m_name->hash()) = val;
  } else {
    Variant obj(m_obj->eval(env));
    String name(m_name->get(env));
    SET_LINE;
    obj.o_lval(name, m_name->hash()) = val;
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
    vobj->o_lval(name, m_name->hash()) += rhs;
    break;
  case T_MINUS_EQUAL:
    vobj->o_lval(name, m_name->hash()) -= rhs;
    break;
  case T_MUL_EQUAL:
    vobj->o_lval(name, m_name->hash()) *= rhs;
    break;
  case T_DIV_EQUAL:
    vobj->o_lval(name, m_name->hash()) /= rhs;
    break;
  case T_CONCAT_EQUAL:
    concat_assign(vobj->o_lval(name, m_name->hash()), rhs);
    break;
  case T_MOD_EQUAL:
    vobj->o_lval(name, m_name->hash()) %= rhs;
    break;
  case T_AND_EQUAL:
    vobj->o_lval(name, m_name->hash()) &= rhs;
    break;
  case T_OR_EQUAL:
    vobj->o_lval(name, m_name->hash()) |= rhs;
    break;
  case T_XOR_EQUAL:
    vobj->o_lval(name, m_name->hash()) ^= rhs;
    break;
  case T_SL_EQUAL:
    vobj->o_lval(name, m_name->hash()) <<= rhs;
    break;
  case T_SR_EQUAL:
    vobj->o_lval(name, m_name->hash()) >>= rhs;
    break;
  default:
    ASSERT(false);
  }
  return rhs;
}

bool ObjectPropertyExpression::exist(VariableEnvironment &env, int op) const {
  Variant obj(m_obj->evalExist(env));
  String name(m_name->get(env));
  SET_LINE;
  if (op == T_ISSET) {
    // isset will cast to object
    return toObject(obj)->t___isset(name);
  } else {
    // empty
    return empty(toObject(obj).o_get(name, m_name->hash(), false));
  }
}
void ObjectPropertyExpression::unset(VariableEnvironment &env) const {
  Object obj(m_obj->eval(env));
  String name(m_name->get(env));
  SET_LINE;
  obj->t___unset(name);
}

NamePtr ObjectPropertyExpression::getProperty() const { return m_name; }

void ObjectPropertyExpression::dump() const {
  m_obj->dump();
  printf("->");
  m_name->dump();
}

///////////////////////////////////////////////////////////////////////////////
}
}

