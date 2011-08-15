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

#ifndef __EVAL_EXPRESSION_H__
#define __EVAL_EXPRESSION_H__

#include <runtime/eval/ast/construct.h>

namespace HPHP {
namespace Eval {
///////////////////////////////////////////////////////////////////////////////

class LvalExpression;
class ByteCodeProgram;

#define EXPRESSION_ARGS CONSTRUCT_ARGS
#define EXPRESSION_PASS CONSTRUCT_PASS

class Expression : public Construct {
public:
  enum KindOf {
    KindOfArrayElementExpression,
    KindOfArrayExpression,
    KindOfAssignmentOpExpression,
    KindOfAssignmentRefExpression,
    KindOfBinaryOpExpression,
    KindOfClassConstantExpression,
    KindOfClosureExpression,
    KindOfConstantExpression,
    KindOfEncapsListExpression,
    KindOfFunctionCallExpression,
    KindOfIncludeExpression,
    KindOfIncOpExpression,
    KindOfInstanceOfExpression,
    KindOfIssetExpression,
    KindOfListAssignmentExpression,
    KindOfObjectPropertyExpression,
    KindOfQOpExpression,
    KindOfRefParamExpression,
    KindOfScalarExpression,
    KindOfScalarValueExpression,
    KindOfStaticMemberExpression,
    KindOfTempExpression,
    KindOfTempExpressionList,
    KindOfThisExpression,
    KindOfUnaryOpExpression,
    KindOfVariableExpression,
  };
  Expression(KindOf kindOf, EXPRESSION_ARGS) : Construct(CONSTRUCT_PASS),
    m_kindOf(kindOf) {}

  Expression(KindOf kindOf, const Location *loc) : Construct(loc),
    m_kindOf(kindOf) {}
  bool isKindOf(KindOf kindOf) { return m_kindOf == kindOf; }
  virtual ~Expression() {}
  virtual Variant eval(VariableEnvironment &env) const = 0;
  virtual bool evalScalar(VariableEnvironment &env, Variant &r) const {
    return false;
  }
  virtual bool evalStaticScalar(VariableEnvironment &env, Variant &r) const {
    throw FatalErrorException("evalStaticScalar not implemented.");
  }
  virtual Expression *optimize(VariableEnvironment &env) {
    return NULL;
  }
  virtual Variant refval(VariableEnvironment &env, int strict = 2) const;
  virtual bool exist(VariableEnvironment &env, int op) const;
  virtual Variant evalExist(VariableEnvironment &env) const;
  virtual const LvalExpression *toLval() const;
  virtual bool isRefParam() const;
  KindOf getKindOf() const { return m_kindOf; }
protected:
  KindOf m_kindOf;
};

extern void register_for_scalar_value_expression(void *astPtr, bool insert);

template <>
class AstPtr<Expression> : public SmartPtr<Expression> {
public:
  AstPtr() : SmartPtr<Expression>() {}
  template<class Y>
  AstPtr(Y v) : SmartPtr<Expression>(v) {
    if (v && v->getKindOf() == Expression::KindOfScalarValueExpression) {
      register_for_scalar_value_expression(this, true);
    }
  }
  AstPtr(const AstPtr &src) : SmartPtr<Expression>(src) {
    if (m_px && m_px->getKindOf() == Expression::KindOfScalarValueExpression) {
      register_for_scalar_value_expression(this, true);
    }
  }
  ~AstPtr() {
    if (m_px && m_px->getKindOf() == Expression::KindOfScalarValueExpression) {
      register_for_scalar_value_expression(this, false);
    }
  }

  operator bool() const { return this->m_px; }

  template<class Y>
  void check(Y px) {
    if (m_px && m_px->getKindOf() == Expression::KindOfScalarValueExpression) {
      register_for_scalar_value_expression(this, false);
    }
    if (px && px->getKindOf() == Expression::KindOfScalarValueExpression) {
      register_for_scalar_value_expression(this, true);
    }
  }
  template<class Y>
  AstPtr &operator=(Y px) {
    SmartPtr<Expression>::operator=(px);
    check(px);
    return *this;
  }
  AstPtr &operator=(const AstPtr &src) {
    SmartPtr<Expression>::operator=(src.m_px);
    check(src.m_px);
    return *this;
  }

};

DECLARE_AST_PTR(Name);
DECLARE_AST_PTR(Expression);
DECLARE_AST_PTR(LvalExpression);

void optimize(VariableEnvironment &env, ExpressionPtr &exp);

///////////////////////////////////////////////////////////////////////////////
}
}

#endif /* __EVAL_EXPRESSION_H__ */
