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

#ifndef incl_HPHP_CLOSURE_EXPRESSION_H_
#define incl_HPHP_CLOSURE_EXPRESSION_H_

#include "hphp/compiler/expression/expression.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(ClosureExpression);
DECLARE_BOOST_TYPES(FunctionStatement);
DECLARE_BOOST_TYPES(ExpressionList);

class ClosureExpression : public Expression {
public:
  ClosureExpression(EXPRESSION_CONSTRUCTOR_PARAMETERS,
                    FunctionStatementPtr func, ExpressionListPtr vars);

  DECLARE_BASE_EXPRESSION_VIRTUAL_FUNCTIONS;

  virtual ConstructPtr getNthKid(int n) const;
  virtual void setNthKid(int n, ConstructPtr cp);
  virtual int getKidCount() const;

  FunctionStatementPtr getClosureFunction() { return m_func; }
  ExpressionListPtr getClosureVariables() { return m_vars; }
  ExpressionListPtr getClosureValues() { return m_values; }
  bool hasStaticLocals();

private:
  FunctionStatementPtr m_func;
  ExpressionListPtr m_vars;
  ExpressionListPtr m_values;

  static TypePtr s_ClosureType;

  bool hasStaticLocalsImpl(ConstructPtr root);
};

///////////////////////////////////////////////////////////////////////////////
}
#endif // incl_HPHP_CLOSURE_EXPRESSION_H_
