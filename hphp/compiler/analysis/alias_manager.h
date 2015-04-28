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

#ifndef incl_HPHP_ALIAS_MANAGER_H_
#define incl_HPHP_ALIAS_MANAGER_H_

#include "hphp/compiler/expression/expression.h"
#include <map>
#include <set>
#include <vector>

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

  AliasManager();
  ~AliasManager();

  void clear();
  void beginScope();
  void endScope();
  void resetScope();
  ExpressionPtr getCanonical(ExpressionPtr e);

  void gatherInfo(AnalysisResultConstPtr ar, MethodStatementPtr m);
  int optimize(AnalysisResultConstPtr ar, MethodStatementPtr s);

  void setChanged() {
    if (!m_noAdd) {
      m_changes++;
      m_scope->addUpdates(BlockScope::UseKindCaller);
    }
  }

  static bool parseOptimizations(const std::string &optimizations,
                                 std::string &errs);

  int checkAnyInterf(ExpressionPtr rv, ExpressionPtr e, bool &isLoad,
                     int &depth, int &effects, bool forLval = false);
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

  enum { MaxBuckets = 0x10000 };
  enum { FallThrough, CondBranch, Branch, Converge };
  enum { NoCopyProp = 1, NoDeadStore = 2 };
  struct CondStackElem {
    explicit CondStackElem(size_t s = 0) : m_size(s), m_exprs() {}
    size_t              m_size;
    ExpressionPtrList   m_exprs;
  };

  typedef std::set<std::string> StringSet;
  typedef hphp_hash_map<unsigned, BucketMapEntry> BucketMap;
  typedef std::vector<CondStackElem> CondStack;

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
                 int *flags = 0);
  void processAccessChain(ExpressionPtr e);
  void processAccessChainLA(ListAssignmentPtr e);

  void canonicalizeKid(ConstructPtr e, ExpressionPtr kid, int i);
  int canonicalizeKid(ConstructPtr e, ConstructPtr kid, int i);
  ExpressionPtr canonicalizeNode(ExpressionPtr e, bool doAccessChains = false);
  ExpressionPtr canonicalizeNonNull(ExpressionPtr e);
  ExpressionPtr canonicalizeRecurNonNull(ExpressionPtr e);
  ExpressionPtr canonicalizeRecur(ExpressionPtr e);
  StatementPtr canonicalizeRecur(StatementPtr e, int &ret);

  int collectAliasInfoRecur(ConstructPtr cs, bool unused);

  void beginInExpression(StatementPtr parent, ExpressionPtr kid);
  void endInExpression(StatementPtr requestor);
  bool isInExpression() const {
    assert((m_exprIdx >=  0 && m_exprParent) ||
           (m_exprIdx == -1 && !m_exprParent));
    return m_exprIdx != -1;
  }

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

  int                       m_inCall;
  bool                      m_noAdd;
  bool                      m_cleared;
  bool                      m_inPseudoMain;
  BlockScopeRawPtr          m_scope;

  ExpressionPtr             m_expr;
  int                       m_exprIdx;
  StatementPtr              m_exprParent;
  ExpressionPtrVec          m_exprBeginStack;
};

///////////////////////////////////////////////////////////////////////////////
}
#endif // incl_HPHP_ALIAS_MANAGER_H_
