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

#ifndef incl_HPHP_USE_TRAIT_STATEMENT_H_
#define incl_HPHP_USE_TRAIT_STATEMENT_H_

#include "hphp/compiler/statement/statement.h"
#include "hphp/compiler/expression/expression_list.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(ExpressionList);
DECLARE_BOOST_TYPES(StatementList);
DECLARE_BOOST_TYPES(UseTraitStatement);

class UseTraitStatement : public Statement, public IParseHandler {
public:
  UseTraitStatement(STATEMENT_CONSTRUCTOR_PARAMETERS, ExpressionListPtr exp,
                   StatementListPtr stmt);

  DECLARE_STATEMENT_VIRTUAL_FUNCTIONS;
  ExpressionListPtr getExprs() const { return m_exp; }
  StatementListPtr  getStmts() const { return m_stmt;}

  void getUsedTraitNames(std::vector<std::string> &traitNames)
    const {
    traitNames.clear();
    m_exp->getStrings(traitNames);
  }

  // implementing IParseHandler
  virtual void onParseRecur(AnalysisResultConstPtr ar, ClassScopePtr scope);

private:
  ExpressionListPtr m_exp;  // used traits
  StatementListPtr m_stmt;  // trait rules
};

///////////////////////////////////////////////////////////////////////////////
}
#endif // incl_HPHP_USE_TRAIT_STATEMENT_H_
