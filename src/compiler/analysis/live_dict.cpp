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

#include <compiler/analysis/alias_manager.h>
#include <compiler/analysis/live_dict.h>

#include <compiler/expression/expression.h>
#include <compiler/expression/assignment_expression.h>
#include <compiler/expression/simple_variable.h>

#include <compiler/statement/statement.h>
#include <compiler/statement/method_statement.h>

using namespace HPHP;
using namespace boost;
using std::vector;

///////////////////////////////////////////////////////////////////////////////

void LiveDict::build(MethodStatementPtr m) {
  m_coalesce = false;
  m_am.clear();
  m_getVars = true;
  Dictionary::build(m);
  m_getVars = false;
  int s = size();
  Dictionary::build(m);
  resize(s);

  static int rows[] =
    {
      DataFlow::Altered, DataFlow::Available,
      DataFlow::Anticipated, DataFlow::Dying, DataFlow::Used,
      DataFlow::PAvailIn, DataFlow::PAvailOut,
      DataFlow::PAntIn, DataFlow::PAntOut,
      DataFlow::PDieIn, DataFlow::PDieOut
    };
  m_am.graph()->allocateDataFlow(size() + 1,
                                 sizeof(rows)/sizeof(rows[0]), rows);
}

void LiveDict::visit(ExpressionPtr e) {
  if (m_coalesce) {
    if (e->is(Expression::KindOfSimpleVariable)) {
      int id = e->getCanonID();
      int rid = m_remap[id];
      if (rid && rid != id) {
        SimpleVariablePtr sv(static_pointer_cast<SimpleVariable>(e));
        SimpleVariablePtr rv(static_pointer_cast<SimpleVariable>(get(rid)));
        sv->coalesce(rv);
      }
    }
    return;
  }

  if (m_getVars != e->is(Expression::KindOfSimpleVariable)) return;

  if (m_am.insertForDict(e) && m_getVars) {
    record(e);
  }
}

void LiveDict::coalesce(MethodStatementPtr m) {
  m_coalesce = true;
  Dictionary::build(m);
  m_coalesce = false;
}

void LiveDict::updateParams() {
  ControlFlowGraph *g = m_am.graph();
  ControlBlock *b = g->getDfBlock(1);
  size_t width = g->bitWidth();

  BitOps::Bits *avlin = b->getRow(DataFlow::PAvailIn);
  BitOps::Bits *dieout = m_am.graph()->getTempBits(0);
  BitOps::set(width, dieout, 0);
  for (int i = size(); i--; ) {
    if (ExpressionPtr e = get(i)) {
      assert(e->is(Expression::KindOfSimpleVariable));
      Symbol *sym = static_pointer_cast<SimpleVariable>(e)->getSymbol();
      if (sym) {
        if (sym->isParameter()) {
          BitOps::set_bit(e->getCanonID(), avlin, true);
        }
        if (sym->isNeeded() || sym->isReferenced()) {
          BitOps::set_bit(e->getCanonID(), dieout, true);
        }
      }
    }
  }
  for (int i = g->getNumBlocks(); i; i--) {
    b = g->getDfBlock(i);
    if (!b->out_size()) {
      BitOps::bit_or(width, b->getRow(DataFlow::PDieOut),
                     b->getRow(DataFlow::PDieOut), dieout);
    }
  }
}

void LiveDict::beginBlock(ControlBlock *b) {
  Dictionary::beginBlock(b);
  m_dying = b->getRow(DataFlow::Dying);

  m_refs.reset();
  for (int i = size(); i--; ) {
    if (ExpressionPtr e = get(i)) {
      assert(e->is(Expression::KindOfSimpleVariable));
      SimpleVariablePtr sv(static_pointer_cast<SimpleVariable>(e));
      if (m_am.hasWildRefs() || sv->couldBeAliased()) {
        sv->setCanonPtr(m_refs);
        m_refs = sv;
      }
    }
  }
}

void LiveDict::endBlock(ControlBlock *b) {
}

void LiveDict::addConflicts(size_t width,
                            BitOps::Bits *live, BitOps::Bits *dying) {
  BitSetBlock b = m_conflicts.getBlock(0);
  for (int i = width; i--; ) {
    if (BitOps::get_bit(i, live)) {
      BitOps::Bits *row = b.getRow(i);
      BitOps::bit_or_or(width, row, row, live, dying);
    } else if (BitOps::get_bit(i, dying)) {
      BitOps::Bits *row = b.getRow(i);
      BitOps::bit_or(width, row, row, live);
    }
  }
}

