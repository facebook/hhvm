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

#ifndef __SIMPLE_FUNCTION_CALL_H__
#define __SIMPLE_FUNCTION_CALL_H__

#include <compiler/expression/function_call.h>
#include <compiler/analysis/variable_table.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(SimpleFunctionCall);
class SimpleFunctionCall : public FunctionCall, public IParseHandler {
public:
  static void InitFunctionTypeMap();

public:
  SimpleFunctionCall(EXPRESSION_CONSTRUCTOR_PARAMETERS,
                     const std::string &name, ExpressionListPtr params,
                     ExpressionPtr cls);

  DECLARE_BASE_EXPRESSION_VIRTUAL_FUNCTIONS;
  bool isDefineWithoutImpl(AnalysisResultPtr ar);
  void setValid() { m_valid = true; }
  void setNoPrefix() { m_noPrefix = true; }
  void setArrayParams() { m_arrayParams = true; }

  virtual TypePtr inferAndCheck(AnalysisResultPtr ar, TypePtr type,
                                bool coerce);

  virtual int getLocalEffects() const;

  // implementing IParseHandler
  virtual void onParse(AnalysisResultPtr ar, BlockScopePtr scope);

  // extensible analysis by defining a subclass to be RealSimpleFunctionCall
  virtual void onAnalyzeInclude(AnalysisResultPtr ar) {}
  virtual ExpressionPtr onPreOptimize(AnalysisResultPtr ar) {
    return ExpressionPtr();
  }
  virtual void beforeCheck(AnalysisResultPtr ar) {}

  void addDependencies(AnalysisResultPtr ar);
  void addLateDependencies(AnalysisResultPtr ar);
  const std::string &getName() const { return m_name;}
  ExpressionListPtr getParams() const { return m_params; }
  void setSafeCall(int flag) { m_safe = flag; }
  void setSafeDefault(ExpressionPtr def) { m_safeDef = def; }
  virtual ConstructPtr getNthKid(int n) const;
  virtual void setNthKid(int n, ConstructPtr cp);
  static SimpleFunctionCallPtr getFunctionCallForCallUserFunc(
    AnalysisResultPtr ar, SimpleFunctionCallPtr call, bool testOnly,
    int firstParam, bool &error);
  bool preOutputCPP(CodeGenerator &cg, AnalysisResultPtr ar, int state);

protected:
  enum FunctionType {
    UnknownType,
    DefineFunction,
    CreateFunction,
    VariableArgumentFunction,
    ExtractFunction,
    CompactFunction,
    ShellExecFunction,
    ConstantFunction,
    DefinedFunction,
    FunctionExistsFunction,
    ClassExistsFunction,
    InterfaceExistsFunction,
    UnserializeFunction,
    GetDefinedVarsFunction,

    LastType, // marker, not a valid type
  };

  static std::map<std::string, int> FunctionTypeMap;
  int m_type;
  bool m_programSpecific;
  bool m_dynamicConstant;
  bool m_parentClass;
  std::string m_lambda;
  bool m_builtinFunction;
  bool m_noPrefix;

  void outputCPPParamOrderControlled(CodeGenerator &cg, AnalysisResultPtr ar);

  // only used for redeclared functions
  bool canInvokeFewArgs();
  bool m_invokeFewArgsDecision;
  bool m_dynamicInvoke;
  int m_safe;
  ExpressionPtr m_safeDef;
  bool m_arrayParams;

  ExpressionPtr optimize(AnalysisResultPtr ar);
};

///////////////////////////////////////////////////////////////////////////////
}
#endif // __SIMPLE_FUNCTION_CALL_H__
