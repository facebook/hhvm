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

#ifndef __DEPTH_FIRST_VISITOR_H__
#define __DEPTH_FIRST_VISITOR_H__

#include <compiler/analysis/block_scope.h>
#include <compiler/expression/expression.h>
#include <compiler/statement/statement.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

template <class T>
class DepthFirstVisitor {
public:
  DepthFirstVisitor(T d) : m_data(d), m_optCounter(0) {}

  ExpressionPtr visitExprRecur(ExpressionPtr e) {
    for (int i = 0, n = e->getKidCount(); i < n; i++) {
      if (ExpressionPtr kid = e->getNthExpr(i)) {
        ExpressionPtr rep = visitExprRecur(kid);
        if (rep) {
          m_optCounter++;
          e->setNthKid(i, rep);
        }
      }
    }
    return this->visit(e);
  }

  StatementPtr visitStmtRecur(StatementPtr stmt) {
    BlockScopeRawPtr scope = boost::dynamic_pointer_cast<LoopStatement>(stmt) ?
      stmt->getScope() : BlockScopeRawPtr();

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
          if (scope) scope->incLoopNestedLevel();
          if (StatementPtr rep = visitStmtRecur(s)) {
            stmt->setNthKid(i, rep);
            m_optCounter++;
          }
          if (scope) scope->decLoopNestedLevel();
        } else {
          ExpressionPtr e = boost::dynamic_pointer_cast<Expression>(kid);
          if (ExpressionPtr rep = visitExprRecur(e)) {
            stmt->setNthKid(i, rep);
            m_optCounter++;
          }
        }
      }
    }

    return this->visit(stmt);
  }

  int visitScope(BlockScopeRawPtr scope) {
    int flags = 0, f;
    do {
      flags |= f = this->visit(scope);
    } while (f);

    return flags;
  }

  void visitDependencies(BlockScopeRawPtr scope,
                         BlockScopeRawPtrQueue &queue) {
    scope->setMark(1);
    const BlockScopeRawPtrVec &deps = scope->getDeps();
    for (BlockScopeRawPtrVec::const_iterator it = deps.begin(),
           end = deps.end(); it != end; ++it) {
      BlockScopeRawPtr dep = *it;
      if (!dep->getMark()) {
        this->visitDependencies(dep, queue);
      }
    }
    if (int useKinds = this->visitScope(scope)) {
      scope->changed(queue, useKinds);
    } else if (BlockScopeRawPtrQueue *changed = scope->getChangedScopes()) {
      for (BlockScopeRawPtrQueue::iterator it = changed->begin(),
             end = changed->end(); it != end; ) {
        BlockScopeRawPtr bs = *it;
        changed->erase(it++);
        bs->changed(queue, bs->getUpdated() | BlockScope::UseKindAny);
        bs->clearUpdated();
      }
    }
    scope->setMark(2);
  }

  void visitDepthFirst(BlockScopeRawPtrQueue scopes) {
    BlockScopeRawPtrQueue::iterator end = scopes.end();
    for (BlockScopeRawPtrQueue::iterator it = scopes.begin(); it != end; ++it) {
      (*it)->setMark(0);
    }

    while (true) {
      BlockScopeRawPtrQueue::iterator it = scopes.begin();
      if (it == end) break;
      if (!(*it)->getMark()) {
        this->visitDependencies(*it, scopes);
      }
      assert((*it)->getMark() == 2);
      (*it)->setMark(3);
      scopes.erase(it);
    }
  }

  ExpressionPtr visit(ExpressionPtr);
  StatementPtr visit(StatementPtr);
  int visit(BlockScopeRawPtr scope);
private:
  T     m_data;
  int   m_optCounter;
};

///////////////////////////////////////////////////////////////////////////////
}
#endif // __DEPTH_FIRST_VISITOR_H__
