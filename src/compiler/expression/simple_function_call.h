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

#ifndef __SIMPLE_FUNCTION_CALL_H__
#define __SIMPLE_FUNCTION_CALL_H__

#include <compiler/expression/function_call.h>
#include <compiler/analysis/variable_table.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(SimpleFunctionCall);
class SimpleFunctionCall : public FunctionCall {
public:
  static void InitFunctionTypeMap();

public:
  SimpleFunctionCall(EXPRESSION_CONSTRUCTOR_PARAMETERS,
                     const std::string &name, ExpressionListPtr params,
                     ExpressionPtr cls);

  DECLARE_BASE_EXPRESSION_VIRTUAL_FUNCTIONS;
  ExpressionPtr preOptimize(AnalysisResultConstPtr ar);
  ExpressionPtr postOptimize(AnalysisResultConstPtr ar);
  void deepCopy(SimpleFunctionCallPtr exp);

  bool isDefineWithoutImpl(AnalysisResultConstPtr ar);
  void setValid() { m_valid = true; }
  void setNoPrefix() { m_noPrefix = true; }
  void setFromCompiler() { m_fromCompiler = true; }
  void setThrowFatal() { m_noPrefix = true; m_type = ThrowFatalFunction; }
  int isFatalFunction() const { return m_type == ThrowFatalFunction; }
  int isStaticCompact() const { return m_type == StaticCompactFunction; }

  // define(<literal-string>, <scalar>);
  bool isSimpleDefine(StringData **name, TypedValue *value) const;
  virtual TypePtr inferAndCheck(AnalysisResultPtr ar, TypePtr type,
                                bool coerce);

  virtual int getLocalEffects() const;

  // implementing IParseHandler
  virtual void onParse(AnalysisResultConstPtr ar, FileScopePtr fs);

  virtual void beforeCheck(AnalysisResultPtr ar) {}

  void addDependencies(AnalysisResultPtr ar);
  void addLateDependencies(AnalysisResultConstPtr ar);
  void setSafeCall(int flag) { m_safe = flag; }
  void setSafeDefault(ExpressionPtr def) { m_safeDef = def; }
  virtual ConstructPtr getNthKid(int n) const;
  virtual void setNthKid(int n, ConstructPtr cp);
  static SimpleFunctionCallPtr GetFunctionCallForCallUserFunc(
    AnalysisResultConstPtr ar, SimpleFunctionCallPtr call, int testOnly,
    int firstParam, bool &error);
  bool preOutputCPP(CodeGenerator &cg, AnalysisResultPtr ar, int state);
  void setupScopes(AnalysisResultConstPtr ar);
  bool readsLocals() const;
  bool writesLocals() const;
  void updateVtFlags();
  void setLocalThis(const std::string &name) { m_localThis = name; }
  bool isCallToFunction(const char *name) const;
  bool isCompilerCallToFunction(const char *name) const;
protected:
  enum FunctionType {
    UnknownType,
    DefineFunction,
    CreateFunction,
    VariableArgumentFunction,
    ExtractFunction,
    CompactFunction,
    StaticCompactFunction, // compact() with statically known variable names
    ShellExecFunction,
    ConstantFunction,
    DefinedFunction,
    FunctionExistsFunction,
    ClassExistsFunction,
    InterfaceExistsFunction,
    UnserializeFunction,
    GetDefinedVarsFunction,
    FBCallUserFuncSafeFunction,
    ThrowFatalFunction,

    LastType, // marker, not a valid type
  };

  static std::map<std::string, int> FunctionTypeMap;
  int m_type;
  unsigned m_dynamicConstant : 1;
  unsigned m_builtinFunction : 1;
  unsigned m_noPrefix : 1;
  unsigned m_fromCompiler : 1;
  unsigned m_invokeFewArgsDecision : 1;
  unsigned m_dynamicInvoke : 1;
  unsigned m_transformed : 1;
  unsigned m_no_volatile_check : 1;

  int m_safe;
  ExpressionPtr m_safeDef;
  std::string m_lambda;

  void outputCPPParamOrderControlled(CodeGenerator &cg, AnalysisResultPtr ar);
  ExpressionPtr optimize(AnalysisResultConstPtr ar);
private:
  int checkObjCall(AnalysisResultPtr ar);
  FunctionScopePtr
  getFuncScopeFromParams(AnalysisResultPtr ar,
                         BlockScopeRawPtr scope,
                         ExpressionPtr clsName,
                         ExpressionPtr funcName,
                         ClassScopePtr &clsScope);
  std::string getThisString(bool withArrow);
  std::string m_localThis;
  void *m_extra; // e.g., raw pointer to the symbol defined
};

SimpleFunctionCallPtr NewSimpleFunctionCall(
  EXPRESSION_CONSTRUCTOR_PARAMETERS,
  const std::string &name, ExpressionListPtr params,
  ExpressionPtr cls);

///////////////////////////////////////////////////////////////////////////////
}
#endif // __SIMPLE_FUNCTION_CALL_H__
