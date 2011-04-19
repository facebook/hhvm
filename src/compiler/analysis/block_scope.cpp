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

#include <compiler/expression/expression.h>

#include <compiler/analysis/block_scope.h>
#include <compiler/analysis/analysis_result.h>
#include <compiler/statement/statement_list.h>
#include <compiler/analysis/variable_table.h>
#include <compiler/analysis/constant_table.h>
#include <compiler/analysis/class_scope.h>
#include <compiler/analysis/function_scope.h>
#include <compiler/analysis/file_scope.h>

using namespace HPHP;
using namespace boost;

///////////////////////////////////////////////////////////////////////////////

Mutex BlockScope::s_jobStateMutex;
Mutex BlockScope::s_depsMutex;
Mutex BlockScope::s_constMutex;

BlockScope::BlockScope(const std::string &name, const std::string &docComment,
                       StatementPtr stmt, KindOf kind)
  : m_attributeClassInfo(0), m_docComment(docComment), m_stmt(stmt),
    m_kind(kind), m_loopNestedLevel(0),
    m_pass(0), m_updated(0), m_mark(MarkWaitingInQueue), m_changedScopes(0),
    m_effectsTag(1), m_forceRerun(false) {
  m_originalName = name;
  m_name = Util::toLower(name);
  m_variables = VariableTablePtr(new VariableTable(*this));
  m_constants = ConstantTablePtr(new ConstantTable(*this));

  Lock lock(SymbolTable::AllSymbolTablesMutex);
  SymbolTable::AllSymbolTables.push_back(m_variables);
  SymbolTable::AllSymbolTables.push_back(m_constants);
}

std::string BlockScope::getId(CodeGenerator &cg) const {
  return cg.formatLabel(getName());
}

void BlockScope::incLoopNestedLevel() {
  m_loopNestedLevel++;
}

void BlockScope::decLoopNestedLevel() {
  ASSERT(m_loopNestedLevel > 0);
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

ClassScopeRawPtr BlockScope::getContainingClass() {
  BlockScope *bs = this;
  if (bs->is(BlockScope::FunctionScope)) {
    bs = bs->m_outerScope.get();
  }
  if (bs && !bs->is(BlockScope::ClassScope)) {
    bs = 0;
  }
  return ClassScopeRawPtr((HPHP::ClassScope*)bs);
}

ClassScopePtr BlockScope::findExactClass(const std::string &className) {
  if (ClassScopePtr currentCls = getContainingClass()) {
    if (className == currentCls->getName()) {
      return currentCls;
    }
  }
  if (FileScopePtr currentFile = getContainingFile()) {
    StatementList &stmts = *currentFile->getStmt();
    for (int i = stmts.getCount(); i--; ) {
      StatementPtr s = stmts[i];
      if (s && s->is(Statement::KindOfClassStatement)) {
        ClassScopePtr scope =
          static_pointer_cast<ClassStatement>(s)->getClassScope();
        if (className == scope->getName()) {
          return scope;
        }
      }
    }
  }
  return ClassScopePtr();
}

void BlockScope::addUse(BlockScopeRawPtr user, int useKinds) {
  if (is(ClassScope) ? static_cast<HPHP::ClassScope*>(this)->isUserClass() :
      is(FunctionScope) &&
      static_cast<HPHP::FunctionScope*>(this)->isUserFunction()) {

    if (user.get() == this) return;

    Lock lock(s_depsMutex);
    Lock l2(s_jobStateMutex);
    std::pair<BlockScopeRawPtrFlagsHashMap::iterator,bool> val =
      m_userMap.insert(BlockScopeRawPtrFlagsHashMap::value_type(user,
                                                                useKinds));
    if (val.second) {
      m_orderedUsers.push_back(&*val.first);
      user->m_orderedDeps.push_back(BlockScopeRawPtr(this));
      if (user->getMark() == BlockScope::MarkReady ||
          user->getMark() == BlockScope::MarkWaiting) {
        if (getMark() != BlockScope::MarkProcessed) {
          user->setNumDepsToWaitFor(user->getNumDepsToWaitFor()+1);
        }
      }
    } else {
      val.first->second |= useKinds;
    }
  }
}

void BlockScope::addUpdates(int f) {
  if (!m_updated && m_changedScopes && getMark() != MarkProcessing) {
    m_changedScopes->push_back(BlockScopeRawPtr(this));
  }
  m_updated |= f;
}

void BlockScope::changed(BlockScopeRawPtrQueue &todo, int useKinds) {
  for (BlockScopeRawPtrFlagsVec::iterator it = m_orderedUsers.begin(),
         end = m_orderedUsers.end(); it != end; ++it) {
    BlockScopeRawPtrFlagsVec::value_type pf = *it;
    if (pf->second & useKinds && pf->first->getMark() >= MarkProcessedInQueue) {
      if (pf->first->getMark() == MarkProcessed) {
        todo.push_back(pf->first);
      }
      pf->first->setMark(MarkWaitingInQueue);
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

void BlockScope::outputCPP(CodeGenerator &cg, AnalysisResultPtr ar) {
  m_constants->outputCPP(cg, ar);
  m_variables->outputCPP(cg, ar);
}
