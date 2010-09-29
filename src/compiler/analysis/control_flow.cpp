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

#include "compiler/analysis/ast_walker.h"
#include "compiler/analysis/control_flow.h"
#include "compiler/expression/expression.h"
#include "compiler/statement/statement.h"
#include "compiler/statement/method_statement.h"
#include "compiler/statement/statement_list.h"
#include "compiler/statement/if_branch_statement.h"
#include "compiler/statement/for_statement.h"
#include "compiler/statement/while_statement.h"
#include "compiler/statement/do_statement.h"
#include "compiler/statement/foreach_statement.h"
#include "compiler/statement/switch_statement.h"
#include "compiler/statement/break_statement.h"
#include "compiler/statement/try_statement.h"

#include <boost/graph/depth_first_search.hpp>

using namespace std;
using namespace boost;

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

enum ConstructLocation { BeforeConstruct, AfterConstruct,
                         HoldingBlock, EpilogBlock };

typedef hphp_hash_map<ConstructPtr, ConstructLocation,
                      smart_pointer_hash<ConstructPtr> > ConstructPtrLocMap;

class ControlFlowInfo {
public:
  ConstructPtrLocMap    m_targets[2];
  bool                  m_isTarget[2];
  bool                  m_noFallThrough;
};

typedef hphp_hash_map<ConstructPtr, ControlFlowInfo,
                      smart_pointer_hash<ConstructPtr> > ConstructCFIMap;

typedef hphp_hash_map<ConstructPtr, ControlBlockPtr,
                      smart_pointer_hash<ConstructPtr>
                      > ConstructControlBlockPtrMap;

class ControlFlowBuilder : public FunctionWalker {
public:
  ControlFlowBuilder(ControlFlowGraph *g) :
      m_graph(g), m_pass(0) {}

  int before(ConstructRawPtr cp);
  int after(ConstructRawPtr cp);

  void run(StatementPtr s) {
/*
    m_state = AstWalkerStateVec(s);
    m_pass = 0;
    AstWalker::walk(*this, this->m_state, ConstructPtr(), ConstructPtr());
*/
    m_state = AstWalkerStateVec(s);
    m_pass = 1;
    m_cur.reset();
    newBlock();
    m_head = m_cur;
    AstWalker::walk(*this, this->m_state, ConstructPtr(), ConstructPtr());

    m_state = AstWalkerStateVec(s);
    m_pass = 2;
    m_cur = m_head;
    AstWalker::walk(*this, this->m_state, ConstructPtr(), ConstructPtr());
  }

  ControlBlockPtr       head() const { return m_head; }
private:
  typedef ControlFlowGraph::vertex_descriptor   vertex_descriptor;

  void                  addEdge(ConstructPtr cp_from, ConstructLocation l_from,
                                ConstructPtr cp_to, ConstructLocation l_to) {
    assert(cp_from);
    assert(cp_to);
    assert(l_from < 2);

    ControlFlowInfo &from(cfi(cp_from));
    ControlFlowInfo &to(cfi(cp_to));

    from.m_targets[l_from][cp_to] = l_to;
    to.m_isTarget[l_to] = true;
  }
  void                  noFallThrough(ConstructPtr cp) {
    cfi(cp).m_noFallThrough = true;
  }

  ControlFlowInfo       *get(ConstructPtr cp) {
    ConstructCFIMap::iterator it = m_ccfiMap.find(cp);
    return it == m_ccfiMap.end() ? NULL : &it->second;
  }

  ControlFlowInfo       &cfi(ConstructPtr cp) {
    return m_ccfiMap[cp];
  }

  size_t                depth() const { return m_state.size(); }
  ConstructPtr          top(size_t n = 0) {
    size_t ix = m_state.size();
    assert(ix > n);
    ix -= n + 1;
    return m_state[ix].cp;
  }
  ConstructPtr          root() { return m_state[0].cp; }

  void newBlock(ConstructPtr cp = ConstructPtr(),
                ConstructLocation l = BeforeConstruct);
  void endBlock(ConstructPtr cp, ConstructLocation l);

  void addCFEdge(ControlBlockPtr b1, ControlBlockPtr b2);
  void addCFEdge(ControlBlockPtr b1, ConstructPtr c2, ConstructLocation l2);

  ConstructCFIMap                m_ccfiMap;
  ConstructControlBlockPtrMap    m_ccbpMap[4];
  ControlFlowGraph              *m_graph;
  AstWalkerStateVec              m_state;
  int                            m_pass;

  ControlBlockPtr                m_cur;
  ControlBlockPtr                m_head;
};

