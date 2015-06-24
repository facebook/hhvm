/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_IF_BRANCH_STATEMENT_H_
#define incl_HPHP_IF_BRANCH_STATEMENT_H_

#include "hphp/compiler/statement/statement.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(IfBranchStatement);

class IfBranchStatement : public Statement {
public:
  IfBranchStatement(STATEMENT_CONSTRUCTOR_PARAMETERS,
                    ExpressionPtr condition, StatementPtr stmt);

  DECLARE_STATEMENT_VIRTUAL_FUNCTIONS;
  bool hasDecl() const override { return m_stmt && m_stmt->hasDecl(); }
  bool hasRetExp() const override { return m_stmt && m_stmt->hasRetExp(); }
  int getRecursiveCount() const override {
    return m_stmt ? m_stmt->getRecursiveCount() : 0;
  }
  ExpressionPtr getCondition() { return m_condition; }
  StatementPtr getStmt() { return m_stmt; }
  void clearStmt() { m_stmt.reset(); }
private:
  ExpressionPtr m_condition;
  StatementPtr m_stmt;
};

///////////////////////////////////////////////////////////////////////////////
}
#endif // incl_HPHP_IF_BRANCH_STATEMENT_H_
