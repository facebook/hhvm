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

#ifndef __DO_STATEMENT_H__
#define __DO_STATEMENT_H__

#include <compiler/statement/statement.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(DoStatement);

class DoStatement : public Statement {
public:
  DoStatement(STATEMENT_CONSTRUCTOR_PARAMETERS,
              StatementPtr stmt, ExpressionPtr condition);

  DECLARE_STATEMENT_VIRTUAL_FUNCTIONS;
  virtual bool hasDecl() const { return m_stmt && m_stmt->hasDecl(); }
  virtual bool hasRetExp() const { return m_stmt && m_stmt->hasRetExp(); }
  virtual int getRecursiveCount() const {
    return 1 + (m_stmt ? m_stmt->getRecursiveCount() : 0);
  }

private:
  StatementPtr m_stmt;
  ExpressionPtr m_condition;
};

///////////////////////////////////////////////////////////////////////////////
}
#endif // __DO_STATEMENT_H__
