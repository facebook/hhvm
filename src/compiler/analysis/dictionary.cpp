/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#include <compiler/analysis/alias_manager.h>
#include <compiler/analysis/dictionary.h>
#include <compiler/expression/expression.h>
#include <compiler/statement/statement.h>

using namespace HPHP;
using namespace boost;
using std::vector;

///////////////////////////////////////////////////////////////////////////////

Dictionary::Dictionary(AliasManager &am) : m_am(am) {
  m_idMap.push_back(ExpressionPtr());
}

void Dictionary::build(StatementPtr stmt) {
  for (int i = 0, n = stmt->getKidCount(); i < n; i++) {
    if (ConstructPtr kid = stmt->getNthKid(i)) {
      if (StatementPtr s = boost::dynamic_pointer_cast<Statement>(kid)) {
        switch (s->getKindOf()) {
          case Statement::KindOfFunctionStatement:
          case Statement::KindOfMethodStatement:
          case Statement::KindOfClassStatement:
          case Statement::KindOfInterfaceStatement:
            continue;
          default:
            break;
        }
        build(s);
      } else {
        ExpressionPtr e = boost::dynamic_pointer_cast<Expression>(kid);
        build(e);
      }
    }
  }
}

void Dictionary::build(ExpressionPtr e) {
  for (int i = 0, n = e->getKidCount(); i < n; i++) {
    if (ExpressionPtr kid = e->getNthExpr(i)) {
      build(kid);
    }
  }
  visit(e);
}

void Dictionary::record(ExpressionPtr e) {
  e->setCanonID(m_idMap.size());
  m_idMap.push_back(e);
}

void Dictionary::beginBlock(ControlBlock *b) {
  ControlFlowGraph *g = m_am.graph();
  assert(g);

  m_width = g->bitWidth();
  m_altered = b->getRow(DataFlow::Altered);
  m_available = g->rowExists(DataFlow::Available) ?
    b->getRow(DataFlow::Available) : 0;
  m_anticipated = g->rowExists(DataFlow::Anticipated) ?
    b->getRow(DataFlow::Anticipated) : 0;
}

