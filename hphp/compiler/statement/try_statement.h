/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_TRY_STATEMENT_H_
#define incl_HPHP_TRY_STATEMENT_H_

#include "hphp/compiler/statement/statement.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(StatementList);
DECLARE_BOOST_TYPES(TryStatement);

class TryStatement : public Statement {
public:
  
  TryStatement(STATEMENT_CONSTRUCTOR_PARAMETERS,
               StatementPtr tryStmt, StatementListPtr catches);

  TryStatement(STATEMENT_CONSTRUCTOR_PARAMETERS,
               StatementPtr tryStmt, StatementListPtr catches,
               StatementPtr finallyStmt);


  DECLARE_STATEMENT_VIRTUAL_FUNCTIONS;
  virtual bool hasDecl() const;
  virtual bool hasRetExp() const;
  virtual int getRecursiveCount() const;

  StatementPtr getBody() const { return m_tryStmt; }
  StatementListPtr getCatches() const { return m_catches; }
  StatementPtr getFinally() const { return m_finallyStmt; }
private:
  StatementPtr m_tryStmt;
  StatementListPtr m_catches;
  StatementPtr m_finallyStmt;
};

///////////////////////////////////////////////////////////////////////////////
}
#endif // incl_HPHP_TRY_STATEMENT_H_
