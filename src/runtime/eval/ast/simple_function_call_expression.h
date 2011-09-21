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

#ifndef __EVAL_SIMPLE_FUNCTION_CALL_EXPRESSION_H__
#define __EVAL_SIMPLE_FUNCTION_CALL_EXPRESSION_H__

#include <runtime/eval/ast/function_call_expression.h>

namespace HPHP {
namespace Eval {
///////////////////////////////////////////////////////////////////////////////

DECLARE_AST_PTR(SimpleFunctionCallExpression);
DECLARE_AST_PTR(Name);
class Parser;
class Function;

class SimpleFunctionCallExpression : public FunctionCallExpression {
public:
  SimpleFunctionCallExpression(EXPRESSION_ARGS, NamePtr name,
                               const std::vector<ExpressionPtr> &params);
  SimpleFunctionCallExpression(NamePtr name,
                               const std::vector<ExpressionPtr> &params,
                               const Location *loc);
  virtual Variant eval(VariableEnvironment &env) const;
  virtual Expression *optimize(VariableEnvironment &env);
  virtual void dump(std::ostream &out) const;
  // Not quite sure if this is the right place
  static ExpressionPtr make(EXPRESSION_ARGS, NamePtr name,
                            const std::vector<ExpressionPtr> &params,
                            const Parser &p);
protected:
  NamePtr m_name;
  Variant evalCallInfo(
      const CallInfo *cit,
      void *extra,
      VariableEnvironment &env) const;
};

class BuiltinFunctionCallExpression : public SimpleFunctionCallExpression {
public:
  BuiltinFunctionCallExpression(NamePtr name,
                                const std::vector<ExpressionPtr> &params,
                                const CallInfo *callInfo, const Location *loc);
  virtual Variant eval(VariableEnvironment &env) const;
private:
  const CallInfo *m_callInfo;
};

class UserFunctionCallExpression : public SimpleFunctionCallExpression {
public:
  UserFunctionCallExpression(NamePtr name,
                             const std::vector<ExpressionPtr> &params,
                             const int userFuncId, const Location *loc);
  virtual Variant eval(VariableEnvironment &env) const;
private:
  const int m_userFuncId;
};

// see class EvalOverrides in runtime/eval/ext/ext.cpp for the list of
// special override functions (e.g., extract, func_get_arg). They require
// a VariableEnvironment which regular builtin functions do not.
class OverrideFunctionCallExpression : public SimpleFunctionCallExpression {
public:
  OverrideFunctionCallExpression(NamePtr name,
                                 const std::vector<ExpressionPtr> &params,
                                 const Function *override, const Location *loc);
  virtual Variant eval(VariableEnvironment &env) const;
private:
  const Function *m_override;
};

///////////////////////////////////////////////////////////////////////////////
}
}

#endif /* __EVAL_FUNCTION_CALL_EXPRESSION_H__ */
