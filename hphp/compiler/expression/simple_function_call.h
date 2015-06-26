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

#ifndef incl_HPHP_SIMPLE_FUNCTION_CALL_H_
#define incl_HPHP_SIMPLE_FUNCTION_CALL_H_

#include "hphp/compiler/expression/function_call.h"
#include <map>
#include "hphp/compiler/analysis/variable_table.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(SimpleFunctionCall);
struct SimpleFunctionCall final : FunctionCall {
  static void InitFunctionTypeMap();

public:
  SimpleFunctionCall(EXPRESSION_CONSTRUCTOR_PARAMETERS,
                     const std::string &name, bool hadBackslash,
                     ExpressionListPtr params, ExpressionPtr cls);

  DECLARE_BASE_EXPRESSION_VIRTUAL_FUNCTIONS;
  ExpressionPtr preOptimize(AnalysisResultConstPtr ar) override;
  void deepCopy(SimpleFunctionCallPtr exp);

  bool isDefineWithoutImpl(AnalysisResultConstPtr ar);
  void setValid() { m_valid = true; }
  void setThrowFatal() { m_type = FunType::ThrowFatal; }
  void setThrowParseFatal() { m_type = FunType::ThrowParseFatal; }
  bool isParseFatalFunction() const {
    return m_type == FunType::ThrowParseFatal;
  }
  bool isFatalFunction() const {
    return isParseFatalFunction() ||  m_type == FunType::ThrowFatal;
  }
  int isStaticCompact() const { return m_type == FunType::StaticCompact; }

  // define(<literal-string>, <scalar>);
  bool isSimpleDefine(StringData **name, TypedValue *value) const;

  int getLocalEffects() const override;

  // implementing IParseHandler
  void onParse(AnalysisResultConstPtr ar, FileScopePtr fs) override;

  void addLateDependencies(AnalysisResultConstPtr ar);
  void setSafeCall(int flag) { m_safe = flag; }
  void setSafeDefault(ExpressionPtr def) { m_safeDef = def; }
  ConstructPtr getNthKid(int n) const override;
  void setNthKid(int n, ConstructPtr cp) override;
  static SimpleFunctionCallPtr GetFunctionCallForCallUserFunc(
    AnalysisResultConstPtr ar, SimpleFunctionCallPtr call, int testOnly,
    int firstParam, bool &error);
  void setupScopes(AnalysisResultConstPtr ar);
  bool readsLocals() const;
  bool writesLocals() const;
  void updateVtFlags();
  void setLocalThis(const std::string &name) { m_localThis = name; }
  bool isCallToFunction(const char *name) const;
  void resolveNSFallbackFunc(AnalysisResultConstPtr ar, FileScopePtr fs);

  void changeToBytecode() {
    m_changedToBytecode = true;
  }
  bool hasBeenChangedToBytecode() {
    return m_changedToBytecode;
  }

protected:
  enum class FunType {
    Unknown,
    Define,
    Create,
    VariableArgument,
    Extract,
    Assert,
    Compact,
    StaticCompact, // compact() with statically known variable names
    ShellExec,
    Constant,
    Defined,
    FunctionExists,
    ClassExists,
    InterfaceExists,
    Unserialize,
    GetDefinedVars,
    FBCallUserFuncSafe,
    ThrowFatal,
    ThrowParseFatal,
    ClassAlias,
  };

  static std::map<std::string,FunType,stdltistr> FunctionTypeMap;
  FunType m_type;
  unsigned m_dynamicConstant : 1;
  unsigned m_builtinFunction : 1;
  unsigned m_dynamicInvoke : 1;
  unsigned m_transformed : 1;
  unsigned m_changedToBytecode : 1; // true if it morphed into a bytecode
  unsigned m_optimizable : 1; // true if it can be morphed into a bytecode

  int m_safe;
  ExpressionPtr m_safeDef;
  std::string m_lambda;

  ExpressionPtr optimize(AnalysisResultConstPtr ar);
private:
  FunctionScopePtr
  getFuncScopeFromParams(AnalysisResultPtr ar,
                         BlockScopeRawPtr scope,
                         ExpressionPtr clsName,
                         ExpressionPtr funcName,
                         ClassScopePtr &clsScope);
  std::string getThisString(bool withArrow);
  void mungeIfSpecialFunction(AnalysisResultConstPtr ar, FileScopePtr fs);

  std::string m_localThis;
};

SimpleFunctionCallPtr NewSimpleFunctionCall(
  EXPRESSION_CONSTRUCTOR_PARAMETERS,
  const std::string &name, bool hadBackslash, ExpressionListPtr params,
  ExpressionPtr cls);

///////////////////////////////////////////////////////////////////////////////
// hphp_opt functions

ExpressionPtr hphp_opt_fb_call_user_func(CodeGenerator *cg,
                                         AnalysisResultConstPtr ar,
                                         SimpleFunctionCallPtr call, int mode);

ExpressionPtr hphp_opt_is_callable(CodeGenerator *cg,
                                   AnalysisResultConstPtr ar,
                                   SimpleFunctionCallPtr call, int mode);

ExpressionPtr hphp_opt_call_user_func(CodeGenerator *cg,
                                      AnalysisResultConstPtr ar,
                                      SimpleFunctionCallPtr call, int mode);

///////////////////////////////////////////////////////////////////////////////
} // HPHP
#endif // incl_HPHP_SIMPLE_FUNCTION_CALL_H_
