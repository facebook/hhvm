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

#ifndef __EVAL_IF_STATEMENT_H__
#define __EVAL_IF_STATEMENT_H__

#include <runtime/eval/ast/statement.h>

namespace HPHP {
namespace Eval {
///////////////////////////////////////////////////////////////////////////////

DECLARE_AST_PTR(IfBranch);
DECLARE_AST_PTR(IfStatement);
DECLARE_AST_PTR(Expression);

class IfBranch : public Construct {
public:
  IfBranch(CONSTRUCT_ARGS, ExpressionPtr cond, StatementPtr body);
  bool proc(VariableEnvironment &env) const;
  virtual void dump(std::ostream &out) const;
  Variant evalCond(VariableEnvironment &env) const;
  const ExpressionPtr &cond() { return m_cond; }
  const StatementPtr &body() { return m_body; }
private:
  ExpressionPtr m_cond;
  StatementPtr m_body;
};

class IfStatement : public Statement {
public:
  IfStatement(STATEMENT_ARGS, const std::vector<IfBranchPtr> &branches,
              StatementPtr els);
  virtual void eval(VariableEnvironment &env) const;
  virtual void dump(std::ostream &out) const;
private:
  std::vector<IfBranchPtr> m_branches;
  StatementPtr m_else;
};

///////////////////////////////////////////////////////////////////////////////
}
}

#endif /* __EVAL_IF_STATEMENT_H__ */
