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

DECLARE_AST_PTR(Name);
DECLARE_AST_PTR(Expression);
DECLARE_AST_PTR(LvalExpression);
class ByteCodeProgram;

#define EXPRESSION_ARGS CONSTRUCT_ARGS
#define EXPRESSION_PASS CONSTRUCT_PASS

class Expression : public Construct {
public:
  Expression(EXPRESSION_ARGS);
  Expression(const Location *loc) : Construct(loc) {}
  virtual ~Expression() {}
  virtual Variant eval(VariableEnvironment &env) const = 0;
  virtual Variant refval(VariableEnvironment &env, int strict = 2) const;
  virtual bool exist(VariableEnvironment &env, int op) const;
  virtual Variant evalExist(VariableEnvironment &env) const;
  virtual const LvalExpression *toLval() const;
  virtual bool isRefParam() const;

  static Variant evalVector(const std::vector<ExpressionPtr> &v,
                            VariableEnvironment &env);

};

///////////////////////////////////////////////////////////////////////////////
}
}

#endif /* __EVAL_EXPRESSION_H__ */
