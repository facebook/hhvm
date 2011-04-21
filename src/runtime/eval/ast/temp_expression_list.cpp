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

#include <runtime/eval/ast/temp_expression_list.h>
#include <runtime/eval/ast/temp_expression.h>
#include <runtime/eval/ast/variable_expression.h>
#include <runtime/eval/runtime/variable_environment.h>

using namespace std;

namespace HPHP {
namespace Eval {
///////////////////////////////////////////////////////////////////////////////

TempExpressionList::TempExpressionList(ExpressionPtr exp)
    : LvalExpression(exp->loc()), m_last(exp) {
}

int TempExpressionList::append(ExpressionPtr offset) {
  if (offset->is<VariableExpression>()) {
    return -1;
  }
  int index = m_offsets.size();
  m_offsets.push_back(offset);
  return index;
}

void TempExpressionList::append(TempExpressionPtr temp) {
  m_temps.push_back(temp);
}

LvalExpressionPtr TempExpressionList::getLast() {
  if (m_last) {
    return m_last->unsafe_cast<LvalExpression>();
  }
  return LvalExpressionPtr();
}

void TempExpressionList::setLast(ExpressionPtr exp) {
  m_last = exp;
}

void TempExpressionList::takeOffsets(TempExpressionListPtr exp) {
  int index0 = m_offsets.size();
  m_offsets.insert(m_offsets.end(), exp->m_offsets.begin(),
                   exp->m_offsets.end());
  exp->m_offsets.clear();

  for (unsigned int i = 0; i < exp->m_temps.size(); i++) {
    exp->m_temps[i]->adjustIndex(index0);
  }
  m_temps.insert(m_temps.end(), exp->m_temps.begin(), exp->m_temps.end());
  exp->m_temps.clear();
}

bool TempExpressionList::evalOffsets(VariableEnvironment &env) const {
  if (!m_offsets.empty()) {
    vector<Variant> &temps = env.createTempVariables();
    temps.reserve(m_offsets.size());
    for (unsigned int i = 0; i < m_offsets.size(); i++) {
      temps.push_back(m_offsets[i]->eval(env));
    }
    return true;
  }
  return false;
}

///////////////////////////////////////////////////////////////////////////////

TempExpressionHelper::TempExpressionHelper(const TempExpressionList *exp,
                                           VariableEnvironment &env)
    : m_env(env) {
  m_release = exp->evalOffsets(env);
}

TempExpressionHelper::~TempExpressionHelper() {
  if (m_release) {
    m_env.releaseTempVariables();
  }
}

///////////////////////////////////////////////////////////////////////////////

void TempExpressionList::dump(std::ostream &out) const {
  // those m_offsets will be dumped inside m_last by TempExpression
  m_last->dump(out);
}

Variant TempExpressionList::eval(VariableEnvironment &env) const {
  TempExpressionHelper helper(this, env);
  return m_last->eval(env);
}

Variant TempExpressionList::refval(VariableEnvironment &env,
                                   int strict /* = 2 */) const {
  TempExpressionHelper helper(this, env);
  return m_last->refval(env, strict);
}

bool TempExpressionList::exist(VariableEnvironment &env, int op) const {
  TempExpressionHelper helper(this, env);
  return m_last->exist(env, op);
}

Variant TempExpressionList::evalExist(VariableEnvironment &env) const {
  TempExpressionHelper helper(this, env);
  return m_last->evalExist(env);
}

bool TempExpressionList::isRefParam() const {
  return m_last->isRefParam();
}

Variant &TempExpressionList::lval(VariableEnvironment &env) const {
  TempExpressionHelper helper(this, env);
  const LvalExpression *exp = m_last->toLval();
  if (exp) {
    return exp->lval(env);
  }
  return Variant::lvalInvalid();
}

bool TempExpressionList::weakLval(VariableEnvironment &env,
                                  Variant* &v) const {
  TempExpressionHelper helper(this, env);
  const LvalExpression *exp = m_last->toLval();
  if (exp) {
    return exp->weakLval(env, v);
  }
  return false;
}

Variant TempExpressionList::set(VariableEnvironment &env, CVarRef val) const {
  TempExpressionHelper helper(this, env);
  return setImpl(env, val);
}

Variant TempExpressionList::setRef(VariableEnvironment &env,
                                   CVarRef val) const {
  TempExpressionHelper helper(this, env);
  return setRefImpl(env, val);
}

Variant TempExpressionList::setImpl(VariableEnvironment &env,
                                    CVarRef val) const {
  const LvalExpression *exp = m_last->toLval();
  if (exp) {
    return exp->set(env, val);
  }
  return null;
}

Variant TempExpressionList::setRefImpl(VariableEnvironment &env,
                                       CVarRef val) const {
  const LvalExpression *exp = m_last->toLval();
  if (exp) {
    return exp->setRef(env, val);
  }
  return null;
}

Variant TempExpressionList::setOp(VariableEnvironment &env, int op,
                                  CVarRef rhs) const {
  TempExpressionHelper helper(this, env);
  const LvalExpression *exp = m_last->toLval();
  if (exp) {
    return exp->setOp(env, op, rhs);
  }
  return null;
}

void TempExpressionList::unset(VariableEnvironment &env) const {
  TempExpressionHelper helper(this, env);
  const LvalExpression *exp = m_last->toLval();
  if (exp) {
    exp->unset(env);
  }
}

///////////////////////////////////////////////////////////////////////////////
}
}
