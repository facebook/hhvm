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
#include "hphp/compiler/analysis/file_scope.h"

#include <sys/stat.h>
#include <folly/ScopeGuard.h>
#include <map>

#include "hphp/compiler/analysis/analysis_result.h"
#include "hphp/compiler/analysis/class_scope.h"
#include "hphp/compiler/analysis/code_error.h"
#include "hphp/compiler/analysis/constant_table.h"
#include "hphp/compiler/analysis/function_scope.h"
#include "hphp/compiler/analysis/lambda_names.h"
#include "hphp/compiler/analysis/variable_table.h"

#include "hphp/compiler/expression/expression_list.h"
#include "hphp/compiler/expression/include_expression.h"
#include "hphp/compiler/expression/simple_function_call.h"
#include "hphp/compiler/expression/user_attribute.h"

#include "hphp/compiler/option.h"

#include "hphp/compiler/parser/parser.h"

#include "hphp/compiler/statement/exp_statement.h"
#include "hphp/compiler/statement/function_statement.h"
#include "hphp/compiler/statement/statement_list.h"

#include "hphp/util/logger.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

FileScope::FileScope(const string &fileName, int fileSize, const MD5 &md5)
  : BlockScope("", "", StatementPtr(), BlockScope::FileScope),
    m_size(fileSize), m_md5(md5), m_system(false),
    m_isHHFile(false), m_preloadPriority(0),
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
      ExpStatementPtr expStmt = dynamic_pointer_cast<ExpStatement>(stmt);
      ExpressionPtr exp = expStmt->getExpression();
      exp->setFileLevel();
    }
    if (stmt->is(Statement::KindOfStatementList)) {
      setFileLevel(dynamic_pointer_cast<StatementList>(stmt));
    }
  }
}

void FileScope::setSystem() {
  m_fileName = "/:" + m_fileName;
  m_system = true;
}

void FileScope::setHHFile() {
  m_isHHFile = true;
}

FunctionScopePtr FileScope::setTree(AnalysisResultConstPtr ar,
                                    StatementListPtr tree) {
  m_tree = tree;
  setFileLevel(tree);
  return createPseudoMain(ar);
}

void FileScope::cleanupForError(AnalysisResultConstPtr ar) {
  for (StringToClassScopePtrVecMap::const_iterator iter = m_classes.begin();
       iter != m_classes.end(); ++iter) {
    for (ClassScopePtr cls: iter->second) {
      cls->getVariables()->cleanupForError(ar);
    }
  }

  getConstants()->cleanupForError(ar);

  StringToFunctionScopePtrMap().swap(m_functions);
  delete m_redeclaredFunctions;
  m_redeclaredFunctions = 0;
  StringToClassScopePtrVecMap().swap(m_classes);
  m_pseudoMain.reset();
  m_tree.reset();
}