class dfn_assign : public boost::default_dfs_visitor {
public:
  dfn_assign(int &dfn) : m_dfn(dfn) {}

  void discover_vertex(ControlFlowGraph::vertex_descriptor u,
                       const ControlFlowGraph &g) {
    ControlBlockPtr cb = get(control_block, g, u);
    cb->setDfn(m_dfn++);
  }
private:
  int   &m_dfn;
};

class dfs_dump : public boost::default_dfs_visitor {
public:
  dfs_dump(AnalysisResultPtr ar) : m_ar(ar) {}

  void discover_vertex(ControlFlowGraph::vertex_descriptor u,
                       const ControlFlowGraph &g) {
    ControlBlockPtr cb = get(control_block, g, u);
    cb->dump(0, m_ar, &g);
  }
private:
  AnalysisResultPtr m_ar;
};

///////////////////////////////////////////////////////////////////////////////
}

using namespace HPHP;

ControlBlock::ControlBlock(const AstWalkerStateVec &s) : m_dfn(0), m_start(s) {

}

void ControlFlowBuilder::newBlock(ConstructPtr cp, ConstructLocation l) {
  endBlock(cp, l);
  ControlBlockPtr cb(new ControlBlock(m_state));
  vertex_descriptor v = add_vertex(cb, *m_graph);
  cb->setVertex(v);
  m_cur = cb;
}

void ControlFlowBuilder::endBlock(ConstructPtr cp, ConstructLocation l) {
  if (m_cur) {
    if (l == BeforeConstruct) {
      m_cur->setEndBefore(cp);
    } else {
      m_cur->setEndAfter(cp);
    }
    m_cur.reset();
  }
}

