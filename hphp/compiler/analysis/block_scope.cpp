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


#include "hphp/compiler/analysis/block_scope.h"

#include "hphp/compiler/analysis/analysis_result.h"
#include "hphp/compiler/expression/expression.h"
#include "hphp/compiler/statement/statement_list.h"
#include "hphp/compiler/analysis/variable_table.h"
#include "hphp/compiler/analysis/constant_table.h"
#include "hphp/compiler/analysis/class_scope.h"
#include "hphp/compiler/analysis/function_scope.h"
#include "hphp/compiler/analysis/file_scope.h"

using namespace HPHP;

///////////////////////////////////////////////////////////////////////////////

Mutex BlockScope::s_jobStateMutex;
Mutex BlockScope::s_depsMutex;
Mutex BlockScope::s_constMutex;

BlockScope::BlockScope(const std::string &name, const std::string &docComment,
                       StatementPtr stmt, KindOf kind)
  : m_attributeClassInfo(0), m_docComment(docComment), m_stmt(stmt),
    m_kind(kind), m_loopNestedLevel(0),
    m_pass(0), m_updated(0), m_runId(0), m_mark(MarkWaitingInQueue),
    m_effectsTag(1), m_numDepsToWaitFor(0),
    m_forceRerun(false), m_inTypeInference(false),
    m_inVisitScopes(false), m_needsReschedule(false),
    m_rescheduleFlags(0), m_selfUser(0) {
  m_originalName = name;
  m_name = Util::toLower(name);
  m_variables = VariableTablePtr(new VariableTable(*this));
  m_constants = ConstantTablePtr(new ConstantTable(*this));

  Lock lock(SymbolTable::AllSymbolTablesMutex);
  SymbolTable::AllSymbolTables.push_back(m_variables);
  SymbolTable::AllSymbolTables.push_back(m_constants);
}

std::string BlockScope::getId() const {
  return CodeGenerator::FormatLabel(getName());
}

void BlockScope::incLoopNestedLevel() {
  m_loopNestedLevel++;
}

void BlockScope::decLoopNestedLevel() {
  assert(m_loopNestedLevel > 0);
  m_loopNestedLevel--;
}

FileScopeRawPtr BlockScope::getContainingFile() {
  BlockScope *bs = this;
  while (bs) {
    if (bs->is(BlockScope::FileScope)) {
      break;
    }
    bs = bs->getOuterScope().get();
  }

  return FileScopeRawPtr((HPHP::FileScope*)bs);
}

AnalysisResultRawPtr BlockScope::getContainingProgram() {
  BlockScope *bs = this;
  while (bs) {
    if (bs->is(BlockScope::ProgramScope)) {
      break;
    }
    bs = bs->getOuterScope().get();
  }

  return AnalysisResultRawPtr((AnalysisResult*)bs);
}

FunctionScopeRawPtr BlockScope::getContainingNonClosureFunction() {
  BlockScope *bs = this;
  // walk out through all the closures
  while (bs && bs->is(BlockScope::FunctionScope)) {
    HPHP::FunctionScope *fs = static_cast<HPHP::FunctionScope*>(bs);
    if (!fs->isClosure()) {
      return FunctionScopeRawPtr(fs);
    }
    bs = bs->m_outerScope.get();
  }
  return FunctionScopeRawPtr();
}

ClassScopeRawPtr BlockScope::getContainingClass() {
  BlockScope *bs = getContainingNonClosureFunction().get();
  if (!bs) {
    bs = this;
  }
  if (bs && bs->is(BlockScope::FunctionScope)) {
    bs = bs->m_outerScope.get();
  }
  if (!bs || !bs->is(BlockScope::ClassScope)) {
    return ClassScopeRawPtr();
  }
  return ClassScopeRawPtr((HPHP::ClassScope*)bs);
}

ClassScopeRawPtr BlockScope::findExactClass(ClassScopeRawPtr cls) {
  if (ClassScopeRawPtr currentCls = getContainingClass()) {
    if (cls->getName() == currentCls->getName()) {
      return currentCls;
    }
  }
  if (FileScopePtr currentFile = getContainingFile()) {
    return currentFile->resolveClass(cls);
  }
  return ClassScopeRawPtr();
}

