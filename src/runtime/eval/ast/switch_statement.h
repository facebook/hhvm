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

#ifndef __EVAL_SWITCH_STATEMENT_H__
#define __EVAL_SWITCH_STATEMENT_H__

#include <runtime/eval/ast/statement.h>

namespace HPHP {
namespace Eval {
///////////////////////////////////////////////////////////////////////////////

DECLARE_AST_PTR(CaseStatement);
DECLARE_AST_PTR(SwitchStatement);
DECLARE_AST_PTR(Expression);

/**
 * Not sure if this should really be a statement
 */
class CaseStatement : public Statement {
public:
  CaseStatement(STATEMENT_ARGS, ExpressionPtr match, StatementPtr body);
  bool match(VariableEnvironment &env, CVarRef value) const;
  virtual void eval(VariableEnvironment &env) const;
  bool isDefault() const;
  virtual void dump() const;
private:
  ExpressionPtr m_match;
  StatementPtr m_body;
};

class SwitchStatement : public Statement {
public:
  SwitchStatement(STATEMENT_ARGS, ExpressionPtr source,
                  const std::vector<CaseStatementPtr> &cases);
  virtual void eval(VariableEnvironment &env) const;
  virtual void dump() const;
private:
  ExpressionPtr m_source;
  std::vector<CaseStatementPtr> m_cases;
};

///////////////////////////////////////////////////////////////////////////////
}
}

#endif /* __EVAL_SWITCH_STATEMENT_H__ */
