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

#ifndef __EVAL_STATIC_MEMBER_EXPRESSION_H__
#define __EVAL_STATIC_MEMBER_EXPRESSION_H__

#include <runtime/eval/ast/lval_expression.h>

namespace HPHP {
namespace Eval {
///////////////////////////////////////////////////////////////////////////////

DECLARE_AST_PTR(StaticMemberExpression);
DECLARE_AST_PTR(Name);

class StaticMemberExpression : public LvalExpression {
public:
  StaticMemberExpression(EXPRESSION_ARGS, const NamePtr &cls,
      const NamePtr &variable);
  virtual void unset(VariableEnvironment &env) const;
  virtual bool exist(VariableEnvironment &env, int op) const;
  virtual Variant &lval(VariableEnvironment &env) const;
  virtual void dump(std::ostream &out) const;
private:
  NamePtr m_class;
  NamePtr m_variable;
};

///////////////////////////////////////////////////////////////////////////////
}
}

#endif /* __EVAL_STATIC_MEMBER_EXPRESSION_H__ */
