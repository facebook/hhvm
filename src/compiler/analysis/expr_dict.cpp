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
//#include <compiler/analysis/control_flow.h>
#include <compiler/analysis/expr_dict.h>

#include <compiler/expression/expression.h>
#include <compiler/expression/assignment_expression.h>

#include <compiler/statement/statement.h>
#include <compiler/statement/method_statement.h>

using namespace HPHP;
using namespace boost;
using std::vector;

///////////////////////////////////////////////////////////////////////////////

void ExprDict::build(MethodStatementPtr m) {
  m_am.clear();
  Dictionary::build(m);
  static int rows[] =
    {
      DataFlow::Altered, DataFlow::Available, DataFlow::Anticipated,
      DataFlow::AvailIn, DataFlow::AvailOut,
      DataFlow::AntIn, DataFlow::AntOut
    };

  m_am.graph()->allocateDataFlow(size()+1,
                                 sizeof(rows)/sizeof(rows[0]), rows);
}

void ExprDict::visit(ExpressionPtr e) {
  if (m_am.insertForDict(e)) {
    record(e);
  }
}

void ExprDict::beginBlock(ControlBlock *b) {
  Dictionary::beginBlock(b);

  ExpressionPtr list;
  for (int i = size(); i--; ) {
    if (ExpressionPtr e = get(i)) {
      int cls = e->getExprClass();
      if ((cls & Expression::Update) ||
          (e->is(Expression::KindOfConstantExpression) && !e->isScalar())) {
        e->setCanonPtr(list);
        list = e;
      }
    }
  }

  m_active = list;

  if (m_available) {
    vector<ExpressionRawPtr>(size()).swap(m_avlExpr);
    vector<ExpressionRawPtr>().swap(m_avlAccess);
  }
}

void ExprDict::endBlock(ControlBlock *b) {
  if (m_available) {
    /*
      We optimistically set (almost) everything as
      available as we encounter it.
      Then we check the "accesses" one by one and mark
      them unavailable as necessary.
      Now we go through bottom up, removing non-available
      expressions, and updating m_available.
    */
    for (int ix = 0, n = m_avlExpr.size(); ix < n; ix++) {
      if (ExpressionPtr e = m_avlExpr[ix]) {
        if (e->isAvailable()) {
          int i = e->getKidCount();
          while (true) {
            if (!i--) {
              BitOps::set_bit(ix, m_available, true);
              break;
            }
            if (ExpressionPtr k = e->getNthExpr(i)) {
              if (!k->isAvailable()) {
                e->clearAvailable();
                break;
              }
            }
          }
        }
      }
    }
  }

  for (int ix = 0, n = size(); ix < n; ix++) {
    if (ExpressionPtr e = get(ix)) {
      if (!BitOps::get_bit(ix, m_altered)) {
        int i = e->getKidCount();
        while (i--) {
          if (ExpressionPtr k = e->getNthExpr(i)) {
            if (BitOps::get_bit(k->getCanonID(), m_altered)) {
              BitOps::set_bit(ix, m_altered, true);
              break;
            }
          }
        }
      }
    }
  }
}

