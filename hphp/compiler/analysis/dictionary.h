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

#ifndef incl_HPHP_DICTIONARY_H_
#define incl_HPHP_DICTIONARY_H_

#include "hphp/compiler/hphp.h"
#include "hphp/compiler/analysis/data_flow.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class AliasManager;
DECLARE_BOOST_TYPES(Statement);
DECLARE_BOOST_TYPES(Expression);
DECLARE_BOOST_TYPES(MethodStatement);

class Dictionary {
public:
  typedef std::vector<ExpressionPtr> IdMap;

  explicit Dictionary(AliasManager &am);
  void build(MethodStatementPtr s);
  void build(StatementPtr s);
  void build(ExpressionPtr s);
  virtual void visit(ExpressionPtr e) = 0;

  void record(ExpressionPtr e);
  int size() const { return m_idMap.size(); }
  void resize(int s) { m_idMap.resize(s); }
  ExpressionPtr get(int id) const { return m_idMap[id]; }
  AliasManager &am() { return m_am; }

  void beginBlock(ControlBlock *b);
protected:
  AliasManager &m_am;
  BitOps::Bits *m_altered;
  BitOps::Bits *m_available;
  BitOps::Bits *m_anticipated;
  size_t        m_width;
private:
  IdMap m_idMap;
};

template <class Dict>
class AttributeTagger : public DataFlowWalker {
public:
  AttributeTagger(ControlFlowGraph *g, Dict &d) :
      DataFlowWalker(g), m_dict(d) {}

  void walk() { DataFlowWalker::walk(*this); }
  void processAccess(ExpressionPtr e) {
    m_dict.updateAccess(e);
  }

  void beforeBlock(ControlBlock *b) {
    m_dict.beginBlock(b);
  }
  void afterBlock(ControlBlock *b) {
    m_dict.endBlock(b);
  }
private:
  Dict &m_dict;
};

///////////////////////////////////////////////////////////////////////////////
}
#endif // incl_HPHP_DICTIONARY_H_
