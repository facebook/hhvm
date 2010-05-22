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

#ifndef __EVAL_INC_OP_EXPRESSION_H__
#define __EVAL_INC_OP_EXPRESSION_H__

#include <runtime/eval/ast/expression.h>

namespace HPHP {
namespace Eval {
///////////////////////////////////////////////////////////////////////////////

DECLARE_AST_PTR(IncOpExpression);
DECLARE_AST_PTR(LvalExpression);

class IncOpExpression : public Expression {
public:
  IncOpExpression(EXPRESSION_ARGS, LvalExpressionPtr exp, bool inc, bool front);
  virtual Variant eval(VariableEnvironment &env) const;
  virtual void dump() const;
private:
  LvalExpressionPtr m_exp;
  bool m_inc;
  bool m_front;
};

///////////////////////////////////////////////////////////////////////////////
}
}

#endif /* __EVAL_INC_OP_EXPRESSION_H__ */
