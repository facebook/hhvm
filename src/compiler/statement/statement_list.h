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

#ifndef __STATEMENT_LIST_H__
#define __STATEMENT_LIST_H__

#include <compiler/statement/statement.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(StatementList);

class StatementList : public Statement {
public:
  StatementList(STATEMENT_CONSTRUCTOR_PARAMETERS);

  DECLARE_STATEMENT_VIRTUAL_FUNCTIONS;
  virtual bool hasDecl() const;
  virtual bool hasImpl() const;
  virtual bool hasRetExp() const;
  virtual void addElement(StatementPtr stmt);
  virtual void insertElement(StatementPtr stmt, int index = 0);
  virtual int getRecursiveCount() const {
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
  bool mergeConcatAssign(AnalysisResultPtr ar);

  StatementPtrVec m_stmts;
  bool m_included; // whether includes have been inlined
};

///////////////////////////////////////////////////////////////////////////////
}
#endif // __STATEMENT_LIST_H__
