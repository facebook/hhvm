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
#include "hphp/compiler/analysis/control_flow.h"

#include <boost/graph/depth_first_search.hpp>

#include "hphp/compiler/analysis/ast_walker.h"
#include "hphp/compiler/analysis/data_flow.h"
#include "hphp/compiler/expression/expression.h"
#include "hphp/compiler/expression/binary_op_expression.h"
#include "hphp/compiler/expression/unary_op_expression.h"
#include "hphp/compiler/expression/qop_expression.h"
#include "hphp/compiler/statement/statement.h"
#include "hphp/compiler/statement/method_statement.h"
#include "hphp/compiler/statement/statement_list.h"
#include "hphp/compiler/statement/if_branch_statement.h"
#include "hphp/compiler/statement/for_statement.h"
#include "hphp/compiler/statement/while_statement.h"
#include "hphp/compiler/statement/do_statement.h"
#include "hphp/compiler/statement/foreach_statement.h"
#include "hphp/compiler/statement/switch_statement.h"
#include "hphp/compiler/statement/break_statement.h"
#include "hphp/compiler/statement/try_statement.h"
#include "hphp/compiler/statement/finally_statement.h"
#include "hphp/compiler/statement/label_statement.h"
#include "hphp/compiler/statement/goto_statement.h"
#include "hphp/compiler/statement/case_statement.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

enum ConstructLocation { BeforeConstruct, AfterConstruct,
                         HoldingBlock };

typedef hphp_hash_map<ConstructRawPtr, ConstructLocation,
                      smart_pointer_hash<ConstructRawPtr> > ConstructPtrLocMap;

class ControlFlowInfo {
public:
  ConstructPtrLocMap    m_targets[2];
  bool                  m_isTarget[2];
  bool                  m_noFallThrough;
};

typedef hphp_hash_map<ConstructRawPtr, ControlFlowInfo,
                      smart_pointer_hash<ConstructRawPtr> > ConstructCFIMap;

typedef hphp_hash_map<ConstructRawPtr, ControlBlock*,
                      smart_pointer_hash<ConstructRawPtr>
                      > ConstructControlBlockPtrMap;

class ControlFlowBuilder : public FunctionWalker {
public:
  explicit ControlFlowBuilder(ControlFlowGraph *g) :
      m_graph(g), m_pass(0), m_cur(0), m_head(0) {}

  int before(ConstructRawPtr cp);
  int after(ConstructRawPtr cp);
  int afterEach(ConstructRawPtr cp, int ix, ConstructRawPtr kid);
  void run(StatementPtr s) {
    m_state = AstWalkerStateVec(s);
    m_pass = 1;
    m_cur = 0;
    newBlock();
    m_head = m_cur;
    AstWalker::walk(*this, this->m_state, ConstructRawPtr(), ConstructRawPtr());
    for (LabelInfoMap::iterator it = m_labelInfoMap.begin(),
           end = m_labelInfoMap.end(); it != end; ++it) {
      LabelInfo &li = it->second;
      if (li.first && li.second.size()) {
        for (GotoStatementPtrVec::iterator ii = li.second.begin(),
               ee = li.second.end(); ii != ee; ++ii) {
          addEdge(*ii, AfterConstruct, li.first, BeforeConstruct);
        }
      }
    }
    newBlock();

    m_state = AstWalkerStateVec(s);
    m_pass = 2;
    m_cur = m_head;
    AstWalker::walk(*this, this->m_state, ConstructRawPtr(), ConstructRawPtr());
  }

  ControlBlock          *head() const { return m_head; }
private:
  typedef ControlFlowGraph::vertex_descriptor   vertex_descriptor;

  void addEdge(ConstructRawPtr cp_from, ConstructLocation l_from,
               ConstructRawPtr cp_to, ConstructLocation l_to) {
    always_assert(cp_from);
    always_assert(cp_to);
    always_assert(l_from < 2);

    ControlFlowInfo &from(cfi(cp_from));
    ControlFlowInfo &to(cfi(cp_to));

    from.m_targets[l_from][cp_to] = l_to;
    to.m_isTarget[l_to] = true;
  }
  void                  noFallThrough(ConstructRawPtr cp) {
    cfi(cp).m_noFallThrough = true;
  }