void LiveDict::buildConflicts() {
  ControlFlowGraph &g = *m_am.graph();
  int num = g.getNumBlocks();
  size_t width = g.bitWidth();

  m_conflicts.alloc(1, width, -width, 0);

  BitOps::Bits *live = g.getTempBits(0);
  BitOps::Bits *dying = g.getTempBits(1);

  BitOps::Bits *params = g.getDfBlock(1)->getRow(DataFlow::PAvailIn);
  BitOps::set(width, dying, true);
  addConflicts(width, params, dying);

  for (int i = num; i ; i--) {
    ControlBlock *b = g.getDfBlock(i);
    BitOps::bit_and(width, live,
                    b->getRow(DataFlow::PAvailIn), b->getRow(DataFlow::PAntIn));
    BitOps::bit_and(width, dying,
                    b->getRow(DataFlow::PAvailIn), b->getRow(DataFlow::PDieIn));

    addConflicts(width, live, dying);
    BitOps::bit_or(width, dying, dying, live);
    addConflicts(width, b->getRow(DataFlow::Used), dying);
    BitOps::bit_and(width, live,
                    b->getRow(DataFlow::PAvailOut),
                    b->getRow(DataFlow::PAntOut));
    BitOps::bit_and(width, dying,
                    b->getRow(DataFlow::PAvailOut),
                    b->getRow(DataFlow::PDieOut));
    addConflicts(width, live, dying);
  }
}

