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

namespace HPHP {
namespace Eval {
///////////////////////////////////////////////////////////////////////////////

DECLARE_AST_PTR(VariableExpression);

class VariableExpression : public LvalExpression {
public:
  VariableExpression(EXPRESSION_ARGS, NamePtr name, int idx = -1);
  virtual Variant eval(VariableEnvironment &env) const;
  virtual Variant evalExist(VariableEnvironment &env) const;
  virtual Variant &lval(VariableEnvironment &env) const;
  virtual bool weakLval(VariableEnvironment &env, Variant* &v) const;
  virtual void unset(VariableEnvironment &env) const;
  virtual Variant set(VariableEnvironment &env, CVarRef val) const;
  virtual Variant setOp(VariableEnvironment &env, int op, CVarRef rhs) const;
  NamePtr getName() const;
  virtual void dump(std::ostream &out) const;
private:
  NamePtr m_name;
  int m_idx;
  Variant &getRef(VariableEnvironment &env, bool initNotice) const;
};

///////////////////////////////////////////////////////////////////////////////
}
}

#endif /* __EVAL_VARIABLE_EXPRESSION_H__ */
