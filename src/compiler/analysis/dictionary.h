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

#ifndef __DICTIONARY_H__
#define __DICTIONARY_H__

#include <compiler/hphp.h>
#include <compiler/analysis/data_flow.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class AliasManager;
DECLARE_BOOST_TYPES(Statement);
DECLARE_BOOST_TYPES(Expression);
DECLARE_BOOST_TYPES(MethodStatement);

class Dictionary {
public:
  typedef std::vector<ExpressionPtr> IdMap;

  Dictionary(AliasManager &am);
  void build(StatementPtr s);
  void build(ExpressionPtr s);
  virtual void visit(ExpressionPtr e) = 0;

  void record(ExpressionPtr e);
  int size() const { return m_idMap.size(); }
  ExpressionPtr get(int id) const { return m_idMap[id]; }
  AliasManager &am() { return m_am; }
protected:
  AliasManager &m_am;
private:
  IdMap m_idMap;
};

class ExprDict : public Dictionary {
public:
  ExprDict(AliasManager &am) : Dictionary(am) {}
  /* Building the dictionary */
  void build(MethodStatementPtr m);
  void visit(ExpressionPtr e);

  /* Computing the attributes */
  void beginBlock(ControlBlock *b);
  void endBlock(ControlBlock *b);
  void updateAccess(ExpressionPtr e);

  /* Copy propagation */
  void beforePropagate(ControlBlock *b);
  ExpressionPtr propagate(ExpressionPtr e);
private:
  BitOps::Bits *m_altered;
  BitOps::Bits *m_available;
  BitOps::Bits *m_anticipated;
  size_t        m_width;
  std::vector<ExpressionRawPtr> m_avlExpr;
  std::vector<ExpressionRawPtr> m_avlAccess;
  ExpressionPtr m_active;
};

///////////////////////////////////////////////////////////////////////////////
}
#endif // __DICTIONARY_H__