int ControlFlowBuilder::before(ConstructRawPtr cp) {
  int ret = FunctionWalker::before(cp);
  if (ret == WalkContinue) {
    if (m_pass == 1) {
      if (StatementPtr s = dynamic_pointer_cast<Statement>(cp)) {
        Statement::KindOf stype = s->getKindOf();
        switch (stype) {
          case Statement::KindOfFunctionStatement:
          case Statement::KindOfMethodStatement:
          case Statement::KindOfClassStatement:
          case Statement::KindOfInterfaceStatement:
            assert(false);
            break;

          case Statement::KindOfStaticStatement:
            addEdge(s, BeforeConstruct, s, AfterConstruct);
            break;

          case Statement::KindOfClassVariable:
            assert(false);
            break;

          case Statement::KindOfClassConstant:
          case Statement::KindOfGlobalStatement:
          case Statement::KindOfUnsetStatement:
          case Statement::KindOfExpStatement:
          case Statement::KindOfStatementList:
          case Statement::KindOfBlockStatement:
          case Statement::KindOfTryStatement:
            break;

          case Statement::KindOfIfStatement:
            break;

          case Statement::KindOfIfBranchStatement: {
            IfBranchStatementPtr ibr =
              static_pointer_cast<IfBranchStatement>(s);
            if (ibr->getCondition()) {
              if (ibr->getStmt()) {
                addEdge(ibr->getCondition(), AfterConstruct,
                        ibr, AfterConstruct);
                addEdge(ibr->getStmt(), AfterConstruct, top(2), AfterConstruct);
                noFallThrough(ibr);
              } else {
                addEdge(ibr->getCondition(), AfterConstruct,
                        top(2), AfterConstruct);
              }
            }
            break;
          }

          case Statement::KindOfForStatement: {
            ConstructPtr cond(s->getNthKid(ForStatement::CondExpr));
            ConstructPtr body(s->getNthKid(ForStatement::BodyStmt));
            ConstructPtr incr(s->getNthKid(ForStatement::IncExpr));
            if (cond) addEdge(cond, AfterConstruct, s, AfterConstruct);
            ConstructPtr end = incr ? incr : body ? body : cond;
            ConstructPtr start = cond ? cond : body ? body : incr;
            if (end) addEdge(end, AfterConstruct, start, BeforeConstruct);
            noFallThrough(s);
            break;
          }

          case Statement::KindOfWhileStatement: {
            ConstructPtr cond(s->getNthKid(WhileStatement::CondExpr));
            ConstructPtr body(s->getNthKid(WhileStatement::BodyStmt));
            addEdge(cond, AfterConstruct, s, AfterConstruct);
            addEdge(body ? body : cond, AfterConstruct, cond, BeforeConstruct);
            noFallThrough(s);
            break;
          }

          case Statement::KindOfDoStatement: {
            ConstructPtr cond(s->getNthKid(DoStatement::CondExpr));
            addEdge(cond, AfterConstruct, s, BeforeConstruct);
            break;
          }

          case Statement::KindOfForEachStatement: {
            ConstructPtr body(s->getNthKid(ForEachStatement::BodyStmt));
            ConstructPtr name(s->getNthKid(ForEachStatement::NameExpr));
            ConstructPtr value(s->getNthKid(ForEachStatement::ValueExpr));
            ConstructPtr begin = name ? name : value;
            ConstructPtr end = body ? body : value;
            addEdge(end, AfterConstruct, begin, BeforeConstruct);
            addEdge(begin, BeforeConstruct, s, AfterConstruct);
            break;
          }

          case Statement::KindOfSwitchStatement: {
            SwitchStatementPtr sw(static_pointer_cast<SwitchStatement>(s));
            ExpressionPtr exp = sw->getExp();
            if (StatementListPtr cases = sw->getCases()) {
              addEdge(exp, AfterConstruct, s, AfterConstruct);
              for (int n = cases->getCount(), i = 0; i < n; ++i) {
                addEdge(exp, AfterConstruct, (*cases)[i], BeforeConstruct);
              }
            }
            break;
          }

          case Statement::KindOfCaseStatement:
            break;

          case Statement::KindOfReturnStatement:
            addEdge(s, AfterConstruct, root(), AfterConstruct);
            break;

          case Statement::KindOfBreakStatement:
          case Statement::KindOfContinueStatement: {
            int val = dynamic_pointer_cast<BreakStatement>(s)->getDepth();
            size_t d = depth();
            for (size_t i = 1; i < d; i++) {
              ConstructPtr c = top(i);
              if (LoopStatementPtr l = dynamic_pointer_cast<LoopStatement>(c)) {
                if (val <= 1) {
                  if (stype == Statement::KindOfBreakStatement) {
                    addEdge(s, AfterConstruct, l, AfterConstruct);
                  } else {
                    ConstructPtr kid;
                    switch (l->getKindOf()) {
                      case Statement::KindOfForEachStatement:
                        kid = l->getNthKid(ForEachStatement::NameExpr);
                        if (!kid) {
                          kid = l->getNthKid(ForEachStatement::ValueExpr);
                        }
                        break;
                      case Statement::KindOfForStatement:
                        kid = l->getNthKid(ForStatement::IncExpr);
                        if (!kid) kid = l->getNthKid(ForStatement::CondExpr);
                        if (!kid) kid = l->getNthKid(ForStatement::BodyStmt);
                        break;
                      case Statement::KindOfWhileStatement:
                        kid = l->getNthKid(WhileStatement::CondExpr);
                        break;
                      case Statement::KindOfDoStatement:
                        kid = l->getNthKid(DoStatement::CondExpr);
                        break;
                      default:
                        assert(0);
                    }
                    assert(kid);
                    addEdge(s, AfterConstruct, kid, BeforeConstruct);
                  }
                }
                if (val && !--val) break;
              } else if (SwitchStatementPtr sw =
                         dynamic_pointer_cast<SwitchStatement>(c)) {
                if (val <= 1) {
                  addEdge(s, AfterConstruct, sw, AfterConstruct);
                }
                if (val && !--val) break;
              }
            }
            break;
          }

          case Statement::KindOfThrowStatement: {
            size_t d = depth();
            for (size_t i = 1; i < d; i++) {
              TryStatementPtr t = dynamic_pointer_cast<TryStatement>(top(i));
              if (t) {
                StatementListPtr catches = t->getCatches();
                for (int n = catches->getCount(), j = 0; j < n; ++j) {
                  addEdge(s, AfterConstruct, (*catches)[j], BeforeConstruct);
                }
                break;
              }
            }
            break;
          }

          case Statement::KindOfCatchStatement:
          case Statement::KindOfEchoStatement:
            break;
        }
      }


      if (ControlFlowInfo *c = get(cp)) {
        if (c->m_targets[BeforeConstruct].size()) {
          if (!m_cur) newBlock();
          m_ccbpMap[HoldingBlock][cp] = m_cur;
          endBlock(cp, BeforeConstruct);
        } else if (c->m_isTarget[BeforeConstruct]) {
          endBlock(cp, BeforeConstruct);
        }
      }
      if (!m_cur) newBlock();
      m_ccbpMap[BeforeConstruct][cp] = m_cur;
    } else if (m_pass == 2) {
      ControlBlockPtr bb = m_ccbpMap[BeforeConstruct][cp];
      assert(bb);
      if (bb != m_cur) {
        if (m_cur) {
          addCFEdge(m_cur, bb);
        }
        m_cur = bb;
      }
      if (ControlFlowInfo *c = get(cp)) {
        ConstructPtrLocMap &beforeTargets =
          c->m_targets[BeforeConstruct];
        if (beforeTargets.size()) {
          ControlBlockPtr hb = m_ccbpMap[HoldingBlock][cp];
          assert(hb);
          addCFEdge(hb, bb);
          ConstructPtrLocMap::iterator it =
            beforeTargets.begin(), end = beforeTargets.end();
          while (it != end) {
            addCFEdge(hb, it->first, it->second);
            ++it;
          }
        }
      }
    }
  }
  return ret;
}

