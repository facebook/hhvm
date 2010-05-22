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

#ifndef __CATCH_STATEMENT_H__
#define __CATCH_STATEMENT_H__

#include <compiler/statement/statement.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(CatchStatement);

class CatchStatement : public Statement {
public:
  CatchStatement(STATEMENT_CONSTRUCTOR_PARAMETERS,
                 const std::string &className, const std::string &variable,
                 StatementPtr stmt);

  DECLARE_STATEMENT_VIRTUAL_FUNCTIONS;
  virtual bool hasDecl() const { return m_stmt && m_stmt->hasDecl(); }
  virtual bool hasRetExp() const { return m_stmt && m_stmt->hasRetExp(); }
  virtual int getRecursiveCount() const {
    return (m_stmt ? m_stmt->getRecursiveCount() : 0);
  }
  const std::string &getVariable() const { return m_variable; }
private:
  std::string m_className;
  std::string m_variable;
  StatementPtr m_stmt;
  bool m_valid;
};

///////////////////////////////////////////////////////////////////////////////
}
#endif // __CATCH_STATEMENT_H__
