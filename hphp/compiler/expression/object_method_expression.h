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

#ifndef __OBJECT_METHOD_EXPRESSION_H__
#define __OBJECT_METHOD_EXPRESSION_H__

#include <compiler/expression/function_call.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(ObjectMethodExpression);
DECLARE_BOOST_TYPES(ClassScope);

class ObjectMethodExpression : public FunctionCall {
public:
  ObjectMethodExpression(EXPRESSION_CONSTRUCTOR_PARAMETERS,
                         ExpressionPtr object, ExpressionPtr method,
                         ExpressionListPtr params);

  DECLARE_BASE_EXPRESSION_VIRTUAL_FUNCTIONS;
  virtual ConstructPtr getNthKid(int n) const;
  virtual void setNthKid(int n, ConstructPtr cp);
  ExpressionPtr preOptimize(AnalysisResultConstPtr ar);

  virtual TypePtr inferAndCheck(AnalysisResultPtr ar, TypePtr type,
                                bool coerce);
  virtual bool preOutputCPP(CodeGenerator &cg, AnalysisResultPtr ar,
      int state);

  ExpressionPtr getObject() const { return m_object; }
private:
  ExpressionPtr m_object;
  int m_objTemp;

  void setInvokeParams(AnalysisResultPtr ar);
  // for avoiding code generate toObject(Variant)
  bool directVariantProxy(AnalysisResultPtr ar);
  bool m_bindClass;

  void outputCPPObject(CodeGenerator &cg, AnalysisResultPtr ar);
  void outputCPPObjectCall(CodeGenerator &cg, AnalysisResultPtr ar);
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __OBJECT_METHOD_EXPRESSION_H__
