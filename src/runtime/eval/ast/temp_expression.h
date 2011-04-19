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

#ifndef __EVAL_AST_TEMP_EXPRESSION_H__
#define __EVAL_AST_TEMP_EXPRESSION_H__

#include <runtime/eval/ast/expression.h>

namespace HPHP {
namespace Eval {
///////////////////////////////////////////////////////////////////////////////

DECLARE_AST_PTR(TempExpression);

class TempExpression : public Expression {
public:
  TempExpression(ExpressionPtr exp, int index = 0);
  virtual Variant eval(VariableEnvironment &env) const;
  virtual void dump(std::ostream &out) const;

  /**
   * In ListAssigment, we may be pointed to a different vector of temp
   * variables, after
   *
   *    list(TempExprList - [TempExpr1, TempExpr2],
   *         TempExprList - [TempExpr1, TempExpr2])
   *
   * becomes
   *
   *    TempExprList list([TempExpr1, TempExpr2],
   *                      [TempExpr3, TempExpr4]) <--- adjust to 3 and 4
   */
  void adjustIndex(int index0) { m_index += index0;}

private:
  ExpressionPtr m_exp;
  int m_index;
};

///////////////////////////////////////////////////////////////////////////////
}
}

#endif /* __EVAL_AST_TEMP_EXPRESSION_H__ */