/*
  The classical use/def isnt quite enough here. There are two unusual
  issues:
   - the ref/non-ref issue.
     An assignment to a var which is referenced doesnt end its
     lifetime. In fact it could just be a "use" of the var. But it
     also counts as a def.
   - the destructor issue.
     variables which might need to be destroyed later are technically
     alive, but dont interfere with any other variables in the same state.
     They /do/ interfere with any truly "live" variables, however.
     These are "dying".

   So we end up defining, use, kill, def and dying.

  use : a read of the variable, an ordinary assignment if it could
        be referenced

  kill : an unset, a ref assignment, or, for non referenced vars, any assignment

  def : ref or normal assignment

  dying : a variable whose destructor is (partially/locally) anticipated.
*/
void LiveDict::updateAccess(ExpressionPtr e) {
  int cls = e->getExprClass();
  if (cls & Expression::Store) {
    /*
      Handled when we see the lhs
    */
    return;
  }

  int eid = e->getCanonID();
  int context = e->getContext();
  bool unset = false;
  bool store = false;
  if (context & Expression::LValue && context & Expression::UnsetContext) {
    unset = true;
  } else if (context & Expression::AssignmentLHS) {
    store = true;
  }

  if (e->is(Expression::KindOfSimpleVariable)) {
    SimpleVariablePtr sv(static_pointer_cast<SimpleVariable>(e));
    bool use = false, kill = false, def = false;
    Symbol *sym = sv->getSymbol();
    if (unset) {
      kill = true;
    } else if (store) {
      if (context & Expression::RefAssignmentLHS ||
          (!m_am.hasWildRefs() && sym && !sym->isReferenced())) {
        kill = true;
      }
      def = true;
    } else if ((context & Expression::Declaration) == Expression::Declaration) {
      // a global declaration
      def = kill = true;
    } else if (context & (Expression::LValue|
                          Expression::RefValue|
                          Expression::DeepReference|
                          Expression::UnsetContext|
                          Expression::OprLValue)) {
      use = def = true;
    } else {
      use = true;
    }
    if (kill && (!sym || sym->isNeeded() || sym->isReferenced()) &&
        !BitOps::get_bit(eid, m_altered) &&
        !BitOps::get_bit(eid, m_available)) {
      BitOps::set_bit(eid, m_dying, true);
    }
    if (use &&
        !BitOps::get_bit(eid, m_altered) &&
        !BitOps::get_bit(eid, m_available)) {
      BitOps::set_bit(eid, m_anticipated, true);
      e->setAnticipated();
    }
    if (kill) {
      BitOps::set_bit(eid, m_altered, true);
      BitOps::set_bit(eid, m_available, def);
    } else if (def) {
      BitOps::set_bit(eid, m_available, true);
    }

    if (!m_am.couldBeAliased(sv)) {
      return;
    }
  } else if (!e->is(Expression::KindOfDynamicVariable) &&
             (unset || (context & Expression::RefAssignmentLHS))) {
    // An unset, or a reference assignment to anything other
    // than a simple or dynamic variable can never affect a simple
    // variable (outside of pseudoMain).
    return;
  }

  if (store || cls & (Expression::Load|Expression::Call)) {
    bool mod =
      store ||
      (cls & Expression::Load &&
       e->getContext() & (Expression::LValue|
                          Expression::RefValue|
                          Expression::UnsetContext|
                          Expression::DeepReference|
                          Expression::OprLValue));

    ExpressionPtr cur = m_refs, prev;
    bool isLoad;
    int depth = 0, effects = 0;
    while (cur) {
      ExpressionPtr next = cur->getCanonLVal();
      int cid = cur->getCanonID();
      if (cid != eid &&
          m_am.checkAnyInterf(e, cur, isLoad, depth, effects) !=
          AliasManager::DisjointAccess) {
        if (mod) {
          BitOps::set_bit(cid, m_available, true);
        }
        if (!BitOps::get_bit(cid, m_altered) &&
            !BitOps::get_bit(cid, m_available)) {
          BitOps::set_bit(cid, m_anticipated, true);
        }
        if (!prev) {
          m_refs = next;
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

struct Colorizer {
  Colorizer(int w) : toNode(w) {}

  struct NodeInfo {
    NodeInfo(int index) : originalIndex(index), size(0), color(-1) {}
    int originalIndex;
    int size;
    int color;
    std::set<int> conflicts;
  };

  typedef std::vector<NodeInfo>::iterator NodeIterator;
  NodeIterator begin() { return nodes.begin(); }
  NodeIterator end() { return nodes.end(); }

  std::vector<int> toNode; // map canonId to node index
  std::vector<NodeInfo> nodes;
  void addNode(int i) {
    nodes.push_back(NodeInfo(i));
    toNode[i] = nodes.size();
  }

  void addConflict(NodeInfo &ni, int i) {
    int ix = toNode[i];
    if (ix) {
      std::pair<std::set<int>::iterator, bool> ret =
        ni.conflicts.insert(ix-1);
      assert(ret.second);
      ni.size++;
    }
  }

  class NodeCmp {
  public:
    NodeCmp(const Colorizer *c) : m_c(c) {}
    bool operator()(int a, int b) {
      const NodeInfo &n1 = m_c->nodes[a];
      const NodeInfo &n2 = m_c->nodes[b];
      return n1.size < n2.size;
    }
  private:
    const Colorizer *m_c;
  };

  void sort(bool tryHarder, BitOps::Bits *tmp) {
    // First, sort on increasing size
    int size = nodes.size();
    std::vector<int> sorted(size);
    for (int i = size; i--; sorted[i] = i)
      ;
    std::sort(sorted.begin(), sorted.end(), NodeCmp(this));
    if (tryHarder) {
      /*
        We have sorted the nodes by number of conflicts. Coloring
        them from most conflicts to fewest should produce reasonable
        results. But we can do better with a dynamic order.
        Repeatedly remove a node with fewest conflicts from the graph
        until there are none left, then color them in the reverse order
      */
      std::vector<int> isorted(size);
      std::vector<int> buckets;

      for (int i = 0, j = -1; i < size; i++) {
        isorted[sorted[i]] = i;
        const NodeInfo &n = nodes[sorted[i]];
        while (j < n.size) {
          buckets.push_back(i);
          j++;
        }
      }
      buckets.push_back(size);

      // Repeatedly move the next node into bucket zero,
      // decrementing the sizes of all its not-yet-chosen
      // conflicts, and adjusting their position in
      // the list accordingly
      for (int i = 0; i < size; i++) {
        NodeInfo &node = nodes[sorted[i]];
        int bucket = node.size;
        for (int j = 0; j <= bucket; j++) {
          assert(buckets[j] == i);
          buckets[j]++;
        }
        for (std::set<int>::iterator it = node.conflicts.begin(),
               end = node.conflicts.end(); it != end; ++it) {
          int ix = *it;
          int si = isorted[ix];
          if (si > i) {
            NodeInfo &n = nodes[ix];
            int &bi = buckets[n.size];
            if (si != bi) {
              assert(si > bi);
              sorted[si] = sorted[bi];
              sorted[bi] = ix;
              isorted[sorted[si]] = si;
              isorted[ix] = bi;
            }
            bi++;
            n.size--;
          }
        }
      }
    }

    // now choose the colors
    for (int i = size; i--; ) {
      NodeInfo &n = nodes[sorted[i]];
      BitOps::set(size, tmp, 0);
      for (std::set<int>::iterator it = n.conflicts.begin(),
             end = n.conflicts.end(); it != end; ++it) {
        NodeInfo &n2 = nodes[*it];
        if (n2.color >= 0) BitOps::set_bit(n2.color, tmp, true);
      }
      int j = 0;
      while (BitOps::get_bit(j, tmp)) {
        j++;
        assert(j < size);
      }
      n.color = j;
    }
  }
};

bool LiveDict::color(TypePtr type) {
  ControlFlowGraph &g = *m_am.graph();
  size_t width = g.bitWidth();

  Colorizer col(width);

  BitSetBlock b = m_conflicts.getBlock(0);

  for (int i = size(); i--; ) {
    if (ExpressionPtr e = get(i)) {
      if (Type::SameType(type, e->getCPPType())) {
        Symbol *sym = static_pointer_cast<SimpleVariable>(e)->getSymbol();
        if (sym &&
            !sym->isGlobal() &&
            !sym->isParameter() &&
            !sym->isStatic()) {
          col.addNode(i);
        }
      }
    }
  }

  for (Colorizer::NodeIterator it = col.begin(), end = col.end();
       it != end; ++it) {
    Colorizer::NodeInfo &ni = *it;
    BitOps::Bits *row = b.getRow(ni.originalIndex);
    for (int i = width; i--; ) {
      if (i != ni.originalIndex && BitOps::get_bit(i, row)) {
        col.addConflict(ni, i);
      }
    }
  }

  BitOps::Bits *tmp = g.getTempBits(0);
  col.sort(true, tmp);

  std::map<int,int> cmap;

  bool doit = false;
  for (Colorizer::NodeIterator it = col.begin(), end = col.end();
       it != end; ++it) {
    Colorizer::NodeInfo &ni = *it;
    int &ix = cmap[ni.color];
    if (!ix) {
      ix = ni.originalIndex;
    } else {
      doit = true;
    }
    m_remap[ni.originalIndex] = ix;
  }
  return doit;
}
