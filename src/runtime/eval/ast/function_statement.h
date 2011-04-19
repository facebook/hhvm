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
  void clearIdx() { m_idx = -1;}
  int getIdx() const { return m_idx;}
  void setIdx(int idx) { m_idx = idx;}

  bool isRef() const { return m_ref; }
  void bind(VariableEnvironment &env, CVarRef val, bool ref = false) const;
  void bindDefault(VariableEnvironment &env) const;
  virtual void dump(std::ostream &out) const;
  void getInfo(ClassInfo::ParameterInfo &info, VariableEnvironment &env) const;
  bool isOptional() const;
  bool hasTypeHint() const { return !m_type.empty(); }
  void addNullDefault(void *parser);
  int argNum() const { return m_argNum; }
  const std::string &type() const { return m_type; }
  std::string name() const;
  String getName() const;

private:
  std::string m_type;
  NamePtr m_name;
  ExpressionPtr m_defVal;
  String m_fnName;
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
  void init(void *parser, bool ref, const std::vector<ParameterPtr> &params,
            StatementListStatementPtr body, bool has_call_to_get_args);
  String name() const { return m_name; }
  void changeName(const std::string &name);
  // Eval is called at declaration, not invocation
  virtual void eval(VariableEnvironment &env) const;
  Variant invoke(CArrRef params) const;
  // Direct invoke is faster and gives access to the caller and its env
  Variant directInvoke(VariableEnvironment &env,
                       const FunctionCallExpression *caller) const;
  Variant invokeClosure(CObjRef closure, VariableEnvironment &env,
                        const FunctionCallExpression *caller,
                        int start = 0) const;
  Variant invokeClosure(CArrRef params) const;
  Variant invokeImpl(FuncScopeVariableEnvironment &fenv, CArrRef params) const;
  virtual LVariableTable *getStaticVars(VariableEnvironment &env) const;
  virtual void dump(std::ostream &out) const;
  void getInfo(ClassInfo::MethodInfo &info) const;
  bool refReturn() const { return m_ref; }
  const std::vector<ParameterPtr>& getParams() const { return m_params; }
  bool hasBody() const { return m_body;}
  virtual const CallInfo *getCallInfo() const;
  virtual String fullName() const;

  void dumpHeader(std::ostream &out) const;
  void dumpBody(std::ostream &out) const;

  bool hasReturn() const { return m_yieldCount == -1;}
  bool hasYield() const { return m_yieldCount > 0;}
  void setHasReturn() { m_yieldCount = -1;}
  int addYield() { ASSERT(m_yieldCount >= 0); return ++m_yieldCount;}
  int getYieldCount() const { return m_yieldCount;}
  void setClosure(void *closure) { m_closure = closure;}

  void setName(const std::string &name) { m_name = AtomicString(name);}

protected:
  bool m_ref;
  AtomicString m_name;
  std::vector<ParameterPtr> m_params;

  StatementListStatementPtr m_body;
  bool m_hasCallToGetArgs;
  mutable char m_maybeIntercepted;
  int m_yieldCount;
  void *m_closure;

  std::string m_docComment;

  void directBind(VariableEnvironment &env,
                  const FunctionCallExpression *caller,
                  FuncScopeVariableEnvironment &fenv,
                  int start = 0) const;
  Variant evalBody(VariableEnvironment &env) const;
  CallInfo m_callInfo;
private:
  void bindParams(FuncScopeVariableEnvironment &fenv, CArrRef params) const;
  static Variant Invoker(void *ms, CArrRef params);
  static Variant InvokerFewArgs(void *ms, int count, INVOKE_FEW_ARGS_IMPL_ARGS);
};

///////////////////////////////////////////////////////////////////////////////
}
}

#endif /* __EVAL_AST_FUNCTION_STATEMENT_H__ */
