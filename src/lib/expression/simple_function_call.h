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

#include <lib/expression/function_call.h>
#include <lib/analysis/variable_table.h>
#include <lib/hphp_unique.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////


DECLARE_BOOST_TYPES(SimpleFunctionCall);
class SimpleFunctionCall : public FunctionCall, public IParseHandler {
  friend class SimpleFunctionCallHook;
public:
  SimpleFunctionCall(EXPRESSION_CONSTRUCTOR_PARAMETERS,
                     const std::string &name, ExpressionListPtr params,
                     const std::string *className);

  ~SimpleFunctionCall();
  DECLARE_BASE_EXPRESSION_VIRTUAL_FUNCTIONS;
  bool isDefineWithoutImpl(AnalysisResultPtr ar);
  void setValid() { m_valid = true; }
  void setNoPrefix() { m_noPrefix = true; }

  virtual TypePtr inferAndCheck(AnalysisResultPtr ar, TypePtr type,
                                bool coerce);

  virtual bool hasEffect() const;

  // implementing IParseHandler
  virtual void onParse(AnalysisResultPtr ar);

  void *getHookData() { return m_hookData;}
  static void setHookHandler(void (*hookHandler)(AnalysisResultPtr ar,
                                                 SimpleFunctionCall *call,
                                                 HphpHookUniqueId id)) {
    m_hookHandler = hookHandler;
  }

  const std::string &getName() const { return m_name;}  
private:
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
  static void InitFunctionTypeMap();
  int m_type;
  bool m_programSpecific;
  bool m_dynamicConstant;
  bool m_parentClass;
  std::string m_lambda;
  bool m_builtinFunction;
  bool m_noPrefix;

  // hook
  static void (*m_hookHandler)(AnalysisResultPtr ar,
                               SimpleFunctionCall *call,
                               HphpHookUniqueId id);
  void *m_hookData;
};

///////////////////////////////////////////////////////////////////////////////
}
#endif // __SIMPLE_FUNCTION_CALL_H__
