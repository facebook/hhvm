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

#ifndef __EVAL_OBJECT_PROPERTY_EXPRESSION_H__
#define __EVAL_OBJECT_PROPERTY_EXPRESSION_H__

#include <runtime/eval/ast/lval_expression.h>

namespace HPHP {
namespace Eval {
///////////////////////////////////////////////////////////////////////////////

DECLARE_AST_PTR(ObjectPropertyExpression);
DECLARE_AST_PTR(Name);

class ObjectPropertyExpression : public LvalExpression {
public:
  ObjectPropertyExpression(EXPRESSION_ARGS, ExpressionPtr obj, NamePtr name);
  virtual Variant eval(VariableEnvironment &env) const;
  virtual Variant evalExist(VariableEnvironment &env) const;
  virtual Variant &lval(VariableEnvironment &env) const;
  virtual bool weakLval(VariableEnvironment &env, Variant* &v) const;
  virtual Variant set(VariableEnvironment &env, CVarRef val) const;
  virtual bool exist(VariableEnvironment &env, int op) const;
  virtual void unset(VariableEnvironment &env) const;
  virtual Variant setOp(VariableEnvironment &env, int op, CVarRef rhs) const;
  ExpressionPtr getObject() { return m_obj; }
  NamePtr getProperty() const;
  virtual void dump() const;
private:
  ExpressionPtr m_obj;
  NamePtr m_name;
};

///////////////////////////////////////////////////////////////////////////////
}
}

#endif /* __EVAL_OBJECT_PROPERTY_EXPRESSION_H__ */
