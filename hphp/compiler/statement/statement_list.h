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

#ifndef incl_HPHP_STATEMENT_LIST_H_
#define incl_HPHP_STATEMENT_LIST_H_

#include "hphp/compiler/statement/statement.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(StatementList);

class StatementList : public Statement {
public:
  explicit StatementList(STATEMENT_CONSTRUCTOR_PARAMETERS);
  StatementListPtr shallowClone();

  DECLARE_STATEMENT_VIRTUAL_FUNCTIONS;
  StatementPtr preOptimize(AnalysisResultConstPtr ar) override;
  bool hasDecl() const override;
  bool hasImpl() const override;
  ExpressionPtr getEffectiveImpl(AnalysisResultConstPtr ar) const;
  bool hasBody() const override;
  bool hasRetExp() const override;

  void addElement(StatementPtr stmt) override;
  void insertElement(StatementPtr stmt, int index = 0) override;
  int getRecursiveCount() const override {
    int ct = 0;
    for (StatementPtrVec::const_iterator it = m_stmts.begin();
         it != m_stmts.end(); ++it) {
      ct += (*it)->getRecursiveCount();
    }
    return ct;
  }
  void removeElement(int index);

  int getCount() const { return m_stmts.size();}
  StatementPtr operator[](int index);

  /**
   * This is for reordering out-of-order defaults.
   */
  void shift(int from, int to);

private:
  StatementPtrVec m_stmts;
  bool m_included; // whether includes have been inlined
};

///////////////////////////////////////////////////////////////////////////////
}
#endif // incl_HPHP_STATEMENT_LIST_H_
