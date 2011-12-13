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

#ifndef __EVAL_USE_TRAIT_STATEMENT_H__
#define __EVAL_USE_TRAIT_STATEMENT_H__

#include <runtime/eval/ast/expression.h>
#include <runtime/eval/ast/name.h>
#include <runtime/eval/ast/statement.h>
#include <runtime/eval/ast/statement_list_statement.h>

namespace HPHP {
namespace Eval {
///////////////////////////////////////////////////////////////////////////////

DECLARE_AST_PTR(UseTraitStatement);

class UseTraitStatement : public Statement {
public:
  UseTraitStatement(STATEMENT_ARGS, const std::vector<NamePtr> &names,
                    StatementListStatementPtr stmts);
  const std::vector<NamePtr> &getNames() const { return m_names; }
  const StatementListStatementPtr &getStmts() const { return m_stmts; }

  virtual void eval(VariableEnvironment &env) const;
  virtual void dump(std::ostream &out) const;
private:
  std::vector<NamePtr> m_names;
  StatementListStatementPtr m_stmts;
};

///////////////////////////////////////////////////////////////////////////////
}
}

#endif /* __EVAL_USE_TRAIT_STATEMENT_H__ */
