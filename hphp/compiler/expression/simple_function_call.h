/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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
  void analyzeProgram(AnalysisResultConstRawPtr ar) override;
  void deepCopy(SimpleFunctionCallPtr exp);

  void setValid() { m_valid = true; }
  void setThrowFatal() { m_type = FunType::ThrowFatal; }
  void setThrowParseFatal() { m_type = FunType::ThrowParseFatal; }
  bool isParseFatalFunction() const {
    return m_type == FunType::ThrowParseFatal;
  }
  bool isFatalFunction() const {
    return isParseFatalFunction() ||  m_type == FunType::ThrowFatal;
  }

  // define(<literal-string>, <scalar>);
  bool isSimpleDefine(StringData **name, TypedValue *value) const;

  bool isScalar() const override;
  bool getScalarValue(Variant &value) override;

  // implementing IParseHandler
  void onParse(AnalysisResultConstRawPtr ar, FileScopePtr fs) override;

  void setSafeCall(int flag) { m_safe = flag; }
  void setSafeDefault(ExpressionPtr def) { m_safeDef = def; }
  ConstructPtr getNthKid(int n) const override;
  void setNthKid(int n, ConstructPtr cp) override;
  void setupScopes(AnalysisResultConstRawPtr ar);
  bool readsLocals() const;
  bool writesLocals() const;
  void updateVtFlags();
  void setLocalThis(const std::string& name) { m_localThis = name; }
  bool isCallToFunction(folly::StringPiece name) const;
  std::string getFullName() const;

protected:
  enum class FunType {
    Unknown,
    Define,
    Create,
    VariableArgument,
    Extract,
    Assert,
    Compact,
    ShellExec,
    Constant,
    Defined,
    FunctionExists,
    ClassExists,
    InterfaceExists,
    Unserialize,
    GetDefinedVars,
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
  unsigned m_optimizable : 1; // true if it can be morphed into a bytecode

  int m_safe;
  ExpressionPtr m_safeDef;
  std::string m_lambda;

private:
  std::string getThisString(bool withArrow);
  void mungeIfSpecialFunction(AnalysisResultConstRawPtr ar, FileScopePtr fs);

  std::string m_localThis;
};

SimpleFunctionCallPtr NewSimpleFunctionCall(
  EXPRESSION_CONSTRUCTOR_PARAMETERS,
  const std::string &name, bool hadBackslash, ExpressionListPtr params,
  ExpressionPtr cls);

///////////////////////////////////////////////////////////////////////////////
} // HPHP
#endif // incl_HPHP_SIMPLE_FUNCTION_CALL_H_
