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
#include "hphp/compiler/analysis/file_scope.h"

#include <sys/stat.h>
#include <folly/ScopeGuard.h>
#include <map>

#include "hphp/compiler/analysis/analysis_result.h"
#include "hphp/compiler/analysis/class_scope.h"
#include "hphp/compiler/analysis/function_scope.h"

#include "hphp/compiler/expression/closure_expression.h"
#include "hphp/compiler/expression/expression_list.h"
#include "hphp/compiler/expression/include_expression.h"
#include "hphp/compiler/expression/simple_function_call.h"
#include "hphp/compiler/expression/user_attribute.h"

#include "hphp/compiler/option.h"

#include "hphp/compiler/parser/parser.h"

#include "hphp/compiler/statement/exp_statement.h"
#include "hphp/compiler/statement/class_statement.h"
#include "hphp/compiler/statement/function_statement.h"
#include "hphp/compiler/statement/statement_list.h"

#include "hphp/util/logger.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

__thread FileScope* FileScope::s_current;

///////////////////////////////////////////////////////////////////////////////

FileScope::FileScope(const std::string &fileName, int fileSize)
  : BlockScope("", "", StatementPtr(), BlockScope::FileScope),
    m_size(fileSize), m_system(false),
    m_isHHFile(false), m_useStrictTypes(false),
    m_useStrictTypesForBuiltins(false), m_preloadPriority(0),
    m_fileName(fileName), m_redeclaredFunctions(0) {
  pushAttribute(); // for global scope
}

///////////////////////////////////////////////////////////////////////////////
// parser functions

void FileScope::setFileLevel(StatementListPtr stmtList) {
  for (int i = 0; i < stmtList->getCount(); i++) {
    StatementPtr stmt = (*stmtList)[i];
    stmt->setFileLevel();
    if (stmt->is(Statement::KindOfExpStatement)) {
      auto expStmt = dynamic_pointer_cast<ExpStatement>(stmt);
      ExpressionPtr exp = expStmt->getExpression();
      exp->setFileLevel();
    }
    if (stmt->is(Statement::KindOfStatementList)) {
      setFileLevel(dynamic_pointer_cast<StatementList>(stmt));
    }
  }
}

void FileScope::setSystem() {
  assertx(m_fileName[0] == '/' && m_fileName[1] == ':');
  m_system = true;
}

void FileScope::setHHFile() {
  m_isHHFile = true;
}

void FileScope::setUseStrictTypes() {
  m_useStrictTypes = true;
}

void FileScope::setUseStrictTypesForBuiltins() {
  m_useStrictTypesForBuiltins = true;
}


FunctionScopePtr FileScope::setTree(AnalysisResultConstRawPtr ar,
                                    StatementListPtr tree) {
  m_tree = tree;
  setFileLevel(tree);
  return createPseudoMain(ar);
}

void FileScope::cleanupForError(AnalysisResultConstRawPtr /*ar*/) {
  StringToFunctionScopePtrMap().swap(m_functions);
  delete m_redeclaredFunctions;
  m_redeclaredFunctions = 0;
  StringToClassScopePtrVecMap().swap(m_classes);
  m_pseudoMain.reset();
  m_tree.reset();
}

template <class Meth>
void makeFatalMeth(FileScope& file,
                   AnalysisResultConstRawPtr ar,
                   const std::string& msg,
                   int line,
                   Meth meth) {
  auto labelScope = std::make_shared<LabelScope>();
  auto r = Location::Range(line, 0, line, 0);
  BlockScopePtr scope;
  auto args = std::make_shared<ExpressionList>(scope, r);
  args->addElement(Expression::MakeScalarExpression(ar, scope, r, msg));
  auto e =
    std::make_shared<SimpleFunctionCall>(scope, r, "throw_fatal", false, args,
                                         ExpressionPtr());
  meth(e);
  auto exp = std::make_shared<ExpStatement>(scope, labelScope, r, e);
  auto stmts = std::make_shared<StatementList>(scope, labelScope, r);
  stmts->addElement(exp);

  FunctionScopePtr fs = file.setTree(ar, stmts);
  fs->setOuterScope(file.shared_from_this());
  fs->getStmt()->resetScope(fs);
  exp->copyLocationTo(fs->getStmt());
  file.setOuterScope(const_cast<AnalysisResult*>(ar.get())->shared_from_this());
}

