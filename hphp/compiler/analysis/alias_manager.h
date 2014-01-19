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

#ifndef incl_HPHP_ALIAS_MANAGER_H_
#define incl_HPHP_ALIAS_MANAGER_H_

#include "hphp/compiler/expression/expression.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(MethodStatement);
DECLARE_BOOST_TYPES(SimpleVariable);
DECLARE_BOOST_TYPES(ListAssignment);
DECLARE_BOOST_TYPES(ArrayElementExpression);

class ControlFlowGraph;

class BucketMapEntry {
 public:
  BucketMapEntry() : m_num(0), m_next(0) {}
 public:
  ExpressionPtr find(ExpressionPtr e);
  void add(ExpressionPtr e);
  void clear();
  void beginScope();
  void endScope();
  void resetScope();
  void pop_back() { m_exprs.pop_back(); }
  ExpressionPtr back() { return m_exprs.back(); }
  ExpressionPtrList::iterator begin() { return m_exprs.begin(); }
  ExpressionPtrList::iterator end() { return m_exprs.end(); }
  ExpressionPtrList::reverse_iterator rbegin() { return m_exprs.rbegin(); }
  ExpressionPtrList::reverse_iterator rend() { return m_exprs.rend(); }
  bool isLast(ExpressionPtr e) { return *rbegin() == e; }
  bool isSubLast(ExpressionPtr e);
  size_t size() { return m_num; }
  void stash(size_t from, ExpressionPtrList &to);
  void import(ExpressionPtrList &from);
  void erase(ExpressionPtrList::reverse_iterator rit,
             ExpressionPtrList::reverse_iterator &end) {
    // the base of a reverse iterator points one beyond
    // the element the reverse iterator points to, so
    // decrement it...
    ExpressionPtrList::iterator it(--rit.base());
    // erasing an element /can/ invalidate the end
    // reverse_iterator, so fix it if necessary
    if (end.base() == it) end = rit;
    m_exprs.erase(it);
    // rit is still valid, and now (magically) points at
    // the next element... no need to return a new iterator.
    m_num--;
  }
  BucketMapEntry *next() const { return m_next; }
  void link(BucketMapEntry *&tail) {
    if (!m_next) {
      if (tail) {
        m_next = tail->m_next;
        tail->m_next = this;
      } else {
        m_next = this;
      }
      tail = this;
    }
  }
 private:
  ExpressionPtrList     m_exprs;
  std::vector<size_t>   m_stack;
  size_t                m_num;
  BucketMapEntry        *m_next;
};

class AliasManager {
 public:
  enum { SameAccess, SameLValueAccess, InterfAccess,
    DisjointAccess, NotAccess };

  explicit AliasManager(int opt);
  ~AliasManager();

  void clear();
  void beginScope();
  void endScope();
  void resetScope();
  bool insertForDict(ExpressionPtr e);
  ExpressionPtr getCanonical(ExpressionPtr e);

  void gatherInfo(AnalysisResultConstPtr ar, MethodStatementPtr m);
  int optimize(AnalysisResultConstPtr ar, MethodStatementPtr s);
  void finalSetup(AnalysisResultConstPtr ar, MethodStatementPtr m);
  int copyProp(MethodStatementPtr m);

  void setChanged() {
    if (!m_noAdd) {
      m_changes++;
      m_scope->addUpdates(BlockScope::UseKindCaller);
    }
  }

  static bool parseOptimizations(const std::string &optimizations,
                                 std::string &errs);

  ControlFlowGraph *graph() { return m_graph; }
  int checkAnyInterf(ExpressionPtr rv, ExpressionPtr e, bool &isLoad,
                     int &depth, int &effects, bool forLval = false);
  bool hasWildRefs() const { return m_wildRefs; }
  bool couldBeAliased(SimpleVariablePtr sv);

  AnalysisResultConstPtr getAnalysisResult() { return m_arp; }
 private:

  int findInterf0(
    ExpressionPtr rv, bool isLoad,
    ExpressionPtr &rep,
    ExpressionPtrList::reverse_iterator begin,
    ExpressionPtrList::reverse_iterator end,
    int *flags = 0,
    bool allowLval = false, bool forLval = false,
    int depth = 0, int min_depth = 0,
    int max_depth = 0);

  void setCanonPtrForArrayCSE(
      ExpressionPtr e,
      ExpressionPtr rep);

  void doFinal(MethodStatementPtr m);
  enum { MaxBuckets = 0x10000 };
  enum { FallThrough, CondBranch, Branch, Converge };
  enum { NoCopyProp = 1, NoDeadStore = 2 };
  struct CondStackElem {
    explicit CondStackElem(size_t s = 0) : m_size(s), m_exprs() {}
    size_t              m_size;
    ExpressionPtrList   m_exprs;
  };

  void performReferencedAndNeededAnalysis(MethodStatementPtr m);
  void insertTypeAssertions(AnalysisResultConstPtr ar, MethodStatementPtr m);
  void removeTypeAssertions(AnalysisResultConstPtr ar, MethodStatementPtr m);

