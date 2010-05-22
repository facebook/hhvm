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

#ifndef __ARRAY_PAIR_EXPRESSION_H__
#define __ARRAY_PAIR_EXPRESSION_H__

#include <compiler/expression/expression.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(ArrayPairExpression);

class ArrayPairExpression : public Expression {
public:
  ArrayPairExpression(EXPRESSION_CONSTRUCTOR_PARAMETERS,
                      ExpressionPtr name, ExpressionPtr value, bool ref);

  DECLARE_EXPRESSION_VIRTUAL_FUNCTIONS;
  virtual bool containsDynamicConstant(AnalysisResultPtr ar) const;
  virtual bool isScalar() const;

  ExpressionPtr getName() { return m_name;}
  ExpressionPtr getValue() { return m_value;}

  virtual int getLocalEffects() const { return NoEffect; }
  bool isScalarArrayPair() const;

  void outputCPPControlledEval(CodeGenerator &cg, AnalysisResultPtr ar,
                               int temp);
  void preOutputStash(CodeGenerator &cg, AnalysisResultPtr ar,
                      int state);

  bool isRef() const { return m_ref; }

  bool canonCompare(ExpressionPtr e) const;
private:
  ExpressionPtr m_name;
  ExpressionPtr m_value;
  bool m_ref;

  bool outputCPPName(CodeGenerator &cg, AnalysisResultPtr ar);
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __ARRAY_PAIR_EXPRESSION_H__