void FileScope::makeFatal(AnalysisResultConstRawPtr ar,
                          const std::string& msg,
                          int line) {
  auto meth = [](SimpleFunctionCallPtr e) { e->setThrowFatal(); };
  makeFatalMeth(*this, ar, msg, line, meth);
}

void FileScope::makeParseFatal(AnalysisResultConstRawPtr ar,
                               const std::string& msg,
                               int line) {
  auto meth = [](SimpleFunctionCallPtr e) { e->setThrowParseFatal(); };
  makeFatalMeth(*this, ar, msg, line, meth);
}

void FileScope::addFunction(AnalysisResultConstRawPtr /*ar*/,
                            FunctionScopePtr funcScope) {
  FunctionScopePtr &fs = m_functions[funcScope->getScopeName()];
  if (!fs) {
    fs = funcScope;
    return;
  }

  if (!m_redeclaredFunctions) {
    m_redeclaredFunctions = new StringToFunctionScopePtrVecMap;
  }
  auto& funcVec = (*m_redeclaredFunctions)[funcScope->getScopeName()];
  if (!funcVec.size()) {
    fs->setLocalRedeclaring();
    funcVec.push_back(fs);
  }
  funcScope->setLocalRedeclaring();
  funcVec.push_back(funcScope);
}

void FileScope::addClass(AnalysisResultConstRawPtr /*ar*/,
                         ClassScopePtr classScope) {
  m_classes[classScope->getScopeName()].push_back(classScope);
}

int FileScope::getFunctionCount() const {
  int total = FunctionContainer::getFunctionCount();
  for (auto iter = m_classes.begin(); iter != m_classes.end(); ++iter) {
    for (ClassScopePtr cls: iter->second) {
      total += cls->getFunctionCount();
    }
  }
  return total;
}

void FileScope::pushAttribute() {
  m_attributes.push_back(0);
}

void FileScope::setAttribute(Attribute attr) {
  assert(!m_attributes.empty());
  m_attributes.back() |= attr;
}

int FileScope::popAttribute() {
  assert(!m_attributes.empty());
  int ret = m_attributes.back();
  m_attributes.pop_back();
  return ret;
}

///////////////////////////////////////////////////////////////////////////////

void FileScope::analyzeProgram(AnalysisResultConstRawPtr ar) {
  if (!m_pseudoMain) return;
  s_current = this;
  SCOPE_EXIT { s_current = nullptr; };
  ar->analyzeProgram(m_pseudoMain->getStmt());
  ClosureExpression::processLambdas(ar, std::move(m_lambdas));
}

void FileScope::visit(AnalysisResultPtr ar,
                      void (*cb)(AnalysisResultPtr, StatementPtr, void*),
                      void *data) {
  if (m_pseudoMain) {
    cb(ar, m_pseudoMain->getStmt(), data);
  }
}

const std::string &FileScope::pseudoMainName() {
  if (m_pseudoMainName.empty()) {
    m_pseudoMainName = Option::MangleFilename(m_fileName, true);
  }
  return m_pseudoMainName;
}

FunctionScopePtr FileScope::createPseudoMain(AnalysisResultConstRawPtr ar) {
  StatementListPtr st = m_tree;
  auto labelScope = std::make_shared<LabelScope>();
  auto f =
    std::make_shared<FunctionStatement>(
      BlockScopePtr(),
      labelScope,
      Location::Range(),
      ModifierExpressionPtr(),
      false, pseudoMainName(),
      ExpressionListPtr(), TypeAnnotationPtr(),
      st, 0, "", ExpressionListPtr());
  f->setFileLevel();
  auto pseudoMain =
    std::make_shared<HPHP::FunctionScope>(
      ar, true,
      pseudoMainName(),
      f, false, 0, 0,
      ModifierExpressionPtr(),
      m_attributes[0], "",
      shared_from_this(),
      std::vector<UserAttributePtr>(),
      true);
  f->setBlockScope(pseudoMain);
  auto& fs = m_functions[pseudoMainName()];
  always_assert(!fs);
  fs = pseudoMain;
  m_pseudoMain = pseudoMain;
  return pseudoMain;
}

}
