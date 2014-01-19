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

#ifndef incl_HPHP_CASE_STATEMENT_H_
#define incl_HPHP_CASE_STATEMENT_H_

#include "hphp/compiler/statement/statement.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(CaseStatement);

class CaseStatement : public Statement {
public:
  CaseStatement(STATEMENT_CONSTRUCTOR_PARAMETERS,
                ExpressionPtr condition, StatementPtr stmt);

  DECLARE_STATEMENT_VIRTUAL_FUNCTIONS;
  virtual bool hasDecl() const { return m_stmt && m_stmt->hasDecl(); }
  virtual bool hasRetExp() const { return m_stmt && m_stmt->hasRetExp(); }
  virtual int getRecursiveCount() const {
    return m_stmt ? m_stmt->getRecursiveCount() : 0;
  }
  void inferAndCheck(AnalysisResultPtr ar, TypePtr type, bool coerce);

  /**
   * Whether condition is a literal.
   */
  bool isLiteralInteger() const;
  bool isLiteralString() const;
  int64_t getLiteralInteger() const;
  std::string getLiteralString() const;

  bool getScalarConditionValue(Variant &v) const {
    if (!m_condition || !m_condition->getScalarValue(v)) return false;
    return true;
  }

  /**
   * SwitchStatement needs to inspect this expression.
   */
  ExpressionPtr getCondition() { return m_condition;}
  StatementPtr getStatement() { return m_stmt; }
private:
  ExpressionPtr m_condition;
  StatementPtr m_stmt;
};

///////////////////////////////////////////////////////////////////////////////
}
#endif // incl_HPHP_CASE_STATEMENT_H_
