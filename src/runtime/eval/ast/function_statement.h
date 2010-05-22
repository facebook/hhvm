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

#ifndef __EVAL_AST_FUNCTION_STATEMENT_H__
#define __EVAL_AST_FUNCTION_STATEMENT_H__

#include <runtime/eval/base/function.h>
#include <runtime/eval/ast/statement.h>
#include <runtime/base/util/request_local.h>
#include <runtime/base/class_info.h>

#include <runtime/eval/analysis/block.h>

namespace HPHP {
namespace Eval {
///////////////////////////////////////////////////////////////////////////////

DECLARE_AST_PTR(StatementListStatement);
DECLARE_AST_PTR(Parameter);
DECLARE_AST_PTR(FunctionStatement);
DECLARE_AST_PTR(Expression);
DECLARE_AST_PTR(Name);
DECLARE_AST_PTR(StaticStatement);
class FunctionCallExpression;
class FuncScopeVariableEnvironment;

class Parameter : public Construct {
public:
  Parameter(CONSTRUCT_ARGS, const std::string &type, const std::string &name,
            int idx, bool ref, ExpressionPtr defVal, int argNum);
  bool isRef() const { return m_ref; }
  void bind(VariableEnvironment &env, CVarRef val, bool ref = false) const;
  void bindDefault(VariableEnvironment &env) const;
  virtual void dump() const;
  void getInfo(ClassInfo::ParameterInfo &info, VariableEnvironment &env) const;
  bool isOptional() const;
  void dropDefault();
  int argNum() const { return m_argNum; }
private:
  std::string m_type;
  NamePtr m_name;
  ExpressionPtr m_defVal;
  const char *m_fnName;
  int m_idx;
  DataType m_kind;
  int m_argNum;
  bool m_ref;
  bool m_nullDefault;
};

class FunctionStatement : public Statement, public Block, public Function {
public:
  FunctionStatement(STATEMENT_ARGS, const std::string &name,
                    const std::string &doc);
  ~FunctionStatement();
  void init(bool ref, const std::vector<ParameterPtr> params,
            StatementListStatementPtr body, bool has_call_to_get_args);
  const std::string &name() const { return m_name; }
  const std::string &lname() const { return m_lname; }
  void changeName(const std::string &name);
  // Eval is called at declaration, not invocation
  virtual void eval(VariableEnvironment &env) const;
  Variant invoke(CArrRef params) const;
  // Direct invoke is faster and gives access to the caller and its env
  Variant directInvoke(VariableEnvironment &env,
                       const FunctionCallExpression *caller) const;
  Variant invokeImpl(VariableEnvironment &env, CArrRef params) const;
  virtual LVariableTable *getStaticVars(VariableEnvironment &env) const;
  virtual void dump() const;
  void getInfo(ClassInfo::MethodInfo &info) const;
  bool refReturn() const { return m_ref; }
  const std::vector<ParameterPtr>& getParams() const { return m_params; }
protected:
  bool m_ref;
  std::string m_name;
  std::string m_lname;
  std::vector<ParameterPtr> m_params;

  StatementListStatementPtr m_body;
  bool m_hasCallToGetArgs;

  std::string m_docComment;

  virtual const std::string &fullName() const;
  void directBind(VariableEnvironment &env,
                  const FunctionCallExpression *caller,
                  FuncScopeVariableEnvironment &fenv) const;
  Variant evalBody(VariableEnvironment &env) const;
};

///////////////////////////////////////////////////////////////////////////////
}
}

#endif /* __EVAL_AST_FUNCTION_STATEMENT_H__ */
