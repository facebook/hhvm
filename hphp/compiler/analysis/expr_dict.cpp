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

#include "hphp/compiler/analysis/expr_dict.h"
#include "hphp/compiler/analysis/alias_manager.h"

#include "hphp/compiler/expression/expression.h"
#include "hphp/compiler/expression/assignment_expression.h"

#include "hphp/compiler/statement/statement.h"
#include "hphp/compiler/statement/method_statement.h"

using namespace HPHP;
using std::vector;

///////////////////////////////////////////////////////////////////////////////

ExprDict::ExprDict(AliasManager &am) : Dictionary(am) {
  // canon id 0 is reserved for a sentinel expr node
  m_canonIdMap.push_back(0);
  m_canonTypeMap.push_back(TypePtrIdxPair(TypePtr(), 0));
}

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

TypePtr ExprDict::extractTypeAssertion(ExpressionPtr e) const {
  assert(e);
  return e->isTypeAssertion() ? e->getAssertedType() : TypePtr();
}

void ExprDict::getTypes(ExpressionPtr e, TypePtrIdxPairVec &types) {
  return getTypes(e->getCanonID(), types);
}

class TypeFunc { public:
  bool operator()(const TypePtrIdxPair& entry) const {
    return entry.first != nullptr;
  }
};
static TypeFunc s_type_func;

void ExprDict::getTypes(int id, TypePtrIdxPairVec &types) {
  return getEntries(id, types, s_type_func);
}

class NoFilterFunc { public:
  bool operator()(const TypePtrIdxPair& entry) const {
    return true;
  }
};
static NoFilterFunc s_no_filter_func;

void ExprDict::getAllEntries(int id, TypePtrIdxPairVec &types) {
  return getEntries(id, types, s_no_filter_func);
}

template <class T>
void ExprDict::getEntries(int id, TypePtrIdxPairVec &entries, T func) {
  assert(m_canonIdMap.size() == m_canonTypeMap.size());
  assert(id >= 0 && id < (int) m_canonTypeMap.size());

  TypePtrIdxPair cur = m_canonTypeMap[id];
  if (func(cur)) entries.push_back(TypePtrIdxPair(cur.first, id));
  while (cur.second != id) {
    int id0 = cur.second;
    cur = m_canonTypeMap[id0];
    if (func(cur)) entries.push_back(TypePtrIdxPair(cur.first, id0));
  }
}

bool ExprDict::containsAssertion(TypePtr assertion,
                                 const TypePtrIdxPairVec &types,
                                 TypePtrIdxPair &entry) const {
  assert(assertion);
  for (TypePtrIdxPairVec::const_iterator it = types.begin();
       it != types.end(); ++it) {
    const TypePtrIdxPair &cur = *it;
    assert(cur.first);
    if (Type::SameType(cur.first, assertion)) {
      entry = cur;
      return true;
    }
  }
  return false;
}

void ExprDict::visit(ExpressionPtr e) {
  assert(m_canonIdMap.size() == m_canonTypeMap.size());
  if (m_am.insertForDict(e)) {
    // we've never seen e's structure before, so record it
    record(e);
    if (e->getCanonID() >= m_canonTypeMap.size()) {
      m_canonTypeMap.resize(e->getCanonID() + 1);
      m_canonIdMap.resize(e->getCanonID() + 1);
    }
    m_canonTypeMap[e->getCanonID()] =
      TypePtrIdxPair(extractTypeAssertion(e), e->getCanonID());
    m_canonIdMap[e->getCanonID()] = e->getCanonID();
  } else if (e->isTypeAssertion()) {
    TypePtrIdxPairVec types;
    assert(isCanonicalStructure(e->getCanonID()));
    getTypes(e->getCanonID(), types);
    TypePtrIdxPair entry;
    if (containsAssertion(e->getAssertedType(), types, entry)) {
      e->setCanonID(entry.second);
    } else {
      // new type assertion seen, record it
      int oldId = e->getCanonID();
      assert(isCanonicalStructure(oldId));
      record(e);
      // insert it into the list
      if (e->getCanonID() >= m_canonTypeMap.size()) {
        m_canonTypeMap.resize(e->getCanonID() + 1);
        m_canonIdMap.resize(e->getCanonID() + 1);
      }
      m_canonIdMap[e->getCanonID()] = oldId;
      TypePtrIdxPair &head = m_canonTypeMap[oldId];
      int oldSecond = head.second;
      head.second = e->getCanonID();
      m_canonTypeMap[e->getCanonID()] =
        TypePtrIdxPair(e->getAssertedType(), oldSecond);
    }
  }
}

