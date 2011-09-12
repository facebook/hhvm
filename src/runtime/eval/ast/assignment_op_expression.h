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

#ifndef __EVAL_ASSIGNMENT_OP_EXPRESSION_H__
#define __EVAL_ASSIGNMENT_OP_EXPRESSION_H__

#include <runtime/eval/ast/expression.h>

namespace HPHP {
namespace Eval {
///////////////////////////////////////////////////////////////////////////////

DECLARE_AST_PTR(AssignmentOpExpression);
DECLARE_AST_PTR(LvalExpression);

class AssignmentOpExpression : public Expression {
public:
  AssignmentOpExpression(EXPRESSION_ARGS, int op, LvalExpressionPtr lhs,
                         ExpressionPtr rhs);
  virtual Expression *optimize(VariableEnvironment &env);
  virtual Variant eval(VariableEnvironment &env) const;
  virtual Variant refval(VariableEnvironment &env, int strict = 2) const;
  LvalExpressionPtr getLhs() const { return m_lhs; }
  ExpressionPtr getRhs() const { return m_rhs; }
  virtual void dump(std::ostream &out) const;
private:
  int m_op;
  LvalExpressionPtr m_lhs;
  ExpressionPtr m_rhs;
};

class VariableAssignmentExpression : public Expression {
public:
  VariableAssignmentExpression(LvalExpressionPtr lhs, ExpressionPtr rhs,
                               const Location *loc);
  virtual Variant eval(VariableEnvironment &env) const;
  virtual Variant refval(VariableEnvironment &env, int strict = 2) const;
  virtual void dump(std::ostream &out) const;
private:
  LvalExpressionPtr m_lhs;
  ExpressionPtr m_rhs;
};

class VariablePlusEqualExpression : public Expression {
public:
  VariablePlusEqualExpression(LvalExpressionPtr lhs, ExpressionPtr rhs,
                              const Location *loc);
  virtual Variant eval(VariableEnvironment &env) const;
  virtual Variant refval(VariableEnvironment &env, int strict = 2) const;
  virtual void dump(std::ostream &out) const;
private:
  LvalExpressionPtr m_lhs;
  ExpressionPtr m_rhs;
};

class VariableMinusEqualExpression : public Expression {
public:
  VariableMinusEqualExpression(LvalExpressionPtr lhs, ExpressionPtr rhs,
                              const Location *loc);
  virtual Variant eval(VariableEnvironment &env) const;
  virtual Variant refval(VariableEnvironment &env, int strict = 2) const;
  virtual void dump(std::ostream &out) const;
private:
  LvalExpressionPtr m_lhs;
  ExpressionPtr m_rhs;
};

class VariableMulEqualExpression : public Expression {
public:
  VariableMulEqualExpression(LvalExpressionPtr lhs, ExpressionPtr rhs,
                              const Location *loc);
  virtual Variant eval(VariableEnvironment &env) const;
  virtual Variant refval(VariableEnvironment &env, int strict = 2) const;
  virtual void dump(std::ostream &out) const;
private:
  LvalExpressionPtr m_lhs;
  ExpressionPtr m_rhs;
};

class VariableAndEqualExpression : public Expression {
public:
  VariableAndEqualExpression(LvalExpressionPtr lhs, ExpressionPtr rhs,
                              const Location *loc);
  virtual Variant eval(VariableEnvironment &env) const;
  virtual Variant refval(VariableEnvironment &env, int strict = 2) const;
  virtual void dump(std::ostream &out) const;
private:
  LvalExpressionPtr m_lhs;
  ExpressionPtr m_rhs;
};

class VariableOrEqualExpression : public Expression {
public:
  VariableOrEqualExpression(LvalExpressionPtr lhs, ExpressionPtr rhs,
                              const Location *loc);
  virtual Variant eval(VariableEnvironment &env) const;
  virtual Variant refval(VariableEnvironment &env, int strict = 2) const;
  virtual void dump(std::ostream &out) const;
private:
  LvalExpressionPtr m_lhs;
  ExpressionPtr m_rhs;
};

class VariableXorEqualExpression : public Expression {
public:
  VariableXorEqualExpression(LvalExpressionPtr lhs, ExpressionPtr rhs,
                              const Location *loc);
  virtual Variant eval(VariableEnvironment &env) const;
  virtual Variant refval(VariableEnvironment &env, int strict = 2) const;
  virtual void dump(std::ostream &out) const;
private:
  LvalExpressionPtr m_lhs;
  ExpressionPtr m_rhs;
};

class VariableSLEqualExpression : public Expression {
public:
  VariableSLEqualExpression(LvalExpressionPtr lhs, ExpressionPtr rhs,
                              const Location *loc);
  virtual Variant eval(VariableEnvironment &env) const;
  virtual Variant refval(VariableEnvironment &env, int strict = 2) const;
  virtual void dump(std::ostream &out) const;
private:
  LvalExpressionPtr m_lhs;
  ExpressionPtr m_rhs;
};

class VariableSREqualExpression : public Expression {
public:
  VariableSREqualExpression(LvalExpressionPtr lhs, ExpressionPtr rhs,
                              const Location *loc);
  virtual Variant eval(VariableEnvironment &env) const;
  virtual Variant refval(VariableEnvironment &env, int strict = 2) const;
  virtual void dump(std::ostream &out) const;
private:
  LvalExpressionPtr m_lhs;
  ExpressionPtr m_rhs;
};

///////////////////////////////////////////////////////////////////////////////
}
}

#endif /* __EVAL_ASSIGNMENT_OP_EXPRESSION_H__ */