FunctionScopeRawPtr BlockScope::findExactFunction(FunctionScopeRawPtr func) {
  if (FileScopePtr currentFile = getContainingFile()) {
    return currentFile->resolveFunction(func);
  }
  return FunctionScopeRawPtr();
}

bool BlockScope::hasUser(BlockScopeRawPtr user, int useKinds) const {
  if (is(ClassScope) ?
        static_cast<const HPHP::ClassScope*>(this)->isUserClass() :
        is(FunctionScope) &&
        static_cast<const HPHP::FunctionScope*>(this)->isUserFunction()) {

    if (user.get() == this) {
      return m_selfUser & useKinds;
    }

    Lock lock(s_depsMutex);
    BlockScopeRawPtrFlagsHashMap::const_iterator it =
      m_userMap.find(user);
    return it != m_userMap.end() && it->second & useKinds;
  }
  return true; // builtins/systems always have a user of anybody
}

void BlockScope::addUse(BlockScopeRawPtr user, int useKinds) {
  if ((is(ClassScope) || is(FunctionScope)) && getStmt()) {

    if (user.get() == this) {
      m_selfUser |= useKinds;
      return;
    }

    Lock lock(s_depsMutex);
    Lock l2(s_jobStateMutex);
    std::pair<BlockScopeRawPtrFlagsHashMap::iterator,bool> val =
      m_userMap.insert(BlockScopeRawPtrFlagsHashMap::value_type(user,
                                                                useKinds));
    if (val.second) {
      m_orderedUsers.push_back(&*val.first);
      user->m_orderedDeps.push_back(
          std::make_pair(BlockScopeRawPtr(this), &(val.first->second)));
      assert(user->getMark() != BlockScope::MarkReady &&
             user->getMark() != BlockScope::MarkWaiting);
    } else {
      val.first->second |= useKinds;
    }
  }
}

void BlockScope::addUpdates(int f) {
  assert(f > 0);
  if (inTypeInference()) {
    // we *must* have the mutex
    getInferTypesMutex().assertOwnedBySelf();

    m_updated |= f;

    // If this scope is currently being processed by this thread, don't bother
    // adding it to the updated map. otherwise, add it
    if (AnalysisResult::s_currentScopeThreadLocal->get() != this) {
      BlockScopeRawPtr self(this);
      std::pair< BlockScopeRawPtrFlagsHashMap::iterator, bool > val =
        AnalysisResult::s_changedScopesMapThreadLocal->insert(
          BlockScopeRawPtrFlagsHashMap::value_type(self, f));
      if (!val.second) {
        // not new, or the updated bits together
        val.first->second |= f;
      }
    }
  } else {
    m_updated |= f;
  }
}

void BlockScope::announceUpdates(int f) {
  assert(f > 0);
  if (inTypeInference()) {
    // If this scope is currently being processed by this thread, don't bother
    // adding it to the updated map. otherwise, add it
    if (AnalysisResult::s_currentScopeThreadLocal->get() != this) {
      BlockScopeRawPtr self(this);
      std::pair< BlockScopeRawPtrFlagsHashMap::iterator, bool > val =
        AnalysisResult::s_changedScopesMapThreadLocal->insert(
          BlockScopeRawPtrFlagsHashMap::value_type(self, f));
      if (!val.second) {
        // not new, or the updated bits together
        val.first->second |= f;
      }
    }
  }
}

ModifierExpressionPtr
BlockScope::setModifiers(ModifierExpressionPtr modifiers) {
  ModifierExpressionPtr oldModifiers = m_modifiers;
  m_modifiers = modifiers;
  return oldModifiers;
}

void BlockScope::inferTypes(AnalysisResultPtr ar) {
  if (m_stmt) {
    m_stmt->inferTypes(ar);
  }
}

void BlockScope::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  m_constants->outputPHP(cg, ar);
  m_variables->outputPHP(cg, ar);
}

int BlockScope::ScopeCompare::cmp(const BlockScopeRawPtr &p1,
                                  const BlockScopeRawPtr &p2) const {
  int d1 = p1->m_kind - p2->m_kind;
  if (d1) return d1;
  return strcasecmp(p1->getName().c_str(), p2->getName().c_str());
}
