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
#include <runtime/eval/ast/ref_param_expression.h>

namespace HPHP {
namespace Eval {
///////////////////////////////////////////////////////////////////////////////

RefParamExpression::RefParamExpression(EXPRESSION_ARGS, LvalExpressionPtr lv)
  : LvalExpression(EXPRESSION_PASS), m_lv(lv) {}

Variant &RefParamExpression::lval(VariableEnvironment &env) const {
  return m_lv->lval(env);
}

bool RefParamExpression::weakLval(VariableEnvironment &env, Variant* &v) const {
  return m_lv->weakLval(env, v);
}

Variant RefParamExpression::refval(VariableEnvironment &env,
                                   bool strict /* = true */) const {
  return m_lv->refval(env, strict);
}

Variant RefParamExpression::set(VariableEnvironment &env, CVarRef val) const {
  return m_lv->set(env, val);
}

Variant RefParamExpression::setOp(VariableEnvironment &env, int op,
                                  CVarRef rhs) const {
  return m_lv->setOp(env, op, rhs);
}

void RefParamExpression::unset(VariableEnvironment &env) const {
  m_lv->unset(env);
}

void RefParamExpression::dump() const {
  printf("&");
  m_lv->dump();
}

bool RefParamExpression::isRefParam() const {
  return true;
}

///////////////////////////////////////////////////////////////////////////////
}
}
