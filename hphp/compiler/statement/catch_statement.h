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

#ifndef incl_HPHP_CATCH_STATEMENT_H_
#define incl_HPHP_CATCH_STATEMENT_H_

#include "hphp/compiler/statement/statement.h"
#include "hphp/compiler/expression/simple_variable.h"
#include "hphp/compiler/expression/static_class_name.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(CatchStatement);

class CatchStatement : public Statement, public StaticClassName {
public:
  CatchStatement(STATEMENT_CONSTRUCTOR_PARAMETERS,
                 const std::string &className, const std::string &variable,
                 StatementPtr stmt);

  CatchStatement(STATEMENT_CONSTRUCTOR_PARAMETERS,
                 const std::string &className, const std::string &variable,
                 StatementPtr stmt, StatementPtr finallyStmt);

  DECLARE_STATEMENT_VIRTUAL_FUNCTIONS;
  bool hasDecl() const override { return m_stmt && m_stmt->hasDecl(); }
  bool hasRetExp() const override { return m_stmt && m_stmt->hasRetExp(); }
  int getRecursiveCount() const override {
    return (m_stmt ? m_stmt->getRecursiveCount() : 0)
           + (m_finallyStmt ? m_finallyStmt->getRecursiveCount() : 0);
  }
  const std::string &getVariableName() const { return m_variable->getName(); }
  const std::string &getClassName(void*) const { return m_origClassName; }
  SimpleVariablePtr getVariable() const { return m_variable; }
  StatementPtr getStmt() const { return m_stmt; }
  StatementPtr getFinally() const { return m_finallyStmt; }
  void setStmt(StatementPtr s) { m_stmt = s; }
private:
  SimpleVariablePtr m_variable;
  StatementPtr m_stmt;
  StatementPtr m_finallyStmt;
  bool m_valid;
};

///////////////////////////////////////////////////////////////////////////////
}
#endif // incl_HPHP_CATCH_STATEMENT_H_
