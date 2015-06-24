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

#ifndef incl_HPHP_BLOCK_STATEMENT_H_
#define incl_HPHP_BLOCK_STATEMENT_H_

#include "hphp/compiler/statement/statement.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(StatementList);
DECLARE_BOOST_TYPES(BlockStatement);

class BlockStatement : public Statement {
public:
  BlockStatement(STATEMENT_CONSTRUCTOR_PARAMETERS, StatementListPtr stmts);

  DECLARE_STATEMENT_VIRTUAL_FUNCTIONS;

  StatementListPtr getStmts() const { return m_stmts; }
  bool hasDecl() const override;
  bool hasImpl() const override;
  bool hasBody() const override;
  bool hasRetExp() const override;
  int getRecursiveCount() const override;
private:
  StatementListPtr m_stmts;
};

///////////////////////////////////////////////////////////////////////////////
}
#endif // incl_HPHP_BLOCK_STATEMENT_H_