  void setEdge(ConstructRawPtr cp_from, ConstructLocation l_from,
               ConstructRawPtr cp_to, ConstructLocation l_to) {
    always_assert(cp_from);
    always_assert(cp_to);
    always_assert(l_from < 2);

    ControlFlowInfo &from(cfi(cp_from));
    from.m_targets[l_from].clear();
    addEdge(cp_from, l_from, cp_to, l_to);
  }

  ControlFlowInfo       *get(ConstructRawPtr cp) {
    ConstructCFIMap::iterator it = m_ccfiMap.find(cp);
    return it == m_ccfiMap.end() ? nullptr : &it->second;
  }

  ControlFlowInfo       &cfi(ConstructRawPtr cp) {
    return m_ccfiMap[cp];
  }

  size_t                depth() const { return m_state.size(); }
  ConstructRawPtr       top(size_t n = 0) {
    size_t ix = m_state.size();
    always_assert(ix > n);
    ix -= n + 1;
    return m_state[ix].cp;
  }
  ConstructRawPtr       root() { return m_state[0].cp; }

  void newBlock(ConstructRawPtr cp = ConstructPtr(),
                ConstructLocation l = BeforeConstruct);
  void endBlock(ConstructRawPtr cp, ConstructLocation l);

  void addCFEdge(ControlBlock *b1, ControlBlock *b2);
  void addCFEdge(ControlBlock *b1, ConstructRawPtr c2, ConstructLocation l2);

  void getTrueFalseBranches(
      int level,
      ConstructPtr &trueBranch, ConstructLocation &tLoc,
      ConstructPtr &falseBranch, ConstructLocation &fLoc);

  ConstructCFIMap                m_ccfiMap;
  ConstructControlBlockPtrMap    m_ccbpMap[4];
  ControlFlowGraph               *m_graph;
  AstWalkerStateVec              m_state;
  int                            m_pass;
  ControlBlock                   *m_cur;
  ControlBlock                   *m_head;

  class LabelInfo : public std::pair<LabelStatementPtr, GotoStatementPtrVec> {};
  class LabelInfoMap : public std::map<std::string,LabelInfo> {};
  LabelInfoMap                   m_labelInfoMap;
};

class dfn_assign : public boost::default_dfs_visitor {
public:
  dfn_assign() {}

  void discover_vertex(ControlFlowGraph::vertex_descriptor u,
                       const ControlFlowGraph &g) {
    const_cast<ControlFlowGraph&>(g).dfnAdd(u);
  }
};

class dfs_dump : public boost::default_dfs_visitor {
public:
  explicit dfs_dump(AnalysisResultConstPtr ar) : m_ar(ar) {}

  void discover_vertex(ControlFlowGraph::vertex_descriptor u,
                       const ControlFlowGraph &g) {
    u->dump(0, m_ar, &g);
  }
private:
  AnalysisResultConstPtr m_ar;
};

///////////////////////////////////////////////////////////////////////////////
}

using namespace HPHP;

ControlBlock::ControlBlock(const AstWalkerStateVec &s, ControlBlock *prev) :
    m_dfn(0), m_start(s), m_color(), m_next(0) {
  if (prev) prev->m_next = this;
}

ControlEdge *ControlBlock::find_to(ControlBlock *to) {
  graph_traits::out_edge_iterator it = m_succs.begin(), end = m_succs.end();
  while (it != end) {
    if ((*it)->second == to) return *it;
    ++it;
  }
  return 0;
}

void ControlBlock::add_edge(ControlEdge *e) {
  if (e->first->next() == e->second) {
    e->first->m_succs.push_front(e);
    e->second->m_preds.push_front(e);
  } else {
    e->first->m_succs.push_back(e);
    e->second->m_preds.push_back(e);
  }
}