void ExprDict::updateAccess(ExpressionPtr e) {
  int cls = e->getExprClass();
  int eid = e->getCanonID();
  e->clearAnticipated();
  e->clearAvailable();
  if (m_anticipated &&
      (cls & Expression::Update ?
       !BitOps::get_bit(eid, m_altered) : !e->getLocalEffects())) {
    /*
      Anticipated can be computed bottom up as we go. But note that we
      only know altered for Load/Store expressions.
    */
    int i = e->getKidCount();
    while (true) {
      if (!i--) {
        e->setAnticipated();
        if (!e->hasContext(Expression::AssignmentLHS)) {
          BitOps::set_bit(eid, m_anticipated, true);
        }
        break;
      }
      if (ExpressionPtr k = e->getNthExpr(i)) {
        if (!k->isAnticipated()) {
          break;
        }
      }
    }
  }

  if (m_available) {
    /*
      Available has to be computed optimistically, because we dont yet
      know what is going to be altered between here and the end of the block
      So keep a list of the potentially-available accesses (avlAccess), and
      for each id, the last potentially-available expression (avlExpr).
      For each modifying expression that we process, we remove expressions
      from avlAccess, and at the end, we build up the available expressions
      bottom up.
    */
    if ((cls & (Expression::Store|Expression::Call)) ||
        (cls & Expression::Load &&
         e->getContext() & (Expression::LValue|
                            Expression::RefValue|
                            Expression::UnsetContext|
                            Expression::DeepReference))) {
      bool isLoad;
      int depth = 0, effects = 0;
      for (int i = 0, n = m_avlAccess.size(); i < n; ) {
        ExpressionRawPtr a = m_avlAccess[i];
        if (m_am.checkAnyInterf(e, a, isLoad, depth, effects) !=
            AliasManager::DisjointAccess) {
          int aid = a->getCanonID();
          if (eid != aid || cls == Expression::Load) {
            BitOps::set_bit(aid, m_altered, true);
          }
          if (!(cls & Expression::Store) ||
              a != e->getNthExpr(0)) {
            a->clearAvailable();
            m_avlAccess[i] = m_avlAccess[--n];
            m_avlAccess.resize(n);
            continue;
          }
        }
        i++;
      }
    }
    if (cls & Expression::Update ||
        !e->getContainedEffects()) {
      int i = e->getKidCount();
      while (true) {
        if (!i--) {
          e->setAvailable();
          if (cls & Expression::Update) {
            m_avlAccess.push_back(e);
          }
          m_avlExpr[eid] = e;
          break;
        }
        if (ExpressionPtr k = e->getNthExpr(i)) {
          if (!k->isAvailable()) {
            break;
          }
        }
      }
    }
  }

  if ((cls & (Expression::Store|Expression::Call)) ||
      (cls & Expression::Load &&
       e->getContext() & (Expression::LValue|
                          Expression::RefValue|
                          Expression::UnsetContext|
                          Expression::DeepReference))) {
    ExpressionPtr cur = m_active, prev;
    bool isLoad;
    int depth = 0, effects = 0;
    while (cur) {
      ExpressionPtr next = cur->getCanonLVal();
      int cid = cur->getCanonID();
      if ((cid != eid || cls == Expression::Load) &&
          (BitOps::get_bit(cid, m_altered) ||
           m_am.checkAnyInterf(e, cur, isLoad, depth, effects) !=
           AliasManager::DisjointAccess)) {
        BitOps::set_bit(cid, m_altered, true);
        if (!prev) {
          m_active = next;
        } else {
          prev->setCanonPtr(next);
        }
      } else {
        prev = cur;
      }
      cur = next;
    }
  }
}

void ExprDict::beforePropagate(ControlBlock *b) {
  ControlFlowGraph *g = m_am.graph();
  assert(g);

  m_width = size();
  m_available = g->rowExists(DataFlow::AvailIn) ?
    b->getRow(DataFlow::AvailIn) : 0;
  m_anticipated = g->rowExists(DataFlow::AntIn) ?
    b->getRow(DataFlow::AntIn) : 0;

  vector<ExpressionRawPtr>(size()).swap(m_avlExpr);
  assert(m_available && m_anticipated);
  if (b->getDfn()) {
    for (size_t i = 1; i < m_width; i++) {
      if (!BitOps::get_bit(i, m_available)) continue;

      if (ExpressionPtr e = get(i)) {
        if (e->getKindOf() != Expression::KindOfAssignmentExpression) continue;
        AssignmentExpressionPtr ae(
          dynamic_pointer_cast<AssignmentExpression>(e));
        ExpressionPtr var(ae->getVariable());
        if (!BitOps::get_bit(var->getCanonID(), m_anticipated)) continue;
        ExpressionPtr val(ae->getValue());
        if (!val->isScalar()) continue;
        m_avlExpr[var->getCanonID()] = val;
      }
    }
  }
}

ExpressionPtr ExprDict::propagate(ExpressionPtr e) {
  return m_avlExpr[e->getCanonID()];
}
