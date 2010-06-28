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

#ifndef __EVAL_SCALAR_EXPRESSION_H__
#define __EVAL_SCALAR_EXPRESSION_H__

#include <runtime/eval/ast/expression.h>

namespace HPHP {
namespace Eval {
///////////////////////////////////////////////////////////////////////////////

DECLARE_AST_PTR(ScalarExpression);

class ScalarExpression : public Expression {
public:
  ScalarExpression(EXPRESSION_ARGS);
  ScalarExpression(EXPRESSION_ARGS, bool b);
  ScalarExpression(EXPRESSION_ARGS, const std::string &s);
  ScalarExpression(EXPRESSION_ARGS, int type, const std::string &val);
  virtual Variant eval(VariableEnvironment &env) const;
  Variant getValue() const;
  virtual void dump() const;
private:
  enum Kind {
    SNull,
    SBool,
    SInt,
    SDouble,
    SString
  };
  std::string m_value;
  union {
    int64 num;
    double dbl;
  } m_num;
  Kind m_kind;
  bool m_quoted;
};

///////////////////////////////////////////////////////////////////////////////
}
}

#endif /* __EVAL_SCALAR_EXPRESSION_H__ */
