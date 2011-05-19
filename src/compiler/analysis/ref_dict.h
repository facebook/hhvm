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

#ifndef __REF_DICT_H__
#define __REF_DICT_H__

#include <compiler/analysis/dictionary.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

//DECLARE_BOOST_TYPES(Type);

class RefDict : public Dictionary {
public:
  RefDict(AliasManager &am) : Dictionary(am), first_pass(true) {}

  /* Building the dictionary */
  void build(MethodStatementPtr m);
  void visit(ExpressionPtr e);

  /* Computing the attributes */
  void beginBlock(ControlBlock *b);
  void endBlock(ControlBlock *b);
  void updateParams();
  void updateAccess(ExpressionPtr e);

  void togglePass() { first_pass = !first_pass; }

private:

  bool first_pass;

  BitOps::Bits *m_referenced;
  BitOps::Bits *m_killed;
  BitOps::Bits *m_obj;
  BitOps::Bits *m_noobj;

  MethodStatementPtr m_method_stmt;
};

class RefDictWalker : public ControlFlowGraphWalker {
public:
  RefDictWalker(ControlFlowGraph *g) : 
    ControlFlowGraphWalker(g), first_pass(true) {}
  void walk() { ControlFlowGraphWalker::walk(*this); }
  int after(ConstructRawPtr cp);
  void togglePass() { first_pass = !first_pass; }
private:
  bool first_pass;
};

///////////////////////////////////////////////////////////////////////////////
}
#endif // __REF_DICT_H__