void ControlFlowBuilder::newBlock(ConstructRawPtr cp, ConstructLocation l) {
  endBlock(cp, l);
  m_cur = m_graph->add_vertex(m_state);
}

void ControlFlowBuilder::endBlock(ConstructRawPtr cp, ConstructLocation l) {
  if (m_cur) {
    if (l == BeforeConstruct) {
      m_cur->setEndBefore(cp);
    } else {
      m_cur->setEndAfter(cp);
    }
    m_cur = 0;
  }
}

int ControlFlowBuilder::before(ConstructRawPtr cp) {
  int ret = FunctionWalker::before(cp);
  if (ret == WalkContinue) {
    if (m_pass == 1) {
      if (StatementPtr s = dynamic_pointer_cast<Statement>(cp)) {
        if (FunctionWalker::SkipRecurse(s)) not_reached();
        Statement::KindOf stype = s->getKindOf();
        switch (stype) {
          case Statement::KindOfUseTraitStatement:
          case Statement::KindOfTraitRequireStatement:
          case Statement::KindOfTraitPrecStatement:
          case Statement::KindOfTraitAliasStatement:
            not_reached();

          case Statement::KindOfStaticStatement:
            addEdge(s, BeforeConstruct, s, AfterConstruct);
            break;

          case Statement::KindOfClassVariable:
            not_reached();

          case Statement::KindOfClassConstant:
          case Statement::KindOfGlobalStatement:
          case Statement::KindOfUnsetStatement:
          case Statement::KindOfExpStatement:
          case Statement::KindOfStatementList:
          case Statement::KindOfBlockStatement:
            break;

          case Statement::KindOfTryStatement: {
            TryStatementPtr t = static_pointer_cast<TryStatement>(s);
            StatementListPtr catches = t->getCatches();
            StatementPtr body = t->getBody();
            FinallyStatementPtr finally = static_pointer_cast<FinallyStatement>(t->getFinally());
            if (body) {
              for (int n = catches->getCount(), j = 0; j < n; ++j) {
                addEdge(body, BeforeConstruct,
                        (*catches)[j], BeforeConstruct);
                addEdge(body, AfterConstruct,
                        (*catches)[j], BeforeConstruct);
              }
              if (finally) {
                addEdge(body, BeforeConstruct, finally, BeforeConstruct);
                addEdge(body, AfterConstruct, finally, BeforeConstruct);
              }
              addEdge(body, AfterConstruct, t, AfterConstruct);
              noFallThrough(body);
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

          case Statement::KindOfFinallyStatement:
            break;

          case Statement::KindOfCatchStatement:
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
            ForStatementPtr fs(static_pointer_cast<ForStatement>(s));
            ConstructRawPtr body(fs->getBody());
            ConstructRawPtr cond(fs->getCondExp());
            ConstructRawPtr incr(fs->getIncExp());
            if (cond) addEdge(cond, AfterConstruct, s, AfterConstruct);
            ConstructRawPtr end = incr ? incr : body ? body : cond;
            ConstructRawPtr start = cond ? cond : body ? body : incr;
            if (end) addEdge(end, AfterConstruct, start, BeforeConstruct);
            noFallThrough(s);
            break;
          }

          case Statement::KindOfWhileStatement: {
            WhileStatementPtr ws(static_pointer_cast<WhileStatement>(s));
            ConstructRawPtr body(ws->getBody());
            ConstructRawPtr cond(ws->getCondExp());
            addEdge(cond, AfterConstruct, s, AfterConstruct);
            addEdge(body ? body : cond, AfterConstruct, cond, BeforeConstruct);
            noFallThrough(s);
            break;
          }

          case Statement::KindOfDoStatement: {
            DoStatementPtr ds(static_pointer_cast<DoStatement>(s));
            addEdge(ds->getCondExp(), AfterConstruct, s, BeforeConstruct);
            break;
          }

          case Statement::KindOfForEachStatement: {
            ForEachStatementPtr fs(static_pointer_cast<ForEachStatement>(s));
            ConstructRawPtr body(fs->getBody());
            ConstructRawPtr name(fs->getNameExp());
            ConstructRawPtr value(fs->getValueExp());
            ConstructRawPtr begin = name ? name : value;
            ConstructRawPtr end = body ? body : value;
            addEdge(end, AfterConstruct, begin, BeforeConstruct);
            addEdge(value, AfterConstruct, s, AfterConstruct);
            break;
          }

          case Statement::KindOfSwitchStatement: {
            SwitchStatementPtr sw(static_pointer_cast<SwitchStatement>(s));
            if (StatementListPtr cases = sw->getCases()) {
              ExpressionPtr exp;
              StatementPtr def;
              for (int n = cases->getCount(), i = 0; i < n; ++i) {
                CaseStatementPtr caseStmt =
                  static_pointer_cast<CaseStatement>((*cases)[i]);
                if (!caseStmt->getCondition()) {
                  def = caseStmt->getStatement();
                } else {
                  if (exp) {
                    addEdge(exp, AfterConstruct, caseStmt, BeforeConstruct);
                  }
                  exp = caseStmt->getCondition();
                }
              }
              if (exp) {
                if (def) {
                  addEdge(exp, AfterConstruct, def, BeforeConstruct);
                } else {
                  addEdge(exp, AfterConstruct, s, AfterConstruct);
                }
              }
            }
            break;
          }

          case Statement::KindOfCaseStatement:
            // already handled by switch
            break;

          case Statement::KindOfLabelStatement: {
            LabelStatementPtr l(static_pointer_cast<LabelStatement>(s));
            m_labelInfoMap[l->label()].first = l;
            cfi(l).m_isTarget[BeforeConstruct] = true;
            break;
          }

          case Statement::KindOfGotoStatement: {
            GotoStatementPtr g(static_pointer_cast<GotoStatement>(s));
            m_labelInfoMap[g->label()].second.push_back(g);
            noFallThrough(s);
            break;
          }

          case Statement::KindOfReturnStatement: {
            setEdge(s, AfterConstruct, root(), AfterConstruct);
            noFallThrough(s);
            /*
             * Since almost anything in php /might/ throw, we
             * approximate, and add edges from the beginning and
             * end of a try block. But if there's a return, that
             * would kill the edge from the end of the block.
             * Explicitly add them here if the return is in a try
             */
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

          case Statement::KindOfBreakStatement:
          case Statement::KindOfContinueStatement: {
            noFallThrough(s);
            int val = dynamic_pointer_cast<BreakStatement>(s)->getDepth();
            size_t d = depth();
            for (size_t i = 1; i < d; i++) {
              ConstructRawPtr c = top(i);
              if (LoopStatementPtr l = dynamic_pointer_cast<LoopStatement>(c)) {
                if (val <= 1) {
                  if (stype == Statement::KindOfBreakStatement) {
                    addEdge(s, AfterConstruct, l, AfterConstruct);
                  } else {
                    ConstructRawPtr kid;
                    switch (l->getKindOf()) {
                      case Statement::KindOfForEachStatement: {
                        ForEachStatementPtr fs(static_pointer_cast<ForEachStatement>(l));
                        kid = fs->getNameExp();
                        if (!kid) {
                          kid = fs->getValueExp();
                        }
                        break;
                      }
                      case Statement::KindOfForStatement: {
                        ForStatementPtr fs(static_pointer_cast<ForStatement>(l));
                        kid = fs->getIncExp();
                        if (!kid) kid = fs->getCondExp();
                        if (!kid) kid = fs->getBody();
                        break;
                      }
                      case Statement::KindOfWhileStatement: {
                        WhileStatementPtr ws(static_pointer_cast<WhileStatement>(l));
                        kid = ws->getCondExp();
                        break;
                      }
                      case Statement::KindOfDoStatement: {
                        DoStatementPtr ds(static_pointer_cast<DoStatement>(l));
                        kid = ds->getCondExp();
                        break;
                      }
                      default:
                        always_assert(0);
                    }
                    always_assert(kid);
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

          case Statement::KindOfEchoStatement:
          case Statement::KindOfTypedefStatement:
            break;

          default:
            not_reached();
        }
      } else {
        ExpressionPtr e(dynamic_pointer_cast<Expression>(cp));
        switch (e->getKindOf()) {
          case Expression::KindOfBinaryOpExpression: {
            BinaryOpExpressionPtr b(static_pointer_cast<BinaryOpExpression>(e));
            if (b->isShortCircuitOperator()) {
              ConstructPtr trueBranch, falseBranch;
              ConstructLocation tLoc, fLoc;
              getTrueFalseBranches(0, trueBranch, tLoc, falseBranch, fLoc);
              assert(trueBranch);
              assert(falseBranch);
              if (b->isLogicalOrOperator()) {
                addEdge(b->getExp1(), AfterConstruct, trueBranch, tLoc);
              } else {
                addEdge(b->getExp1(), AfterConstruct, falseBranch, fLoc);
              }
            }
            break;
          }
          case Expression::KindOfQOpExpression: {
            QOpExpressionPtr q(static_pointer_cast<QOpExpression>(e));
            if (ExpressionPtr e1 = q->getYes()) {
              addEdge(q->getCondition(), AfterConstruct,
                      q->getNo(), BeforeConstruct);
              addEdge(e1, AfterConstruct,
                      q->getNo(), AfterConstruct);
              noFallThrough(e1);
            } else {
              addEdge(q->getCondition(), AfterConstruct,
                      q->getNo(), AfterConstruct);
            }
            break;
          }
          default:
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
      ControlBlock *hb = m_ccbpMap[HoldingBlock][cp];
      if (hb) {
        if (hb != m_cur) {
          if (m_cur) {
            addCFEdge(m_cur, hb);
          }
          m_cur = hb;
        }
      }
      ControlBlock *bb = m_ccbpMap[BeforeConstruct][cp];
      always_assert(bb);
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
          always_assert(hb);
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

void ControlFlowBuilder::getTrueFalseBranches(
    int level,
    ConstructPtr &trueBranch, ConstructLocation &tLoc,
    ConstructPtr &falseBranch, ConstructLocation &fLoc) {
  if (trueBranch && falseBranch) return;

  ConstructPtr c(top(level));
  if (StatementPtr s = dynamic_pointer_cast<Statement>(c)) {
    StatementPtr kidBody;
    switch (s->getKindOf()) {
    case Statement::KindOfForStatement: {
      // examine which context we're in
      ForStatementPtr fs(static_pointer_cast<ForStatement>(s));
      ConstructPtr kid(top(level - 1));
      if (kid == fs->getInitExp()) {
        ; // just do the default case
      } else if (kid == fs->getCondExp()) {
        kidBody = fs->getBody();
        goto loop_stmt;
      } else if (kid == fs->getIncExp()) {
        ; // just do the default case
      } else {
        assert(false);
      }
      break;
    }
    case Statement::KindOfWhileStatement: {
      WhileStatementPtr ws(static_pointer_cast<WhileStatement>(s));
      kidBody = ws->getBody();
      goto loop_stmt;
    }
    case Statement::KindOfDoStatement: {
      DoStatementPtr ds(static_pointer_cast<DoStatement>(s));
      kidBody = ds->getBody();
    }
    loop_stmt:
      if (!trueBranch) {
        trueBranch = kidBody;
        tLoc = BeforeConstruct;
      }
      if (!falseBranch) {
        falseBranch = s;
        fLoc = AfterConstruct;
      }
      break;
    case Statement::KindOfIfBranchStatement:
      {
        IfBranchStatementPtr i(
            static_pointer_cast<IfBranchStatement>(s));
        if (!trueBranch) {
          if (i->getStmt()) {
            trueBranch = i->getStmt();
            tLoc = BeforeConstruct;
          } else {
            trueBranch = i;
            tLoc = AfterConstruct;
          }
        }
        if (!falseBranch) {
          falseBranch = i;
          fLoc = AfterConstruct;
        }
      }
      break;
    default:
      break;
    }
  } else if (ExpressionPtr e = dynamic_pointer_cast<Expression>(c)) {
    switch (e->getKindOf()) {
    case Expression::KindOfBinaryOpExpression:
      {
        BinaryOpExpressionPtr b(
            static_pointer_cast<BinaryOpExpression>(e));
        if (b->isShortCircuitOperator()) {
          if (b->isLogicalOrOperator()) {
            if (!trueBranch || !falseBranch) {
              getTrueFalseBranches(
                  level + 1, trueBranch, tLoc,
                  falseBranch, fLoc);
            }
            assert(trueBranch && falseBranch);
            if (level == 0 ||
                b->getExp1() == top(level - 1)) {
              falseBranch = b->getExp2();
              fLoc = BeforeConstruct;
            }
          } else {
            // logical ANDs
            if (!trueBranch || !falseBranch) {
              getTrueFalseBranches(
                  level + 1, trueBranch, tLoc,
                  falseBranch, fLoc);
            }
            assert(trueBranch && falseBranch);
            if (level == 0 ||
                b->getExp1() == top(level - 1)) {
              trueBranch = b->getExp2();
              tLoc = BeforeConstruct;
            }
          }
          return;
        }
      }
      break;
    case Expression::KindOfUnaryOpExpression:
      {
        UnaryOpExpressionPtr u(
            static_pointer_cast<UnaryOpExpression>(e));
        if (u->isLogicalNot()) {
          getTrueFalseBranches(level + 1,
              trueBranch, tLoc, falseBranch, fLoc);
          // swap
          ConstructPtr tp(trueBranch);
          ConstructLocation tl(tLoc);
          trueBranch = falseBranch;
          tLoc = fLoc;
          falseBranch = tp;
          fLoc = tl;
          return;
        }
      }
      break;
    case Expression::KindOfQOpExpression:
      {
        assert(level > 0);
        QOpExpressionPtr qop(
            static_pointer_cast<QOpExpression>(e));
        if (qop->getCondition() == top(level - 1)) {
          if (!trueBranch) {
            if (qop->getYes()) {
              trueBranch = qop->getYes();
              tLoc = BeforeConstruct;
            } else {
              trueBranch = qop;
              tLoc = AfterConstruct;
            }
          }
          if (!falseBranch) {
            falseBranch = qop->getNo();
            fLoc = BeforeConstruct;
          }
          return;
        }
      }
      break;
    default:
      break;
    }
  }
  assert(level > 0);
  if (!trueBranch) {
    trueBranch = top(level - 1);
    tLoc = AfterConstruct;
  }
  if (!falseBranch) {
    falseBranch = top(level - 1);
    fLoc = AfterConstruct;
  }
}

void ControlFlowBuilder::addCFEdge(ControlBlock *b1, ControlBlock *b2) {
  if (!edge(b1, b2, *m_graph).second) {
    add_edge(b1, b2, *m_graph);
  }
}

void ControlFlowBuilder::addCFEdge(ControlBlock *b1,
                                   ConstructRawPtr c2, ConstructLocation l2) {
  ControlBlock *b2 = m_ccbpMap[l2][c2];
  always_assert(b2);
  if (l2 == AfterConstruct) b2 = b2->next();
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
    } else {
      m_ccbpMap[AfterConstruct][cp] = m_cur;
    }
    return ret;
  }
  if (m_pass == 2) {
    ControlBlock *ab = m_ccbpMap[AfterConstruct][cp];
    always_assert(ab);
    if (ab != m_cur) {
      if (m_cur) {
        addCFEdge(m_cur, ab);
      }
      m_cur = ab;
    }
    if (ControlFlowInfo *c = get(cp)) {
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

      if (c->m_noFallThrough && !c->m_isTarget[AfterConstruct]) m_cur = 0;
    }
    return ret;
  }

  return ret;
}

int ControlFlowBuilder::afterEach(ConstructRawPtr cp, int ix,
                                  ConstructRawPtr kid) {
  if (m_pass == 1) {
    if (ControlFlowInfo *c = get(kid)) {
      if (c->m_isTarget[AfterConstruct]) {
        newBlock(kid, AfterConstruct);
      }
    }
  }

  return WalkContinue;
}

void ControlBlock::dump(int spc, AnalysisResultConstPtr ar,
                        const ControlFlowGraph *graph) {
  printf("%08llx (%d)\n  InDegree: %d\n  OutDegree: %d\n",
         (unsigned long long)this, m_dfn,
         (int)in_degree(this, *graph),
         (int)out_degree(this, *graph));

  {
    ControlFlowGraph::graph_traits::in_edge_iterator i, end;
    for (boost::tie(i, end) = in_edges(this, *graph); i != end; ++i) {
      ControlBlock *t = source(*i, *graph);
      printf("    <- %08llx\n", (unsigned long long)t);
    }
  }
  {
    ControlFlowGraph::graph_traits::out_edge_iterator i, end;
    for (boost::tie(i, end) = out_edges(this, *graph);
        i != end; ++i) {
      ControlBlock *t = target(*i, *graph);
      printf("    -> %08llx\n", (unsigned long long)t);
    }
  }

  if (m_dfn) {
    for (int i = 0; i < DataFlow::NumBVs; i++) {
      if (graph->rowExists(i)) {
        BitOps::Bits *row = getRow(i);
        printf("  Row %s:", DataFlow::GetName(i));
        for (int b = 0, n = graph->bitWidth(); b < n; ++b) {
          if (BitOps::get_bit(b, row)) {
            printf(" %1d", b);
          }
        }
        printf("\n");
      }
    }
  }

  Construct::dump(m_start.size() * 2, ar, true, m_start,
                  m_endBefore, m_endAfter);
}

ControlFlowGraph *ControlFlowGraph::buildControlFlow(MethodStatementPtr m) {
  ControlFlowGraph *graph = new ControlFlowGraph;

  graph->m_stmt = m;
  ControlFlowBuilder cfb(graph);
  cfb.run(m->getStmts());
  graph->m_nextDfn = 1;
  depth_first_visit(*graph, cfb.head(),
                    dfn_assign(), get(boost::vertex_color, *graph));
  return graph;
}

ControlFlowGraph::~ControlFlowGraph() {
  for (edge_iterator it = ebegin(), end = eend();
       it != end; ++it) {
    delete *it;
  }
  for (vertex_iterator it = vbegin(), end = vend();
       it != end; ++it) {
    delete *it;
  }
}

ControlBlock *ControlFlowGraph::add_vertex(AstWalkerStateVec &s) {
  ControlBlock *cb =
    new ControlBlock(s, m_blocks.size() ? m_blocks.back() : 0);

  m_blocks.push_back(cb);
  return cb;
}

ControlEdge *ControlFlowGraph::add_edge(ControlBlock *from, ControlBlock *to) {
  ControlEdge *e = new ControlEdge(from, to);
  m_edges.push_back(e);
  ControlBlock::add_edge(e);
  return e;
}

void ControlFlowGraph::dfnAdd(ControlBlock *cb) {
  m_depthFirstBlocks.push_back(cb);
  cb->setDfn(m_nextDfn++);
}

void ControlFlowGraph::allocateDataFlow(size_t width, int rows, int *rowIds) {
  m_bitSetVec.alloc(m_nextDfn, width, rows, rowIds);
  graph_traits::vertex_iterator i,e;
  for (boost::tie(i, e) = vertices(*this); i != e; ++i) {
    ControlBlock *cb = *i;
    cb->setBlock(m_bitSetVec.getBlock(cb->getDfn()));
  }
}

void ControlFlowGraph::dump(AnalysisResultConstPtr ar) {
  printf("Dumping control flow: %s\n", m_stmt->getName().c_str());
  boost::depth_first_search(*this, dfs_dump(ar),
    get(boost::vertex_color, *this));
}
