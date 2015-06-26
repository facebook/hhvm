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

#ifndef incl_HPHP_BLOCK_SCOPE_H_
#define incl_HPHP_BLOCK_SCOPE_H_

#include "hphp/compiler/hphp.h"

#include "hphp/util/bits.h"
#include "hphp/util/lock.h"

#include <tbb/concurrent_hash_map.h>
#include <list>
#include <memory>
#include <set>
#include <utility>
#include <vector>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class CodeGenerator;
DECLARE_BOOST_TYPES(Statement);
DECLARE_BOOST_TYPES(AnalysisResult);
DECLARE_BOOST_TYPES(VariableTable);
DECLARE_BOOST_TYPES(ConstantTable);
DECLARE_BOOST_TYPES(ModifierExpression);
DECLARE_BOOST_TYPES(IncludeExpression);
DECLARE_BOOST_TYPES(StatementList);
DECLARE_BOOST_TYPES(BlockScope);
DECLARE_BOOST_TYPES(ClassScope);
DECLARE_BOOST_TYPES(FunctionScope);
DECLARE_BOOST_TYPES(FileScope);

typedef hphp_hash_map<BlockScopeRawPtr, int,
                      smart_pointer_hash<BlockScopeRawPtr>
                      > BlockScopeRawPtrFlagsHashMap;
typedef hphp_hash_set<BlockScopeRawPtr,
                      smart_pointer_hash<BlockScopeRawPtr>
                      > BlockScopeRawPtrHashSet;
typedef tbb::concurrent_hash_map<BlockScopeRawPtr, int,
                                 smart_pointer_hash<BlockScopeRawPtr> >
        ConcurrentBlockScopeRawPtrIntHashMap;

typedef std::vector<BlockScopeRawPtr> BlockScopeRawPtrVec;
typedef std::list<BlockScopeRawPtr>   BlockScopeRawPtrQueue;
typedef std::vector<BlockScopeRawPtrFlagsHashMap::
                    value_type*> BlockScopeRawPtrFlagsVec;
typedef std::pair< BlockScopeRawPtr, int* >
        BlockScopeRawPtrFlagsPtrPair;
typedef std::vector< std::pair< BlockScopeRawPtr, int* > >
        BlockScopeRawPtrFlagsPtrVec;

typedef SimpleMutex InferTypesMutex;

/**
 * Base class of ClassScope and FunctionScope.
 */
