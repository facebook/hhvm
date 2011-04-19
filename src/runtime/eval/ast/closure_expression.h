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

#ifndef __EVAL_CLOSURE_EXPRESSION_H__
#define __EVAL_CLOSURE_EXPRESSION_H__

#include <runtime/eval/ast/expression.h>

namespace HPHP {
namespace Eval {
///////////////////////////////////////////////////////////////////////////////

DECLARE_AST_PTR(ClosureExpression);
DECLARE_AST_PTR(FunctionStatement);
DECLARE_AST_PTR(Parameter);

class ClosureExpression : public Expression {
public:
  ClosureExpression(EXPRESSION_ARGS, FunctionStatementPtr func,
                    const std::vector<ParameterPtr> &vars);
  virtual Variant eval(VariableEnvironment &env) const;
  virtual void dump(std::ostream &out) const;
  const std::vector<ParameterPtr> &getVars() const { return m_vars;}
private:
  FunctionStatementPtr m_func;
  std::vector<ParameterPtr> m_vars;
};

///////////////////////////////////////////////////////////////////////////////
}
}

#endif /* __EVAL_CLOSURE_EXPRESSION_H__ */
