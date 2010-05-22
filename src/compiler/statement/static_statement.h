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

#ifndef __STATIC_STATEMENT_H__
#define __STATIC_STATEMENT_H__

#include <compiler/statement/statement.h>
#include <compiler/expression/simple_variable.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(ExpressionList);
DECLARE_BOOST_TYPES(StaticStatement);

class StaticStatement : public Statement {
public:
  StaticStatement(STATEMENT_CONSTRUCTOR_PARAMETERS, ExpressionListPtr exp);

  DECLARE_STATEMENT_VIRTUAL_FUNCTIONS;

private:
  ExpressionListPtr m_exp;
};

///////////////////////////////////////////////////////////////////////////////
}
#endif // __STATIC_STATEMENT_H__
