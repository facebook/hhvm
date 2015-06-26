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

#ifndef incl_HPHP_DEPTH_FIRST_VISITOR_H_
#define incl_HPHP_DEPTH_FIRST_VISITOR_H_

#include "hphp/compiler/analysis/analysis_result.h"
#include "hphp/compiler/analysis/ast_walker.h"
#include "hphp/compiler/analysis/block_scope.h"

#include "hphp/compiler/expression/expression.h"
#include "hphp/compiler/statement/statement.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct Pre        {};

template <typename When>
inline int GetPhaseInterestMask() {
  return BlockScope::UseKindAny;
}

template <typename When,
          template <typename> class T>
class DepthFirstVisitor {
public:
  DepthFirstVisitor(T<When> d) : m_data(d) {
    setup();
  }

  void setup() {}
  ExpressionPtr visitExprRecur(ExpressionPtr e) {
    for (int i = 0, n = e->getKidCount(); i < n; i++) {
      if (ExpressionPtr kid = e->getNthExpr(i)) {
        ExpressionPtr rep = visitExprRecur(kid);
        if (rep) {
          e->getScope()->addUpdates(BlockScope::UseKindCaller);
          e->setNthKid(i, rep);
        }
      }
    }
    return this->visit(e);
  }

  StatementPtr visitStmtRecur(StatementPtr stmt) {
    BlockScopeRawPtr scope = dynamic_pointer_cast<LoopStatement>(stmt) ?
      stmt->getScope() : BlockScopeRawPtr();

    for (int i = 0, n = stmt->getKidCount(); i < n; i++) {
      if (ConstructPtr kid = stmt->getNthKid(i)) {
        if (StatementPtr s = dynamic_pointer_cast<Statement>(kid)) {
          if (FunctionWalker::SkipRecurse(s)) continue;
          if (scope) scope->incLoopNestedLevel();
          if (StatementPtr rep = visitStmtRecur(s)) {
            stmt->setNthKid(i, rep);
            stmt->getScope()->addUpdates(BlockScope::UseKindCaller);
          }
          if (scope) scope->decLoopNestedLevel();
        } else {
          ExpressionPtr e = dynamic_pointer_cast<Expression>(kid);
          if (ExpressionPtr rep = visitExprRecur(e)) {
            stmt->setNthKid(i, rep);
            stmt->getScope()->addUpdates(BlockScope::UseKindCaller);
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

  bool activateScope(BlockScopeRawPtr scope) {
    if (scope->getMark() == BlockScope::MarkProcessed) {
      const BlockScopeRawPtrFlagsVec &ordered =
        scope->getOrderedUsers();
      for (BlockScopeRawPtrFlagsVec::const_iterator it = ordered.begin(),
           end = ordered.end(); it != end; ++it) {
        BlockScopeRawPtrFlagsVec::value_type pf = *it;
        if (pf->second & GetPhaseInterestMask<When>()) {
          int m = pf->first->getMark();
          if (m == BlockScope::MarkWaiting ||
              m == BlockScope::MarkReady) {
            pf->first->incNumDepsToWaitFor();
          }
        }
      }
    }

    const BlockScopeRawPtrFlagsPtrVec &deps = scope->getDeps();
    int numDeps = 0;
    for (BlockScopeRawPtrFlagsPtrVec::const_iterator it = deps.begin(),
         end = deps.end(); it != end; ++it) {
      const BlockScopeRawPtrFlagsPtrPair &p(*it);
      if (*p.second & GetPhaseInterestMask<When>()) {
        int m = p.first->getMark();
        if (m == BlockScope::MarkWaiting ||
            m == BlockScope::MarkReady ||
            m == BlockScope::MarkProcessing) {
          numDeps++;
        }
      }
    }

    scope->setNumDepsToWaitFor(numDeps);
    scope->setMark(BlockScope::MarkWaiting);
    if (!numDeps) {
      scope->setMark(BlockScope::MarkReady);
      return true;
    }
    return false;
  }

  void collectOrdering(BlockScopeRawPtrQueue &queue,
                       BlockScopeRawPtr      scope) {
    assert(scope->getMark() != BlockScope::MarkProcessingDeps);
    scope->setMark(BlockScope::MarkProcessingDeps);
    const BlockScopeRawPtrFlagsPtrVec &deps = scope->getDeps();
    for (BlockScopeRawPtrFlagsPtrVec::const_iterator it = deps.begin(),
         end = deps.end(); it != end; ++it) {
      const BlockScopeRawPtrFlagsPtrPair &p(*it);
      if (*p.second & GetPhaseInterestMask<When>()) {
        if (p.first->getMark() == BlockScope::MarkWaitingInQueue) {
          collectOrdering(queue, p.first);
        }
      }
    }
    queue.push_back(scope);
  }

  /**
   * Assumes that scopes contains no duplicate entries
   */
  bool visitParallel(const BlockScopeRawPtrQueue &scopes,
                     bool first, BlockScopeRawPtrQueue &enqueued) {
    bool ret = false;
    int numSetMarks = 0;
    enqueued.clear();
    for (BlockScopeRawPtrQueue::const_iterator it = scopes.begin();
         it != scopes.end(); ++it) {
      BlockScopeRawPtr scope = *it;
      if (!first) {
        if (scope->getMark() != BlockScope::MarkWaiting) {
          always_assert(scope->getMark() == BlockScope::MarkProcessed);
          continue;
        }
      } else {
        assert((*it)->getNumDepsToWaitFor() == 0);
      }
      scope->setMark(BlockScope::MarkWaitingInQueue);
      numSetMarks++;
      ret = true;
    }

    BlockScopeRawPtrQueue buffer;
    for (BlockScopeRawPtrQueue::const_iterator it = scopes.begin();
         it != scopes.end(); ++it) {
      if ((*it)->getMark() == BlockScope::MarkWaitingInQueue) {
        collectOrdering(buffer, *it);
      }
    }
    assert((int)buffer.size() == numSetMarks);

    {
      Lock l1(BlockScope::s_depsMutex);
      Lock l2(BlockScope::s_jobStateMutex);
      for (BlockScopeRawPtrQueue::const_iterator it = buffer.begin();
           it != buffer.end(); ++it) {
        if (activateScope(*it)) {
          enqueue(*it);
          enqueued.push_back(*it);
        }
      }
    }

    // assert that we will make some progress in this iteration
    assert(!ret || !enqueued.empty());
    return ret;
  }

  ExpressionPtr visit(ExpressionPtr) { return ExpressionPtr(); }
  StatementPtr visit(StatementPtr) { return StatementPtr(); }
  int visit(BlockScopeRawPtr scope) { return 0; }
  void enqueue(BlockScopeRawPtr scope) {}
  T<When> &data() { return m_data; }
private:
  T<When>     m_data;
};

///////////////////////////////////////////////////////////////////////////////
}
#endif // incl_HPHP_DEPTH_FIRST_VISITOR_H_
