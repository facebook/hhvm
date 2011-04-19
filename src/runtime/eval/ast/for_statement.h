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

#ifndef __EVAL_FOR_STATEMENT_H__
#define __EVAL_FOR_STATEMENT_H__

#include <runtime/eval/ast/statement.h>

namespace HPHP {
namespace Eval {
///////////////////////////////////////////////////////////////////////////////

DECLARE_AST_PTR(ForStatement);
DECLARE_AST_PTR(Expression);

class ForStatement : public Statement {
public:
  ForStatement(STATEMENT_ARGS,
               const std::vector<ExpressionPtr> &init,
               const std::vector<ExpressionPtr> &cond,
               const std::vector<ExpressionPtr> &next,
               StatementPtr body);
  virtual void eval(VariableEnvironment &env) const;
  virtual void dump(std::ostream &out) const;
private:
  std::vector<ExpressionPtr> m_init;
  std::vector<ExpressionPtr> m_cond;
  std::vector<ExpressionPtr> m_next;
  StatementPtr m_body;
};

///////////////////////////////////////////////////////////////////////////////
}
}

#endif /* __EVAL_FOR_STATEMENT_H__ */
