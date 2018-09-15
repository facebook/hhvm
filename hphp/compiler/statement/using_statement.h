/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#ifndef incl_HPHP_USING_STATEMENT_H_
#define incl_HPHP_USING_STATEMENT_H_

#include "hphp/compiler/expression/expression_list.h"
#include "hphp/compiler/statement/statement.h"
#include "hphp/compiler/statement/statement_list.h"

namespace HPHP {

DECLARE_BOOST_TYPES(UsingStatement);

struct UsingStatement : Statement {
  UsingStatement(
    STATEMENT_CONSTRUCTOR_PARAMETERS,
    bool isAsync, bool isWholeFunc, ExpressionListPtr exp, StatementListPtr body
  );

  DECLARE_STATEMENT_VIRTUAL_FUNCTIONS;

  bool getIsAsync() const            { return m_isAsync; }
  bool getIsWholeFunc() const        { return m_isWholeFunc; }
  ExpressionListPtr getExprs() const { return m_expr; }
  StatementListPtr getBody() const   { return m_body; }

private:
  const bool m_isAsync;
  const bool m_isWholeFunc;
  ExpressionListPtr m_expr;
  StatementListPtr m_body;
};

}

#endif
