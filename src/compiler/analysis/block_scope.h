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

#ifndef __BLOCK_SCOPE_H__
#define __BLOCK_SCOPE_H__

#include <compiler/hphp.h>
#include <util/lock.h>

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

typedef std::vector<BlockScopeRawPtr> BlockScopeRawPtrVec;
typedef std::list<BlockScopeRawPtr> BlockScopeRawPtrQueue;
typedef std::vector<BlockScopeRawPtrFlagsHashMap::
                    value_type*> BlockScopeRawPtrFlagsVec;

/**
 * Base class of ClassScope and FunctionScope.
 */
class BlockScope : public boost::enable_shared_from_this<BlockScope> {
public:
  enum KindOf {
    ClassScope,
    FunctionScope,
    FileScope,
    ProgramScope,
  };

  enum UseKinds {
    UseKindCaller = 1,
    UseKindStaticRef = 2,
    UseKindNonStaticRef = 4,
    UseKindConstRef = 8,
    UseKindParentRef = 16,
    UseKindInclude = 32,
    UseKindAny = (unsigned)-1
  };

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
  const std::string &getName() const { return m_name;}
  void setName(const std::string name) { m_name = name;}
  virtual std::string getId(CodeGenerator &cg) const;
  StatementPtr getStmt() const { return m_stmt;}
  VariableTableConstPtr getVariables() const { return m_variables;}
  ConstantTableConstPtr getConstants() const { return m_constants;}
  VariableTablePtr getVariables() { return m_variables;}
  ConstantTablePtr getConstants() { return m_constants;}
  ClassScopeRawPtr getContainingClass();
  FunctionScopeRawPtr getContainingFunction() const {
    return FunctionScopeRawPtr(is(FunctionScope) ?
                               (HPHP::FunctionScope*)this : 0);
  }
  FileScopeRawPtr getContainingFile();
  AnalysisResultRawPtr getContainingProgram();
  ClassScopePtr findExactClass(const std::string &className);

  void addUse(BlockScopeRawPtr user, int useFlags);
  void changed(BlockScopeRawPtrQueue &todo, int useKinds);


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
   * Triggers type inference of all statements inside this block.
   */
  void inferTypes(AnalysisResultPtr ar);

  /**
   * Generate constant and variable declarations.
   */
  virtual void outputPHP(CodeGenerator &cg, AnalysisResultPtr ar);
  virtual void outputCPP(CodeGenerator &cg, AnalysisResultPtr ar);

  virtual bool inPseudoMain() {
    return false;
  }

  virtual ClassScopePtr getParentScope(AnalysisResultConstPtr ar) {
    return ClassScopePtr();
  }

  void setOuterScope(BlockScopePtr o) { m_outerScope = o; }
  BlockScopePtr getOuterScope() { return m_outerScope.lock(); }
  bool isOuterScope() { return m_outerScope.expired(); }
  const BlockScopeRawPtrVec &getDeps() const { return m_orderedDeps; }
  const BlockScopeRawPtrFlagsVec &getOrderedUsers() const {
    return m_orderedUsers;
  }

  void setMark(Marks m) { m_mark = m; }
  Marks getMark() const { return m_mark; }

  void setLockedMark(Marks m) { Lock l(s_jobStateMutex); m_mark = m; }
  Marks getLockedMark() const { Lock l(s_jobStateMutex); return m_mark; }
  int getLockedNumDepsToWaitFor() const {
    Lock l(s_jobStateMutex); return m_numDepsToWaitFor;
  }
  void setPass(int p) { m_pass = p; }
  void incPass() { m_pass++; }
  int getPass() const { return m_pass; }
  bool isFirstPass() const { return !m_pass; }
  void clearUpdated() { m_updated = 0; }
  void addUpdates(int f);
  int getUpdated() const { return m_updated; }

  BlockScopeRawPtrQueue *getChangedScopes() const { return m_changedScopes; }
  void setChangedScopes(BlockScopeRawPtrQueue *scopes) {
    m_changedScopes = scopes;
  }
  void incEffectsTag() { m_effectsTag++; }
  int getEffectsTag() const { return m_effectsTag; }

  Mutex &getMutex() { return m_mutex; }
  void setNumDepsToWaitFor(int n) { m_numDepsToWaitFor = n; }
  int getNumDepsToWaitFor() const { return m_numDepsToWaitFor; }
  void setForceRerun(bool v) { m_forceRerun = v; }
  bool forceRerun() const { return m_forceRerun; }
protected:
  std::string m_originalName;
  std::string m_name;
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
private:
  Marks m_mark;
  BlockScopeRawPtrVec m_orderedDeps;
  BlockScopeRawPtrFlagsVec m_orderedUsers;
  BlockScopeRawPtrFlagsHashMap m_userMap;
  BlockScopeRawPtrQueue *m_changedScopes;
  int m_effectsTag;
  int m_numDepsToWaitFor;
  Mutex m_mutex;
  bool m_forceRerun;
public:
  static Mutex s_jobStateMutex;
  static Mutex s_depsMutex;
  static Mutex s_constMutex;
};

///////////////////////////////////////////////////////////////////////////////
}
#endif // __BLOCK_SCOPE_H__
