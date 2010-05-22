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

#ifndef __EVAL_AST_METHOD_STATEMENT_H__
#define __EVAL_AST_METHOD_STATEMENT_H__

#include <runtime/eval/ast/function_statement.h>

namespace HPHP {
namespace Eval {
///////////////////////////////////////////////////////////////////////////////

DECLARE_AST_PTR(MethodStatement);
class ClassStatement;

class MethodStatement : public FunctionStatement {
public:
  MethodStatement(STATEMENT_ARGS, const std::string &name,
                  const ClassStatement *cls, int modifiers,
                  const std::string &doc);
  // Eval is called at declaration, not invocation
  virtual void eval(VariableEnvironment &env) const;
  Variant invokeInstance(CObjRef obj, CArrRef params, bool check = true)
    const;
  Variant invokeStatic(const char* cls, CArrRef params, bool check = true)
    const;
  Variant invokeInstanceDirect(CObjRef obj, VariableEnvironment &env,
                               const FunctionCallExpression *caller) const;
  Variant invokeStaticDirect(const char* cls, VariableEnvironment &env,
                             const FunctionCallExpression *caller) const;
  void getInfo(ClassInfo::MethodInfo &info) const;
  virtual LVariableTable *getStaticVars(VariableEnvironment &env) const;
  const ClassStatement *getClass() const { return m_class; }
  int getModifiers() const { return m_modifiers; }
  void attemptAccess(const char *context) const;
  bool isAbstract() const;
private:
  const ClassStatement *m_class;
  int m_modifiers;
  std::string m_fullName;
  virtual const std::string &fullName() const;
};

///////////////////////////////////////////////////////////////////////////////
}
}

#endif /* __EVAL_AST_METHOD_STATEMENT_H__ */
