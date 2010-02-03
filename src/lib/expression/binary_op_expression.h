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

#ifndef __BINARY_OP_EXPRESSION_H__
#define __BINARY_OP_EXPRESSION_H__

#include <lib/expression/expression.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(BinaryOpExpression);

class BinaryOpExpression : public Expression {
public:
  BinaryOpExpression(EXPRESSION_CONSTRUCTOR_PARAMETERS,
                     ExpressionPtr exp1, ExpressionPtr exp2, int op);

  DECLARE_EXPRESSION_VIRTUAL_FUNCTIONS;
  virtual bool hasEffect() const { return m_effect;}
  virtual bool isLiteralString() const;
  virtual std::string getLiteralString() const;

  bool isShortCircuitOperator() const;
  ExpressionPtr getExp1() { return m_exp1;}
  ExpressionPtr getExp2() { return m_exp2;}
  int getOp() { return m_op;}

  ExpressionPtr foldConst(AnalysisResultPtr ar);
  ExpressionPtr foldConstRightAssoc(AnalysisResultPtr ar);

private:
  ExpressionPtr simplifyLogical(AnalysisResultPtr ar);
  ExpressionPtr simplifyArithmetic(AnalysisResultPtr ar);
  ExpressionPtr mergeConcat(AnalysisResultPtr ar);
  ExpressionPtr makeConcatCall(AnalysisResultPtr ar, int count,
                               ExpressionListPtr expList);

  ExpressionPtr m_exp1;
  ExpressionPtr m_exp2;
  int m_op;
  bool m_effect;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __BINARY_OP_EXPRESSION_H__