  typedef std::set<std::string> StringSet;

  class LoopInfo {
  public:
    explicit LoopInfo(StatementPtr s);

    StatementPtr m_stmt;
    StatementPtrVec m_inner;
    bool m_valid;
    StringSet m_candidates;
    StringSet m_excluded;
  };

  typedef hphp_hash_map<unsigned, BucketMapEntry> BucketMap;
  typedef std::vector<CondStackElem> CondStack;
  typedef std::vector<LoopInfo> LoopInfoVec;

  void mergeScope();

  static void clearHelper(BucketMap::value_type &it);
  static void beginScopeHelper(BucketMap::value_type &it);
  static void endScopeHelper(BucketMap::value_type &it);
  static void resetScopeHelper(BucketMap::value_type &it);

  void add(BucketMapEntry &em, ExpressionPtr e);

  void dumpAccessChain();
  int testAccesses(ExpressionPtr e1, ExpressionPtr e2, bool forLval = false);
  void cleanRefs(ExpressionPtr rv,
                 ExpressionPtrList::reverse_iterator it,
                 ExpressionPtrList::reverse_iterator &end,
                 int depth);
  void cleanInterf(ExpressionPtr rv,
                   ExpressionPtrList::reverse_iterator it,
                   ExpressionPtrList::reverse_iterator &end,
                   int depth);
  void killLocals();
  bool okToKill(ExpressionPtr ep, bool killRef);
  int checkInterf(ExpressionPtr rv, ExpressionPtr e, bool &isLoad,
                  int &depth, int &effects, bool forLval = false);
  int findInterf(ExpressionPtr rv, bool isLoad, ExpressionPtr &rep,
                 int *flags = 0, bool allowLval = false);
  void applyAssign(ExpressionPtr lhs, ExpressionPtr rhs);
  void processAccessChain(ExpressionPtr e);
  void processAccessChainLA(ListAssignmentPtr e);

  void canonicalizeKid(ConstructPtr e, ExpressionPtr kid, int i);
  int canonicalizeKid(ConstructPtr e, ConstructPtr kid, int i);
  ExpressionPtr canonicalizeNode(ExpressionPtr e, bool doAccessChains = false);
  ExpressionPtr canonicalizeNonNull(ExpressionPtr e);
  ExpressionPtr canonicalizeRecurNonNull(ExpressionPtr e);
  ExpressionPtr canonicalizeRecur(ExpressionPtr e);
  StatementPtr canonicalizeRecur(StatementPtr e, int &ret);

  void invalidateChainRoots(StatementPtr s);
  void nullSafeDisableCSE(StatementPtr parent, ExpressionPtr kid);
  void disableCSE(StatementPtr s);
  void createCFG(MethodStatementPtr m);
  void deleteCFG();

  int collectAliasInfoRecur(ConstructPtr cs, bool unused);
  void pushStringScope(StatementPtr s);
  void popStringScope(StatementPtr s);
  void stringOptsRecur(StatementPtr s);
  void stringOptsRecur(ExpressionPtr s, bool ok);

  void beginInExpression(StatementPtr parent, ExpressionPtr kid);
  void endInExpression(StatementPtr requestor);
  bool isInExpression() const {
    assert((m_exprIdx >=  0 && m_exprParent) ||
           (m_exprIdx == -1 && !m_exprParent));
    return m_exprIdx != -1;
  }

  /**
   * Take e and walk down the expression chain, marking all
   * interferences as "altered". It is assumed that e is
   * a store (has modifications)
   */
  void markAllLocalExprAltered(ExpressionPtr e);

  BucketMapEntry            m_accessList;
  BucketMap                 m_bucketMap;
  BucketMapEntry            *m_bucketList;

  CondStack                 m_stack;

  unsigned                  m_nextID;

  int                       m_changes;
  int                       m_replaced;
  bool                      m_wildRefs;

  AnalysisResultConstPtr    m_arp;
  VariableTablePtr          m_variables;

  LoopInfoVec               m_loopInfo;

  std::string               m_returnVar;
  int                       m_nrvoFix;

  int                       m_inCall;
  bool                      m_inlineAsExpr;
  bool                      m_noAdd;
  bool                      m_preOpt;
  bool                      m_postOpt;
  bool                      m_cleared;
  bool                      m_inPseudoMain;
  bool                      m_genAttrs;
  bool                      m_hasDeadStore;
  bool                      m_hasChainRoot;
  bool                      m_hasTypeAssertions;
  BlockScopeRawPtr          m_scope;

  ControlFlowGraph          *m_graph;
  std::map<std::string,int> m_gidMap;
  std::map<std::string,SimpleVariablePtr> m_objMap;

  ExpressionPtr             m_expr;
  int                       m_exprIdx;
  StatementPtr              m_exprParent;
  ExpressionPtrVec          m_exprBeginStack;
};

///////////////////////////////////////////////////////////////////////////////
}
#endif // incl_HPHP_ALIAS_MANAGER_H_
