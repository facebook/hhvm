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

#ifndef __FUNCTION_CALL_H__
#define __FUNCTION_CALL_H__

#include <compiler/expression/static_class_name.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(ExpressionList);
DECLARE_BOOST_TYPES(FunctionScope);
DECLARE_BOOST_TYPES(FunctionCall);

class FunctionCall : public Expression, public StaticClassName {
public:
  FunctionCall(EXPRESSION_CONSTRUCTOR_PARAMETERS, ExpressionPtr nameExp,
               const std::string &name, ExpressionListPtr params,
               ExpressionPtr classExp);

  // overriding Expression::outputCPP to implement void wrapper
  virtual void outputCPP(CodeGenerator &cg, AnalysisResultPtr ar);
  virtual bool isRefable(bool checkError = false) const { return true;}
  virtual bool isTemporary() const;

  virtual ConstructPtr getNthKid(int n) const;
  virtual void setNthKid(int n, ConstructPtr cp);
  virtual int getKidCount() const;

  virtual ExpressionPtr preOptimize(AnalysisResultPtr ar);
  virtual ExpressionPtr postOptimize(AnalysisResultPtr ar);

  void setAllowVoidReturn() { m_allowVoidReturn = true;}
  void setFunctionAndClassScope(FunctionScopePtr fsp, ClassScopePtr csp);
  bool preOutputCPP(CodeGenerator &cg, AnalysisResultPtr ar,
                    int state);

protected:
  ExpressionPtr m_nameExp;
  std::string m_name;
  std::string m_origName;
  ExpressionListPtr m_params;

  // Pointers to the corresponding function scope and class scope for this
  // function call, set during the AnalyzeAll phase. These pointers may be
  // null if the function scope or class scope could not be resolved.
  FunctionScopePtr m_funcScope;
  ClassScopePtr m_classScope;

  bool m_valid;
  bool m_validClass;
  int m_extraArg;
  bool m_variableArgument;
  bool m_voidReturn;  // no return type
  bool m_voidWrapper; // void wrapper is needed
  bool m_allowVoidReturn;
  bool m_redeclared;
  bool m_redeclaredClass;
  bool m_derivedFromRedeclaring;

  // Extra arguments form an array, to which the scalar array optimization
  // should also apply.
  int m_argArrayId;
  void optimizeArgArray(AnalysisResultPtr ar);

  /**
   * Each program needs to reset this object's members to revalidate
   * a function call.
   */
  void reset();

  TypePtr checkParamsAndReturn(AnalysisResultPtr ar, TypePtr type,
                               bool coerce, FunctionScopePtr func);
};

///////////////////////////////////////////////////////////////////////////////
}
#endif // __FUNCTION_CALL_H__