void ControlFlowBuilder::addCFEdge(ControlBlockPtr b1, ControlBlockPtr b2) {
  if (!edge(b1->getVertex(), b2->getVertex(), *m_graph).second) {
    add_edge(b1->getVertex(), b2->getVertex(), *m_graph);
  }
}

void ControlFlowBuilder::addCFEdge(ControlBlockPtr b1,
                                   ConstructPtr c2, ConstructLocation l2) {
  ControlBlockPtr b2 = m_ccbpMap[l2][c2];
  assert(b2);
  addCFEdge(b1, b2);
}


int ControlFlowBuilder::after(ConstructRawPtr cp) {
  int ret = WalkContinue;

  if (m_pass == 1) {
    if (!m_cur) newBlock();
    if (ControlFlowInfo *c = get(cp)) {
      m_ccbpMap[AfterConstruct][cp] = m_cur;
      if (c->m_noFallThrough || c->m_targets[AfterConstruct].size()) {
        endBlock(cp, AfterConstruct);
      }

      if (c->m_isTarget[AfterConstruct]) {
        newBlock(cp, AfterConstruct);
        m_ccbpMap[EpilogBlock][cp] = m_cur;
      }
    } else {
      m_ccbpMap[AfterConstruct][cp] = m_cur;
    }
    return ret;
  }
  if (m_pass == 2) {
    ControlBlockPtr ab = m_ccbpMap[AfterConstruct][cp];
    assert(ab);
    if (ab != m_cur) {
      if (m_cur) {
        addCFEdge(m_cur, ab);
      }
      m_cur = ab;
    }
    if (ControlFlowInfo *c = get(cp)) {
      if (!c->m_noFallThrough &&
          c->m_isTarget[AfterConstruct]) {
        addCFEdge(m_cur, m_ccbpMap[EpilogBlock][cp]);
      }

      ConstructPtrLocMap &afterTargets =
        c->m_targets[AfterConstruct];
      if (afterTargets.size()) {
        ConstructPtrLocMap::iterator it =
          afterTargets.begin(), end = afterTargets.end();
        while (it != end) {
          addCFEdge(ab, it->first, it->second);
          ++it;
        }
      }

      if (c->m_noFallThrough && !c->m_isTarget[AfterConstruct]) m_cur.reset();
    }
    return ret;
  }

  return ret;
}

void ControlBlock::dump(int spc, AnalysisResultPtr ar,
                        const ControlFlowGraph *graph) {
  printf("%08llx (%d)\n  InDegree: %d\n  OutDegree: %d\n",
         (unsigned long long)this, m_dfn,
         (int)in_degree(m_vertex, *graph),
         (int)out_degree(m_vertex, *graph));

  {
    ControlFlowGraph::graph_traits::in_edge_iterator i, end;
    for (tie(i, end) = in_edges(m_vertex, *graph); i != end; ++i) {
      vertex_descriptor v = source(*i, *graph);
      ControlBlockPtr t = get(control_block, *graph, v);
      printf("    <- %08llx\n", (unsigned long long)t.get());
    }
  }
  {
    ControlFlowGraph::graph_traits::out_edge_iterator i, end;
    for (tie(i, end) = out_edges(m_vertex, *graph); i != end; ++i) {
      vertex_descriptor v = target(*i, *graph);
      ControlBlockPtr t = get(control_block, *graph, v);
      printf("    -> %08llx\n", (unsigned long long)t.get());
    }
  }
  Construct::dump(m_start.size() * 2, ar, true, m_start,
                  m_endBefore, m_endAfter);
}

ControlFlowGraph *ControlFlowGraph::buildControlFlow(MethodStatementPtr m) {
  ControlFlowGraph *graph = new ControlFlowGraph;

  ControlFlowBuilder cfb(graph);
  cfb.run(m->getStmts());
  int dfn = 1;
  depth_first_visit(*graph, cfb.head()->getVertex(),
                    dfn_assign(dfn), get(vertex_color, *graph));

  return graph;
}

void ControlFlowGraph::dump(AnalysisResultPtr ar) {
  printf("Dumping control flow:\n");
  depth_first_search(*this, dfs_dump(ar), get(vertex_color, *this));
}
