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

#ifndef __EVAL_CLASS_CONSTANT_EXPRESSION_H__
#define __EVAL_CLASS_CONSTANT_EXPRESSION_H__

#include <runtime/eval/ast/expression.h>

namespace HPHP {
namespace Eval {
///////////////////////////////////////////////////////////////////////////////

DECLARE_AST_PTR(ClassConstantExpression);
DECLARE_AST_PTR(Name);

class ClassConstantExpression : public Expression {
public:
 ClassConstantExpression(EXPRESSION_ARGS, const NamePtr &cls,
                         const std::string &constant);
  virtual Variant eval(VariableEnvironment &env) const;
  virtual bool evalStaticScalar(VariableEnvironment &env, Variant &r) const;
  virtual void dump(std::ostream &out) const;
private:
  NamePtr m_class;
  std::string m_constant;
};

///////////////////////////////////////////////////////////////////////////////
}
}

#endif /* __EVAL_CLASS_CONSTANT_EXPRESSION_H__ */
