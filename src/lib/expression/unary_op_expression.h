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

#ifndef __UNARY_OP_EXPRESSION_H__
#define __UNARY_OP_EXPRESSION_H__

#include <lib/expression/expression.h>
#include <cpp/base/type_variant.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(UnaryOpExpression);

class UnaryOpExpression : public Expression {
public:
  UnaryOpExpression(EXPRESSION_CONSTRUCTOR_PARAMETERS,
                    ExpressionPtr exp, int op, bool front);

  DECLARE_EXPRESSION_VIRTUAL_FUNCTIONS;
  virtual void onParse(AnalysisResultPtr ar);
  virtual bool hasEffect() const { return m_effect;}
  virtual bool isRefable() const;
  virtual bool isScalar() const;
  virtual bool isThis() const;
  virtual bool containsDynamicConstant(AnalysisResultPtr ar) const;
  virtual bool getScalarValue(Variant &value);

  ExpressionPtr getExpression() { return m_exp;}
  int getOp() const { return m_op;}

protected:
  ExpressionPtr m_exp;
  int m_op;
  bool m_front;
  bool m_effect;
  int m_arrayId;

private:
  bool preCompute(AnalysisResultPtr ar, CVarRef value, Variant &result);
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __UNARY_OP_EXPRESSION_H__
