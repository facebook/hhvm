/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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
#include <utility>

#include "hphp/compiler/analysis/analysis_result.h"
#include "hphp/compiler/expression/expression.h"
#include "hphp/compiler/statement/statement_list.h"
#include "hphp/compiler/analysis/class_scope.h"
#include "hphp/compiler/analysis/function_scope.h"
#include "hphp/compiler/analysis/file_scope.h"
#include "hphp/util/text-util.h"

using namespace HPHP;

///////////////////////////////////////////////////////////////////////////////

Mutex BlockScope::s_constMutex;

BlockScope::BlockScope(const std::string &name, const std::string &docComment,
                       StatementPtr stmt, KindOf kind)
  : m_docComment(docComment), m_stmt(stmt), m_kind(kind), m_pass(0) {
  m_scopeName = name;
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

void BlockScope::outputPHP(CodeGenerator& /*cg*/, AnalysisResultPtr /*ar*/) {}