void ExprDict::beginBlock(ControlBlock *b) {
  Dictionary::beginBlock(b);

  ExpressionPtr list;
  for (int i = size(); i--; ) {
    if (ExpressionPtr e = get(i)) {
      if (!isCanonicalStructure(i)) continue;
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
    vector<int>(size()).swap(m_avlTypeAsserts);
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
          assert(((int)e->getCanonID()) == ix);
          assert(isCanonicalStructure(ix));
          while (true) {
            if (!i--) {
              BitOps::set_bit(ix, m_available, true);
              break;
            }
            if (ExpressionPtr k = e->getNthExpr(i)) {
              if (!isCanonicalStructure(k->getCanonID())) continue;
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
    if (!isCanonicalStructure(ix)) continue;
    if (ExpressionPtr e = get(ix)) {
      if (!BitOps::get_bit(ix, m_altered)) {
        int i = e->getKidCount();
        while (i--) {
          if (ExpressionPtr k = e->getNthExpr(i)) {
            if (!isCanonicalStructure(k->getCanonID())) continue;
            if (BitOps::get_bit(k->getCanonID(), m_altered)) {
              setStructureOps(ix, m_altered, true);
              break;
            }
          }
        }
      }
    }
  }

  // for each type assertion seen in this control block,
  // if its structure has not been altered, make the type
  // assert available
  for (vector<int>::const_iterator it = m_avlTypeAsserts.begin();
       it != m_avlTypeAsserts.end(); ++it) {
    if (!BitOps::get_bit(id(*it), m_altered)) {
      BitOps::set_bit(*it, m_available, true);
    }
  }
}

void ExprDict::setStructureOps(int idx, BitOps::Bits *bits, bool flag) {
  assert(isCanonicalStructure(idx));
  TypePtrIdxPairVec entries;
  getAllEntries(idx, entries);
  for (TypePtrIdxPairVec::const_iterator it = entries.begin();
       it != entries.end(); ++it) {
    const TypePtrIdxPair &entry = *it;
    BitOps::set_bit(entry.second, bits, flag);
  }
}

void ExprDict::updateAccess(ExpressionPtr e) {
  int cls = e->getExprClass();
  int eid = e->getCanonID();
  e->clearAnticipated();
  e->clearAvailable();

  // bail on non-canonical expressions
  if (!isCanonicalStructure(eid)) {
    // but record we saw a type assertion belonging to this block
    m_avlTypeAsserts.push_back(eid);
    return;
  }

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
          setStructureOps(eid, m_anticipated, true);
        }
        break;
      }
      if (ExpressionPtr k = e->getNthExpr(i)) {
        if (!isCanonicalStructure(k->getCanonID())) continue;
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
          assert(isCanonicalStructure(aid));
          if (eid != aid || cls == Expression::Load) {
            BitOps::set_bit(aid, m_altered, true);
          }
          if (!(cls & Expression::Store) ||
              a != e->getStoreVariable()) {
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
          if (!isCanonicalStructure(k->getCanonID())) continue;
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
      assert(isCanonicalStructure(cid));
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

TypePtr ExprDict::reduceToSingleAssertion(const TypePtrIdxPairVec &types)
  const {
  assert(m_available);
  TypePtr ret;
  for (TypePtrIdxPairVec::const_iterator it = types.begin();
       it != types.end(); ++it) {
    assert(it->first);
    if (!BitOps::get_bit(it->second, m_available)) continue;
    if (!ret) ret = it->first;
    else if (!Type::SameType(ret, it->first)) {
      if (ret->is(Type::KindOfObject) &&
          it->first->is(Type::KindOfObject)) {
        // reconcile
        ret = Type::InferredObject(
            m_am.getAnalysisResult(), ret, it->first);
      } else {
        return TypePtr();
      }
    }
  }
  return ret;
}

void ExprDict::beforePropagate(ControlBlock *b) {
  ControlFlowGraph *g = m_am.graph();
  always_assert(g);

  m_width = size();
  m_available = g->rowExists(DataFlow::AvailIn) ?
    b->getRow(DataFlow::AvailIn) : 0;
  m_anticipated = g->rowExists(DataFlow::AntIn) ?
    b->getRow(DataFlow::AntIn) : 0;

  vector<ExpressionRawPtr>(size()).swap(m_avlExpr);
  vector<TypePtr>(size()).swap(m_avlTypes);
  always_assert(m_available && m_anticipated);
  if (b->getDfn()) {
    for (size_t i = 1; i < m_width; i++) {
      if (!BitOps::get_bit(i, m_available)) continue;
      if (!isCanonicalStructure(i))         continue;
      if (ExpressionPtr e = get(i)) {
        // setup for copy-prop
        if (e->is(Expression::KindOfAssignmentExpression)) {
          AssignmentExpressionPtr ae(
            static_pointer_cast<AssignmentExpression>(e));
          ExpressionPtr var(ae->getVariable());
          assert(isCanonicalStructure(var->getCanonID()));
          if (BitOps::get_bit(var->getCanonID(), m_anticipated)) {
            ExpressionPtr val(ae->getValue());
            if (val->isScalar()) {
              m_avlExpr[var->getCanonID()] = val;
            }
          }
        }
        // setup for type assertion prop
        TypePtrIdxPairVec types;
        getTypes(e->getCanonID(), types);
        TypePtr t = reduceToSingleAssertion(types);
        if (t) {
          m_avlTypes[e->getCanonID()] = t;
        }
      }
    }
  }
}

ExpressionPtr ExprDict::propagate(ExpressionPtr e) {
  return m_avlExpr[e->getCanonID()];
}

TypePtr ExprDict::propagateType(ExpressionPtr e) {
  return m_avlTypes[e->getCanonID()];
}
