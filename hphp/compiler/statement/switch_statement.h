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

#ifndef incl_HPHP_SWITCH_STATEMENT_H_
#define incl_HPHP_SWITCH_STATEMENT_H_

#include "hphp/compiler/statement/statement.h"
#include <map>
#include <memory>
#include <utility>
#include <vector>
#include "hphp/compiler/statement/case_statement.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(StatementList);
DECLARE_BOOST_TYPES(SwitchStatement);

class SwitchStatement : public Statement {
public:
  SwitchStatement(STATEMENT_CONSTRUCTOR_PARAMETERS,
                  ExpressionPtr exp, StatementListPtr cases);

  DECLARE_STATEMENT_VIRTUAL_FUNCTIONS;
  bool hasDecl() const override;
  bool hasRetExp() const override;
  int getRecursiveCount() const override;

  ExpressionPtr getExp() const { return m_exp; }
  StatementListPtr getCases() const { return m_cases; }
private:
  using StatementPtrWithPos = std::pair<int, CaseStatementPtr>;
  using StatementPtrWithPosVec = std::vector<StatementPtrWithPos>;
  using StatementPtrWithPosVecPtr = std::shared_ptr<StatementPtrWithPosVec>;
  using MapIntToStatementPtrWithPosVec =
    std::map<uint64_t, StatementPtrWithPosVecPtr>;
  ExpressionPtr m_exp;
  StatementListPtr m_cases;
};

///////////////////////////////////////////////////////////////////////////////
}
#endif // incl_HPHP_SWITCH_STATEMENT_H_
