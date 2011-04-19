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

#ifndef __EVAL_AST_TEMP_EXPRESSION_LIST_H__
#define __EVAL_AST_TEMP_EXPRESSION_LIST_H__

#include <runtime/eval/ast/lval_expression.h>

namespace HPHP {
namespace Eval {
///////////////////////////////////////////////////////////////////////////////

DECLARE_AST_PTR(Statement);
DECLARE_AST_PTR(TempExpression);
DECLARE_AST_PTR(TempExpressionList);

/**
 * This is artificial and purely for solving evaluation order problems with
 * array element expressions and object offset expressions in places like
 *
 *    $arr[subexpr1][subexpr2] = rhsexpr;
 *
 * where evaluation order should be
 *
 *     temp1 = subexpr1
 *     temp2 = subexpr2
 *     value = rhsexpr
 *     $arr[temp1][temp2] = value;
 *
 * Here we used TempExpressionList and TempExpression to turn original code
 * into
 *
 *    (subexpr1, subexpr2, $arr[temp1][temp2] = rhsexpr);
 *
 */
class TempExpressionList : public LvalExpression {
public:
  static Variant GetTempValue(VariableEnvironment &env, int index);

public:
  TempExpressionList(ExpressionPtr exp);

  // implementing Construct
  virtual void dump(std::ostream &out) const;

  // implementing Expression
  virtual Variant eval(VariableEnvironment &env) const;
  virtual Variant refval(VariableEnvironment &env, int strict = 2) const;
  virtual bool exist(VariableEnvironment &env, int op) const;
  virtual Variant evalExist(VariableEnvironment &env) const;
  virtual bool isRefParam() const;

  // implementing LvalExpression
  virtual Variant &lval(VariableEnvironment &env) const;
  virtual bool weakLval(VariableEnvironment &env, Variant* &v) const;
  virtual Variant set(VariableEnvironment &env, CVarRef val) const;
  virtual Variant setOp(VariableEnvironment &env, int op, CVarRef rhs) const;
  virtual void unset(VariableEnvironment &env) const;

  // without calling evalOffsets(), only for ForEachStatement
  Variant setImpl(VariableEnvironment &env, CVarRef val) const;

  /**
   * Append a subexpression to the list.
   */
  int append(ExpressionPtr offset);
  void append(TempExpressionPtr temp);

  /**
   * For assignment expression to turn
   *    (sub1, sub2, lval) = value
   * into
   *    (sub1, sub2, lval = value)
   */
  LvalExpressionPtr getLast();
  ExpressionPtr getLastExp() { return m_last;}
  void setLast(ExpressionPtr exp);

  /**
   * Move exp's m_offsets to me. For ListAssignment's re-order of offsets.
   */
  void takeOffsets(TempExpressionListPtr exp);

private:
  std::vector<ExpressionPtr> m_offsets;
  std::vector<TempExpressionPtr> m_temps;
  ExpressionPtr m_last;

  /**
   * Evaluate offset expressions and store as temp variables.
   */
  bool evalOffsets(VariableEnvironment &env) const;
  friend class TempExpressionHelper;
};

///////////////////////////////////////////////////////////////////////////////

class TempExpressionHelper {
public:
  TempExpressionHelper(const TempExpressionList *exp,
                       VariableEnvironment &env);
  ~TempExpressionHelper();

private:
  VariableEnvironment &m_env;
  bool m_release;
};

///////////////////////////////////////////////////////////////////////////////
}
}

#endif /* __EVAL_AST_TEMP_EXPRESSION_LIST_H__ */
