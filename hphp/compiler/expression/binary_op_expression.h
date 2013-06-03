/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

class BinaryOpExpression : public Expression {
public:
  BinaryOpExpression(EXPRESSION_CONSTRUCTOR_PARAMETERS,
                     ExpressionPtr exp1, ExpressionPtr exp2, int op);

  DECLARE_EXPRESSION_VIRTUAL_FUNCTIONS;
  ExpressionPtr preOptimize(AnalysisResultConstPtr ar);
  ExpressionPtr postOptimize(AnalysisResultConstPtr ar);
  virtual bool isTemporary() const;
  virtual int getLocalEffects() const;
  virtual bool isLiteralString() const;
  virtual std::string getLiteralString() const;
  virtual bool containsDynamicConstant(AnalysisResultPtr ar) const;

  virtual bool isRefable(bool checkError = false) const;
  bool isShortCircuitOperator() const;
  bool isLogicalOrOperator() const;
  ExpressionPtr getStoreVariable() const { return m_exp1;}
  ExpressionPtr getExp1() { return m_exp1;}
  ExpressionPtr getExp2() { return m_exp2;}
  int getOp() const { return m_op;}

  ExpressionPtr foldConst(AnalysisResultConstPtr ar);
  ExpressionPtr foldRightAssoc(AnalysisResultConstPtr ar);

  virtual ExpressionPtr unneededHelper();
  virtual bool canonCompare(ExpressionPtr e) const;

  static int getConcatList(ExpressionPtrVec &ev, ExpressionPtr exp,
                           bool &hasVoid);
  bool isAssignmentOp() const { return m_assign; }

private:
  void optimizeTypes(AnalysisResultConstPtr ar);
  ExpressionPtr simplifyLogical(AnalysisResultConstPtr ar);
  ExpressionPtr simplifyArithmetic(AnalysisResultConstPtr ar);
  bool isOpEqual();
  ExpressionPtr m_exp1;
  ExpressionPtr m_exp2;
  int m_op;
  bool m_assign;
  mutable bool m_canThrow;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_BINARY_OP_EXPRESSION_H_
