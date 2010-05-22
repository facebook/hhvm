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

#ifndef __EVAL_AST_REF_PARAM_EXPRESSION_H__
#define __EVAL_AST_REF_PARAM_EXPRESSION_H__

#include <runtime/eval/ast/lval_expression.h>

namespace HPHP {
namespace Eval {
///////////////////////////////////////////////////////////////////////////////

DECLARE_AST_PTR(RefParamExpression);

class RefParamExpression : public LvalExpression {
public:
  RefParamExpression(EXPRESSION_ARGS, LvalExpressionPtr lv);
  virtual Variant &lval(VariableEnvironment &env) const;
  virtual bool weakLval(VariableEnvironment &env, Variant* &v) const;
  virtual Variant refval(VariableEnvironment &env, bool strict = true) const;
  virtual Variant set(VariableEnvironment &env, CVarRef val) const;
  virtual Variant setOp(VariableEnvironment &env, int op, CVarRef rhs) const;
  virtual void unset(VariableEnvironment &env) const;
  virtual void dump() const;
  virtual bool isRefParam() const;
private:
  LvalExpressionPtr m_lv;
};

///////////////////////////////////////////////////////////////////////////////
}
}

#endif /* __EVAL_AST_REF_PARAM_EXPRESSION_H__ */