template <class Meth>
void makeFatalMeth(FileScope& file,
                   AnalysisResultConstPtr ar,
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

void FileScope::makeFatal(AnalysisResultConstPtr ar,
                          const std::string& msg,
                          int line) {
  auto meth = [](SimpleFunctionCallPtr e) { e->setThrowFatal(); };
  makeFatalMeth(*this, ar, msg, line, meth);
}

void FileScope::makeParseFatal(AnalysisResultConstPtr ar,
                               const std::string& msg,
                               int line) {
  auto meth = [](SimpleFunctionCallPtr e) { e->setThrowParseFatal(); };
  makeFatalMeth(*this, ar, msg, line, meth);
}

bool FileScope::addFunction(AnalysisResultConstPtr ar,
                            FunctionScopePtr funcScope) {
  if (ar->declareFunction(funcScope)) {
    FunctionScopePtr &fs = m_functions[funcScope->getScopeName()];
    if (fs) {
      if (!m_redeclaredFunctions) {
        m_redeclaredFunctions = new StringToFunctionScopePtrVecMap;
      }
      FunctionScopePtrVec &funcVec =
        (*m_redeclaredFunctions)[funcScope->getScopeName()];
      if (!funcVec.size()) {
        fs->setLocalRedeclaring();
        funcVec.push_back(fs);
      }
      funcScope->setLocalRedeclaring();
      funcVec.push_back(funcScope);
    } else {
      fs = funcScope;
    }
    return true;
  }
  return false;
}

bool FileScope::addClass(AnalysisResultConstPtr ar, ClassScopePtr classScope) {
  if (ar->declareClass(classScope)) {
    m_classes[classScope->getScopeName()].push_back(classScope);
    return true;
  }
  return false;
}

ClassScopePtr FileScope::getClass(const char *name) {
  StringToClassScopePtrVecMap::const_iterator iter = m_classes.find(name);
  if (iter == m_classes.end()) return ClassScopePtr();
  return iter->second.back();
}


int FileScope::getFunctionCount() const {
  int total = FunctionContainer::getFunctionCount();
  for (StringToClassScopePtrVecMap::const_iterator iter = m_classes.begin();
       iter != m_classes.end(); ++iter) {
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

int FileScope::getGlobalAttribute() const {
  assert(m_attributes.size() == 1);
  return m_attributes.back();
}

///////////////////////////////////////////////////////////////////////////////

ExpressionPtr FileScope::getEffectiveImpl(AnalysisResultConstPtr ar) const {
  if (m_tree) return m_tree->getEffectiveImpl(ar);
  return ExpressionPtr();
}

///////////////////////////////////////////////////////////////////////////////

void FileScope::declareConstant(AnalysisResultPtr ar, const string &name) {
  ar->declareConst(shared_from_this(), name);
}

void FileScope::addConstant(const string &name,
                            ExpressionPtr value,
                            AnalysisResultPtr ar, ConstructPtr con) {
  BlockScopePtr f = ar->findConstantDeclarer(name);
  f->getConstants()->add(name, value, ar, con);
}

void FileScope::analyzeProgram(AnalysisResultPtr ar) {
  if (!m_pseudoMain) return;
  m_pseudoMain->getStmt()->analyzeProgram(ar);

  resolve_lambda_names(ar, shared_from_this());
}

void FileScope::visit(AnalysisResultPtr ar,
                      void (*cb)(AnalysisResultPtr, StatementPtr, void*),
                      void *data) {
  if (m_pseudoMain) {
    cb(ar, m_pseudoMain->getStmt(), data);
  }
}

const string &FileScope::pseudoMainName() {
  if (m_pseudoMainName.empty()) {
    m_pseudoMainName = Option::MangleFilename(m_fileName, true);
  }
  return m_pseudoMainName;
}

FunctionScopePtr FileScope::createPseudoMain(AnalysisResultConstPtr ar) {
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
      vector<UserAttributePtr>(),
      true);
  f->setBlockScope(pseudoMain);
  auto& fs = m_functions[pseudoMainName()];
  always_assert(!fs);
  fs = pseudoMain;
  m_pseudoMain = pseudoMain;
  return pseudoMain;
}

static void getFuncScopesSet(BlockScopeRawPtrQueue &v,
                             const StringToFunctionScopePtrMap &funcMap) {
  for (const auto& iter : funcMap) {
    FunctionScopePtr f = iter.second;
    if (!f->isBuiltin()) {
      v.push_back(f);
    }
  }
}

void FileScope::getScopesSet(BlockScopeRawPtrQueue &v) {
  for (const auto& clsVec : getClasses()) {
    for (const auto cls : clsVec.second) {
      if (cls->getStmt()) {
        v.push_back(cls);
        getFuncScopesSet(v, cls->getFunctions());
      }
    }
  }

  getFuncScopesSet(v, getFunctions());
  if (const auto redec = m_redeclaredFunctions) {
    for (const auto& funcVec : *redec) {
      auto i = funcVec.second.begin(), e = funcVec.second.end();
      v.insert(v.end(), ++i, e);
    }
  }
}

void FileScope::getClassesFlattened(ClassScopePtrVec &classes) const {
  for (const auto& clsVec : m_classes) {
    for (auto cls : clsVec.second) {
      classes.push_back(cls);
    }
  }
}

void FileScope::serialize(JSON::DocTarget::OutputStream &out) const {
  JSON::DocTarget::MapStream ms(out);
  ms.add("name", getName());

  ClassScopePtrVec classes;
  getClassesFlattened(classes);
  ms.add("classes", classes);

  FunctionScopePtrVec funcs;
  getFunctionsFlattened(m_redeclaredFunctions, funcs, true);
  ms.add("functions", funcs);

  // TODO(stephentu): constants

  ms.done();
}

}
