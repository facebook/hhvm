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

#ifndef __EVAL_AST_OBJECT_METHOD_EXPRESSION_H__
#define __EVAL_AST_OBJECT_METHOD_EXPRESSION_H__

#include <runtime/eval/ast/simple_function_call_expression.h>

namespace HPHP {
namespace Eval {
///////////////////////////////////////////////////////////////////////////////

DECLARE_AST_PTR(ObjectMethodExpression);
DECLARE_AST_PTR(Name);

class ObjectMethodExpression : public SimpleFunctionCallExpression {
public:
 ObjectMethodExpression(EXPRESSION_ARGS, ExpressionPtr obj, NamePtr name,
                        std::vector<ExpressionPtr> params);
  virtual Variant eval(VariableEnvironment &env) const;
  virtual void dump() const;
private:
  ExpressionPtr m_obj;
};

///////////////////////////////////////////////////////////////////////////////
}
}

#endif /* __EVAL_AST_OBJECT_METHOD_EXPRESSION_H__ */
