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

#ifndef incl_HPHP_FUNCTION_CALL_H_
#define incl_HPHP_FUNCTION_CALL_H_

#include "hphp/compiler/analysis/function_scope.h"
#include "hphp/compiler/expression/static_class_name.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(ExpressionList);
DECLARE_BOOST_TYPES(FunctionScope);
DECLARE_BOOST_TYPES(FunctionCall);

class FunctionCall : public Expression, public StaticClassName {
protected:
  FunctionCall(EXPRESSION_CONSTRUCTOR_BASE_PARAMETERS, ExpressionPtr nameExp,
               const std::string &name, bool hadBackslash,
               ExpressionListPtr params, ExpressionPtr classExp);
public:
  void analyzeProgram(AnalysisResultPtr ar) override;

  bool isRefable(bool checkError = false) const override { return true;}

  ConstructPtr getNthKid(int n) const override;
  void setNthKid(int n, ConstructPtr cp) override;
  int getKidCount() const override;

  ExpressionPtr preOptimize(AnalysisResultConstPtr ar) override;

  const std::string &getName() const = delete;//{ return m_name; }
  const std::string &getOriginalName() const { return m_origName; }
  const std::string getNonNSOriginalName() const {
    auto nsPos = m_origName.rfind('\\');
    if (nsPos == string::npos) {
      return m_origName;
    }
    return m_origName.substr(nsPos + 1);
  }
  ExpressionPtr getNameExp() const { return m_nameExp; }
  const ExpressionListPtr& getParams() const { return m_params; }
  void deepCopy(FunctionCallPtr exp);

  FunctionScopeRawPtr getFuncScope() const { return m_funcScope; }
  void setArrayParams() { m_arrayParams = true; }
  bool isValid() const { return m_valid; }
  bool hadBackslash() const { return m_hadBackslash; }
  bool hasUnpack() const;
  void onParse(AnalysisResultConstPtr ar, FileScopePtr fileScope) override;
  bool checkUnpackParams();
  bool isNamed(const char* name) const;
  bool isNamed(const std::string& name) const {
    return isNamed(name.c_str());
  }
private:
  void checkParamTypeCodeErrors(AnalysisResultPtr);

protected:
  ExpressionPtr m_nameExp;
//  std::string m_name;
  std::string m_origName;
  ExpressionListPtr m_params;

  // Pointers to the corresponding function scope and class scope for this
  // function call, set during the AnalyzeAll phase. These pointers may be
  // null if the function scope or class scope could not be resolved.
  FunctionScopeRawPtr m_funcScope;
  ClassScopeRawPtr m_classScope;

  bool m_valid;
  unsigned m_variableArgument : 1;
  unsigned m_redeclared : 1;
  unsigned m_arrayParams : 1;
  bool m_hadBackslash;

  void markRefParams(FunctionScopePtr func, const std::string &name);

  /**
   * Each program needs to reset this object's members to revalidate
   * a function call.
   */
  void reset();
};

///////////////////////////////////////////////////////////////////////////////
}
#endif // incl_HPHP_FUNCTION_CALL_H_
