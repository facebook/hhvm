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

#ifndef incl_HPHP_UNSET_STATEMENT_H_
#define incl_HPHP_UNSET_STATEMENT_H_

#include "hphp/compiler/statement/statement.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(ExpressionList);
DECLARE_BOOST_TYPES(UnsetStatement);

class UnsetStatement : public Statement {
public:
  UnsetStatement(STATEMENT_CONSTRUCTOR_PARAMETERS, ExpressionListPtr exp);

  DECLARE_STATEMENT_VIRTUAL_FUNCTIONS;
  StatementPtr preOptimize(AnalysisResultConstPtr ar);
  StatementPtr postOptimize(AnalysisResultConstPtr ar);

  ExpressionListPtr getExps() const { return m_exp; }
private:
  ExpressionListPtr m_exp;
};

///////////////////////////////////////////////////////////////////////////////
}
#endif // incl_HPHP_UNSET_STATEMENT_H_
