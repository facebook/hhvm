/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_UNARY_OP_EXPRESSION_H_
#define incl_HPHP_UNARY_OP_EXPRESSION_H_

#include "hphp/compiler/expression/expression.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(UnaryOpExpression);

struct Variant;

class UnaryOpExpression : public Expression,
                          public LocalEffectsContainer {
private:
  void ctorInit();
protected:
  UnaryOpExpression(EXPRESSION_CONSTRUCTOR_BASE_PARAMETERS,
                    ExpressionPtr exp, int op, bool front);
public:
  UnaryOpExpression(EXPRESSION_CONSTRUCTOR_PARAMETERS,
                    ExpressionPtr exp, int op, bool front);

  DECLARE_EXPRESSION_VIRTUAL_FUNCTIONS;
  DECL_AND_IMPL_LOCAL_EFFECTS_METHODS;

  ExpressionPtr preOptimize(AnalysisResultConstPtr ar) override;
  void onParse(AnalysisResultConstPtr ar, FileScopePtr scope);
  bool isRefable(bool checkError = false) const override;
  bool isScalar() const override;
  bool isThis() const override;
  bool containsDynamicConstant(AnalysisResultPtr ar) const override;
  bool getScalarValue(Variant &value) override;

  ExpressionPtr getExpression() { return m_exp;}
  ExpressionPtr getStoreVariable() const override { return m_exp;}
  int getOp() const { return m_op;}
  bool isLogicalNot() const { return m_op == '!'; }
  bool isCast() const;
  bool getFront() const { return m_front; }

  ExpressionPtr unneededHelper() override;
  void setDefinedScope(BlockScopeRawPtr scope);
protected:
  ExpressionPtr m_exp;
  BlockScopeRawPtr m_definedScope;
  int m_op;
  bool m_front;

private:
  bool preCompute(const Variant& value, Variant &result);
  void setExistContext();
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_UNARY_OP_EXPRESSION_H_
