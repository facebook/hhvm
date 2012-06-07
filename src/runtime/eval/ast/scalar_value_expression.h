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

#ifndef __EVAL_AST_SCALAR_VALUE_EXPRESSION_H__
#define __EVAL_AST_SCALAR_VALUE_EXPRESSION_H__

#include <runtime/eval/ast/expression.h>

namespace HPHP {
namespace Eval {
///////////////////////////////////////////////////////////////////////////////

DECLARE_AST_PTR(ScalarValueExpression);

class ScalarValueExpression : public Expression {
public:
  ScalarValueExpression(CVarRef value, const Location* loc);
  ~ScalarValueExpression();
  virtual bool evalScalar(VariableEnvironment &env, Variant &r) const {
    r = m_value;
    return true;
  }
  virtual bool evalStaticScalar(VariableEnvironment &env, Variant &r) const {
    r = m_value;
    return true;
  }
  virtual Variant eval(VariableEnvironment &env) const { return m_value; }
  virtual void dump(std::ostream &out) const;
  inline static Variant &GetScalarValueByRef(Expression *exp) {
    ASSERT(exp->isKindOf(KindOfScalarValueExpression));
    return static_cast<ScalarValueExpression *>(exp)->m_value;
  }
  static void initScalarValues();
  static void registerScalarValues();
  static ScalarValueExpression *GetScalarValueExpression(
    ScalarValueExpression *exp);
  static void InsertExpressionPtr(ExpressionPtr *astPtr);
  static void RemoveExpressionPtr(ExpressionPtr *astPtr);
private:
  typedef std::set<ScalarValueExpression *> ScalarValueExpressionSet;
  static DECLARE_THREAD_LOCAL(ScalarValueExpressionSet,
                              s_scalarValueExpressions);
  typedef std::set<ExpressionPtr *> ScalarValueExpressionRefSet;
  static DECLARE_THREAD_LOCAL(ScalarValueExpressionRefSet, s_refs);
  Variant m_value;
};

///////////////////////////////////////////////////////////////////////////////
}
}

#endif /* __EVAL_AST_SCALAR_VALUE_EXPRESSION_H__ */
