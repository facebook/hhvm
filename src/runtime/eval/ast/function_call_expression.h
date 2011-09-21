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

#ifndef __EVAL_FUNCTION_CALL_EXPRESSION_H__
#define __EVAL_FUNCTION_CALL_EXPRESSION_H__

#include <runtime/eval/ast/expression.h>
#include <runtime/base/array/arg_array.h>

namespace HPHP {
namespace Eval {
///////////////////////////////////////////////////////////////////////////////

class FunctionCallExpression : public Expression {
public:
  const std::vector<ExpressionPtr> &params() const { return m_params; }
  Array getParams(VariableEnvironment &env) const;
  virtual bool exist(VariableEnvironment &env, int op) const;
  virtual Variant refval(VariableEnvironment &env, int strict = 2) const;
  virtual Expression *optimize(VariableEnvironment &env);
protected:
  FunctionCallExpression(EXPRESSION_ARGS,
                         const std::vector<ExpressionPtr> &params);
  FunctionCallExpression(const std::vector<ExpressionPtr> &params,
                         const Location *loc);
  inline Variant evalParam(VariableEnvironment &env,
    const CallInfo *ci, unsigned int index) const {
    ASSERT(0 <= index && index <= m_params.size());
    if (ci->mustBeRef(index)) return strongBind(m_params[index]->refval(env));
    if (ci->isRef(index)) return strongBind(m_params[index]->refval(env, 0));
    return m_params[index]->eval(env);
  }
  ArgArray *prepareArgArray(VariableEnvironment &env, const CallInfo *ci,
                            unsigned int count) const;
  std::vector<ExpressionPtr> m_params;
  void dumpParams(std::ostream &out) const;
};

///////////////////////////////////////////////////////////////////////////////
}
}

#endif /* __EVAL_FUNCTION_CALL_EXPRESSION_H__ */