class BlockScope : private boost::noncopyable,
                   public std::enable_shared_from_this<BlockScope> {
public:
  enum KindOf {
    ClassScope,
    FunctionScope,
    FileScope,
    ProgramScope,
  };

  enum UseKinds {
    /* Callers */
    UseKindCallerInline   = 0x1,
    UseKindCallerParam    = 0x1 << 1,
    UseKindCallerReturn   = 0x1 << 2,
    UseKindCaller         = (UseKindCallerInline |
                             UseKindCallerParam  |
                             UseKindCallerReturn),

    /* Static references */
    UseKindStaticRef      = 0x1 << 3,

    /* 16 bits of Non static references */
    UseKindNonStaticRef0  = 0x1 << 4,
    UseKindNonStaticRef1  = 0x1 << 5,
    UseKindNonStaticRef2  = 0x1 << 6,
    UseKindNonStaticRef3  = 0x1 << 7,
    UseKindNonStaticRef4  = 0x1 << 8,
    UseKindNonStaticRef5  = 0x1 << 9,
    UseKindNonStaticRef6  = 0x1 << 10,
    UseKindNonStaticRef7  = 0x1 << 11,
    UseKindNonStaticRef8  = 0x1 << 12,
    UseKindNonStaticRef9  = 0x1 << 13,
    UseKindNonStaticRef10 = 0x1 << 14,
    UseKindNonStaticRef11 = 0x1 << 15,
    UseKindNonStaticRef12 = 0x1 << 16,
    UseKindNonStaticRef13 = 0x1 << 17,
    UseKindNonStaticRef14 = 0x1 << 18,
    UseKindNonStaticRef15 = 0x1 << 19,
    UseKindNonStaticRef   = (UseKindNonStaticRef0  |
                             UseKindNonStaticRef1  |
                             UseKindNonStaticRef2  |
                             UseKindNonStaticRef3  |
                             UseKindNonStaticRef4  |
                             UseKindNonStaticRef5  |
                             UseKindNonStaticRef6  |
                             UseKindNonStaticRef7  |
                             UseKindNonStaticRef8  |
                             UseKindNonStaticRef9  |
                             UseKindNonStaticRef10 |
                             UseKindNonStaticRef11 |
                             UseKindNonStaticRef12 |
                             UseKindNonStaticRef13 |
                             UseKindNonStaticRef14 |
                             UseKindNonStaticRef15),

    /* Constants */
    UseKindConstRef       = 0x1 << 20,

    /* Other types */
    UseKindParentRef      = 0x1 << 21,
    UseKindInclude        = 0x1 << 22,
    UseKindClosure        = 0x1 << 23,
    UseKindAny            = (unsigned)-1
  };

  /* Assert the size and bit-consecutiveness of UseKindNonStaticRef */
  static_assert(BitCount<UseKindNonStaticRef>::value == 16,
                "UseKindNonStaticRef should have 16 bits set");
  static_assert(BitPhase<UseKindNonStaticRef>::value <= 2,
                "UseKindNonStaticRef set bits should be consecutive");

  static int GetNonStaticRefUseKind(unsigned int hash) {
    int res = ((int)UseKindNonStaticRef0) << (hash % 16);
    assert(res >= ((int)UseKindNonStaticRef0) &&
           res <= ((int)UseKindNonStaticRef15));
    return res;
  }

  enum Marks {
    MarkWaitingInQueue,
    MarkProcessingDeps,
    MarkReady,
    MarkWaiting,
    MarkProcessing,
    MarkProcessedInQueue,
    MarkProcessed
  };

  BlockScope(const std::string &name, const std::string &docComment,
             StatementPtr stmt, KindOf kind);
  virtual ~BlockScope() {}
  bool is(KindOf kind) const { return kind == m_kind;}
  const std::string &getScopeName() const { return m_scopeName;}
  virtual bool isBuiltin() const { return false; }
  StatementPtr getStmt() const { return m_stmt;}
  VariableTableConstPtr getVariables() const { return m_variables;}
  ConstantTableConstPtr getConstants() const { return m_constants;}
  VariableTablePtr getVariables() { return m_variables;}
  ConstantTablePtr getConstants() { return m_constants;}
  ClassScopeRawPtr getContainingClass();
  FunctionScopeRawPtr getContainingNonClosureFunction();
  FunctionScopeRawPtr getContainingFunction() const {
    return FunctionScopeRawPtr(is(FunctionScope) ?
                               (HPHP::FunctionScope*)this : 0);
  }
  FileScopeRawPtr getContainingFile();
  AnalysisResultRawPtr getContainingProgram();

  ClassScopeRawPtr findExactClass(ClassScopeRawPtr cls);

  bool hasUser(BlockScopeRawPtr user, int useFlags) const;
  void addUse(BlockScopeRawPtr user, int useFlags);

  /**
   * Helpers for keeping track of break/continue nested level.
   */
  void incLoopNestedLevel();
  void decLoopNestedLevel();
  int getLoopNestedLevel() const { return m_loopNestedLevel;}

  /**
   * Helpers for parsing class functions and variables.
   */
  ModifierExpressionPtr setModifiers(ModifierExpressionPtr modifiers);
  ModifierExpressionPtr getModifiers() { return m_modifiers;}

  void setClassInfoAttribute(int flag) {
    m_attributeClassInfo |= flag;
  }
  const std::string &getDocComment() const { return m_docComment;}
  void setDocComment(const std::string &doc) { m_docComment = doc;}

  /**
   * Code gen
   */
  virtual void outputPHP(CodeGenerator &cg, AnalysisResultPtr ar);

  virtual bool inPseudoMain() const {
    return false;
  }

  virtual ClassScopePtr getParentScope(AnalysisResultConstPtr ar) const {
    return ClassScopePtr();
  }

  void setOuterScope(BlockScopePtr o) { m_outerScope = o; }
  BlockScopePtr getOuterScope() { return m_outerScope.lock(); }
  bool isOuterScope() { return m_outerScope.expired(); }

  const BlockScopeRawPtrFlagsPtrVec &getDeps() const {
    return m_orderedDeps;
  }
  const BlockScopeRawPtrFlagsVec &getOrderedUsers() const {
    return m_orderedUsers;
  }

  void setMark(Marks m) { m_mark = m;    }
  Marks getMark() const { return m_mark; }

  void setLockedMark(Marks m) { Lock l(s_jobStateMutex); m_mark = m;    }
  Marks getLockedMark() const { Lock l(s_jobStateMutex); return m_mark; }

  void incPass() { m_pass++; }
  bool isFirstPass() const { return !m_pass; }

  void incRunId() { m_runId++; }
  int getRunId() const { return m_runId; }

  void clearUpdated() { m_updated = 0; }
  void addUpdates(int f);
  int getUpdated() const { return m_updated; }

  void setRescheduleFlags(int rescheduleFlags) {
    m_rescheduleFlags = rescheduleFlags;
  }
  int rescheduleFlags() const { return m_rescheduleFlags; }

  void incEffectsTag() { m_effectsTag++; }
  int getEffectsTag() const { return m_effectsTag; }

  Mutex &getMutex() { return m_mutex; }

  void setNumDepsToWaitFor(int n) {
    assert(n >= 0);
    m_numDepsToWaitFor = n;
  }
  int getNumDepsToWaitFor() const {
    return m_numDepsToWaitFor;
  }
  int incNumDepsToWaitFor() {
    return ++m_numDepsToWaitFor;
  }
  int decNumDepsToWaitFor() {
    assert(m_numDepsToWaitFor > 0);
    return --m_numDepsToWaitFor;
  }

  void setForceRerun(bool v) { m_forceRerun = v; }
  bool forceRerun() const { return m_forceRerun; }

  int selfUser() const { return m_selfUser; }

protected:
  std::string m_scopeName;
  int m_attributeClassInfo;
  std::string m_docComment;
  StatementPtr m_stmt;
  KindOf m_kind;
  VariableTablePtr m_variables;
  ConstantTablePtr m_constants;
  BlockScopeRawPtr m_outerScope;

  int m_loopNestedLevel;
  ModifierExpressionPtr m_modifiers;
  StatementListPtr m_includes;
  int m_pass;
  int m_updated;
  int m_runId;
private:
  Marks m_mark;
  BlockScopeRawPtrFlagsPtrVec  m_orderedDeps;
  BlockScopeRawPtrFlagsVec     m_orderedUsers;
  BlockScopeRawPtrFlagsHashMap m_userMap;
  int m_effectsTag;
  int m_numDepsToWaitFor;
  Mutex m_mutex;
  bool m_forceRerun;      /* do we need to be re-run (allows deps to run during
                           * re-schedule) */
  int  m_rescheduleFlags; /* who do we need to run after a re-schedule */
  int  m_selfUser;
public:
  static Mutex s_jobStateMutex;
  static Mutex s_depsMutex;
  static Mutex s_constMutex;
};

///////////////////////////////////////////////////////////////////////////////
}
#endif // incl_HPHP_BLOCK_SCOPE_H_
