/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#ifndef incl_HPHP_BINARY_OP_EXPRESSION_H_
#define incl_HPHP_BINARY_OP_EXPRESSION_H_

#include "hphp/compiler/expression/expression_list.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(BinaryOpExpression);

struct BinaryOpExpression : Expression {
  BinaryOpExpression(EXPRESSION_CONSTRUCTOR_PARAMETERS,
                     ExpressionPtr exp1, ExpressionPtr exp2, int op);

  DECLARE_EXPRESSION_VIRTUAL_FUNCTIONS;
  ExpressionPtr preOptimize(AnalysisResultConstPtr ar) override;
  int getLocalEffects() const override;
  bool isLiteralString() const override;
  std::string getLiteralString() const override;
  bool containsDynamicConstant(AnalysisResultPtr ar) const override;

  bool isRefable(bool checkError = false) const override;
  bool isShortCircuitOperator() const;
  bool isLogicalOrOperator() const;
  ExpressionPtr getStoreVariable() const override { return m_exp1;}
  ExpressionPtr getExp1() { return m_exp1;}
  ExpressionPtr getExp2() { return m_exp2;}
  int getOp() const { return m_op;}

  ExpressionPtr foldConst(AnalysisResultConstPtr ar);
  ExpressionPtr foldRightAssoc(AnalysisResultConstPtr ar);

  ExpressionPtr unneededHelper() override;

  static int getConcatList(std::vector<ExpressionPtr>& ev, ExpressionPtr exp,
                           bool &hasVoid);
  bool isAssignmentOp() const { return m_assign; }

private:
  ExpressionPtr m_exp1;
  ExpressionPtr m_exp2;
  int m_op;
  bool m_assign;
  mutable bool m_canThrow;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_BINARY_OP_EXPRESSION_H_
