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

#ifndef __EVAL_VARIABLE_EXPRESSION_H__
#define __EVAL_VARIABLE_EXPRESSION_H__

#include <runtime/eval/ast/name.h>
#include <runtime/eval/ast/lval_expression.h>
#include <runtime/eval/runtime/variable_environment.h>

namespace HPHP {
namespace Eval {
///////////////////////////////////////////////////////////////////////////////

DECLARE_AST_PTR(VariableExpression);
DECLARE_AST_PTR(ThisVariableExpression);

class VariableExpression : public LvalExpression {
public:
  VariableExpression(EXPRESSION_ARGS, NamePtr name, int idx = -1);
  virtual Expression *optimize(VariableEnvironment &env);
  virtual Variant eval(VariableEnvironment &env) const;
  virtual Variant evalExist(VariableEnvironment &env) const;
  virtual Variant &lval(VariableEnvironment &env) const;
  virtual bool weakLval(VariableEnvironment &env, Variant* &v) const;
  virtual void unset(VariableEnvironment &env) const;
  virtual Variant set(VariableEnvironment &env, CVarRef val) const;
  virtual Variant setRef(VariableEnvironment &env, CVarRef val) const;
  virtual Variant setOp(VariableEnvironment &env, int op, CVarRef rhs) const;
  Name *getName() const { return m_name.get(); }
  int getIdx() const { return m_idx; }
  virtual void dump(std::ostream &out) const;
  static bool CheckCompatibleAssignment(CVarRef left, CVarRef right);
  bool isSuperGlobal() const;
  inline static Variant &GetVariableByRefCheck(VariableEnvironment &env,
                                               Expression *exp) {
    ASSERT(exp->isKindOf(KindOfVariableExpression));
    return static_cast<VariableExpression *>(exp)->getRefCheck(env);
  }
  inline static Variant &GetVariableByRef(
    VariableEnvironment &env, Expression *exp) {
    ASSERT(exp->isKindOf(KindOfVariableExpression));
    return static_cast<VariableExpression *>(exp)->getRef(env);
  }
  Variant &getRef(VariableEnvironment &env) const {
    Variant *var = NULL;
    if (m_idx != -1 && (var = env.getIdx(m_idx))) return *var;
    return getRefHelper(env);
  }
  Variant &getRefCheck(VariableEnvironment &env) const {
    Variant &var = getRef(env);
    /* note that 'if (!env.exists(str, name->hash()))' does not work
     * as undefined local variables are still in the (function) environment */
    if (!var.isInitialized()) raiseUndefined(env);
    return var;
  }
protected:
  NamePtr m_name;
  int m_idx;
  void raiseUndefined(VariableEnvironment &env) const;
  Variant &getRefHelper(VariableEnvironment &env) const;
};

class ThisVariableExpression : public VariableExpression {
public:
  ThisVariableExpression(EXPRESSION_ARGS, NamePtr name, int idx = -1) :
    VariableExpression(EXPRESSION_PASS, name, idx) {
    m_kindOf = KindOfThisVariableExpression;
  }
  virtual Variant eval(VariableEnvironment &env) const;
  virtual Variant evalExist(VariableEnvironment &env) const;
  virtual Variant &lval(VariableEnvironment &env) const;
  virtual bool weakLval(VariableEnvironment &env, Variant* &v) const;
  virtual void unset(VariableEnvironment &env) const;
  virtual Variant set(VariableEnvironment &env, CVarRef val) const {
    // Cannot re-assign $this
    assert(false);
  }
  virtual Variant setRef(VariableEnvironment &env, CVarRef val) const {
    // Cannot re-assign $this
    assert(false);
  }
  // virtual Variant setOp(VariableEnvironment &env, int op, CVarRef rhs) const;
private:
  Variant *getThis(VariableEnvironment &env) const;
};

///////////////////////////////////////////////////////////////////////////////
}
}

#endif /* __EVAL_VARIABLE_EXPRESSION_H__ */
