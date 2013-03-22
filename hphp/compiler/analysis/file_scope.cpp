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

#include <compiler/analysis/file_scope.h>
#include <compiler/analysis/code_error.h>
#include <compiler/analysis/analysis_result.h>
#include <compiler/analysis/class_scope.h>
#include <compiler/statement/statement_list.h>
#include <compiler/statement/exp_statement.h>
#include <compiler/option.h>
#include <compiler/analysis/constant_table.h>
#include <compiler/analysis/function_scope.h>
#include <sys/stat.h>
#include <compiler/parser/parser.h>
#include <util/logger.h>
#include <util/util.h>
#include <util/base.h>
#include <compiler/expression/expression_list.h>
#include <compiler/statement/function_statement.h>
#include <compiler/analysis/variable_table.h>
#include <compiler/expression/simple_function_call.h>
#include <compiler/expression/include_expression.h>
#include <compiler/expression/user_attribute.h>
#include <runtime/base/complex_types.h>

using namespace HPHP;

///////////////////////////////////////////////////////////////////////////////

FileScope::FileScope(const string &fileName, int fileSize, const MD5 &md5)
  : BlockScope("", "", StatementPtr(), BlockScope::FileScope),
    m_size(fileSize), m_md5(md5), m_module(false), m_privateInclude(false),
    m_externInclude(false),
    m_includeState(0), m_fileName(fileName), m_redeclaredFunctions(0) {
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

FunctionScopePtr FileScope::setTree(AnalysisResultConstPtr ar,
                                    StatementListPtr tree) {
  m_tree = tree;
  setFileLevel(tree);
  return createPseudoMain(ar);
}

void FileScope::cleanupForError(AnalysisResultConstPtr ar,
                                int line, const string &msg) {
  for (StringToClassScopePtrVecMap::const_iterator iter = m_classes.begin();
       iter != m_classes.end(); ++iter) {
    BOOST_FOREACH(ClassScopePtr cls, iter->second) {
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
  LocationPtr loc(new Location());
  loc->file = m_fileName.c_str();
  loc->first(line, 0);
  loc->last(line, 0);
  BlockScopePtr scope;
  ExpressionListPtr args(new ExpressionList(scope, loc));
  args->addElement(Expression::MakeScalarExpression(ar, scope, loc, msg));
  SimpleFunctionCallPtr e(
    new SimpleFunctionCall(scope, loc, "throw_fatal", args, ExpressionPtr()));
  e->setThrowFatal();
  ExpStatementPtr exp(new ExpStatement(scope, loc, e));
  StatementListPtr stmts(new StatementList(scope, loc));
  stmts->addElement(exp);

  FunctionScopePtr fs = setTree(ar, stmts);
  fs->setOuterScope(shared_from_this());
  fs->getStmt()->resetScope(fs);
  fs->getStmt()->setLocation(loc);
  setOuterScope(const_cast<AnalysisResult*>(ar.get())->shared_from_this());
}

bool FileScope::addFunction(AnalysisResultConstPtr ar,
                            FunctionScopePtr funcScope) {
  if (ar->declareFunction(funcScope)) {
    FunctionScopePtr &fs = m_functions[funcScope->getName()];
    if (fs) {
      if (!m_redeclaredFunctions) {
        m_redeclaredFunctions = new StringToFunctionScopePtrVecMap;
      }
      FunctionScopePtrVec &funcVec =
        (*m_redeclaredFunctions)[funcScope->getName()];
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
    m_classes[classScope->getName()].push_back(classScope);
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
    BOOST_FOREACH(ClassScopePtr cls, iter->second) {
      total += cls->getFunctionCount();
    }
  }
  return total;
}

void FileScope::countReturnTypes(std::map<std::string, int> &counts) {
  FunctionContainer::countReturnTypes(counts, m_redeclaredFunctions);
  for (StringToClassScopePtrVecMap::const_iterator iter = m_classes.begin();
       iter != m_classes.end(); ++iter) {
    BOOST_FOREACH(ClassScopePtr cls, iter->second) {
      cls->countReturnTypes(counts, 0);
    }
  }
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

bool FileScope::needPseudoMainVariables() const {
  VariableTablePtr variables = m_pseudoMain->getVariables();
  return
    variables->getAttribute(VariableTable::ContainsDynamicVariable) ||
    variables->getSymbols().size() > 0;
}

///////////////////////////////////////////////////////////////////////////////

ExpressionPtr FileScope::getEffectiveImpl(AnalysisResultConstPtr ar) const {
  if (m_tree) return m_tree->getEffectiveImpl(ar);
  return ExpressionPtr();
}

///////////////////////////////////////////////////////////////////////////////

void FileScope::declareConstant(AnalysisResultPtr ar, const string &name) {
  if (!ar->declareConst(shared_from_this(), name)) {
    addConstantDependency(ar, name);
  }
}

void FileScope::addConstant(const string &name, TypePtr type,
                            ExpressionPtr value,
                            AnalysisResultPtr ar, ConstructPtr con) {
  BlockScopePtr f = ar->findConstantDeclarer(name);
  f->getConstants()->add(name, type, value, ar, con);
}

void FileScope::addIncludeDependency(AnalysisResultPtr ar,
                                     const string &file, bool byInlined) {
  ar->addIncludeDependency(shared_from_this(), file);
}
void FileScope::addClassDependency(AnalysisResultPtr ar,
                                   const string &classname) {
  ar->addClassDependency(shared_from_this(), classname);
}
void FileScope::addFunctionDependency(AnalysisResultPtr ar,
                                      const string &funcname, bool byInlined) {
  ar->addFunctionDependency(shared_from_this(), funcname);
}
void FileScope::addConstantDependency(AnalysisResultPtr ar,
                                      const string &decname) {
  ar->addConstantDependency(shared_from_this(), decname);
}

void FileScope::analyzeProgram(AnalysisResultPtr ar) {
  if (m_pseudoMain) {
    m_pseudoMain->getStmt()->analyzeProgram(ar);
  }
}

ClassScopeRawPtr FileScope::resolveClass(ClassScopeRawPtr cls) {
  BlockScopeSet::iterator it = m_providedDefs.find(cls);
  if (it != m_providedDefs.end()) {
    return ClassScopeRawPtr(static_cast<HPHP::ClassScope*>(it->get()));
  }
  return ClassScopeRawPtr();
}

bool FileScope::checkClass(const string &cls) {
  return m_redecBases.find(cls) != m_redecBases.end();
}

FunctionScopeRawPtr FileScope::resolveFunction(FunctionScopeRawPtr func) {
  BlockScopeSet::iterator it = m_providedDefs.find(func);
  if (it != m_providedDefs.end()) {
    return FunctionScopeRawPtr(static_cast<HPHP::FunctionScope*>(it->get()));
  }
  return FunctionScopeRawPtr();
}

/*
 * Insert the class, and its parents (recursively) into m_providedDefs,
 * returning true if the class was already known to exist.
 * If def is true, this is the actual definition for the class,
 * so set the hasBase flags.
 */
bool FileScope::insertClassUtil(AnalysisResultPtr ar,
                                ClassScopeRawPtr cls, bool def) {
  if (!m_providedDefs.insert(cls).second) return true;

  const vector<string> &bases = cls->getBases();
  bool topBasesKnown = def && bases.size() > 31;
  for (unsigned i = 0; i < bases.size(); i++) {
    const string &s = bases[i];
    ClassScopeRawPtr c = ar->findClass(s);
    if (c) {
      if (c->isRedeclaring()) {
        ClassScopeRawPtr cr = resolveClass(c);
        if (!cr) {
          if (i >= 31) topBasesKnown = false;
          m_redecBases.insert(c->getName());
          continue;
        }
        c = cr;
      } else if (!c->isVolatile()) {
        if (def && i < 31) cls->setKnownBase(i);
        continue;
      }
      if (insertClassUtil(ar, c, false)) {
        if (def && i < 31) cls->setKnownBase(i);
      } else {
        if (i >= 31) topBasesKnown = false;
      }
    }
  }

  if (topBasesKnown) cls->setKnownBase(31);
  return false;
}

void FileScope::analyzeIncludesHelper(AnalysisResultPtr ar) {
  m_includeState = 1;
  if (m_pseudoMain) {
    StatementList &stmts = *getStmt();
    bool hoistOnly = false;
    for (int i = 0, n = stmts.getCount(); i < n; i++) {
      StatementPtr s = stmts[i];
      if (!s) continue;
      if (s->is(Statement::KindOfClassStatement) ||
          s->is(Statement::KindOfInterfaceStatement)) {

        ClassScopeRawPtr cls(
          static_pointer_cast<InterfaceStatement>(s)->getClassScope());
        if (hoistOnly) {
          const string &parent = cls->getOriginalParent();
          if (cls->getBases().size() > (parent.empty() ? 0 : 1)) {
            continue;
          }
          if (!parent.empty()) {
            ClassScopeRawPtr c = ar->findClass(parent);
            if (!c || (c->isVolatile() &&
                       !resolveClass(c) && !checkClass(parent))) {
              continue;
            }
          }
        }
        if (cls->isVolatile()) {
          insertClassUtil(ar, cls, true);
        }
        continue;
      }
      if (s->is(Statement::KindOfFunctionStatement)) {
        FunctionScopeRawPtr func(
          static_pointer_cast<FunctionStatement>(s)->getFunctionScope());
        if (func->isVolatile()) m_providedDefs.insert(func);
        continue;
      }
      if (!hoistOnly && s->is(Statement::KindOfExpStatement)) {
        ExpressionRawPtr exp(
          static_pointer_cast<ExpStatement>(s)->getExpression());
        if (exp && exp->is(Expression::KindOfIncludeExpression)) {
          FileScopeRawPtr fs(
            static_pointer_cast<IncludeExpression>(exp)->getIncludedFile(ar));
          if (fs && fs->m_includeState != 1) {
            if (!fs->m_includeState) {
              if (m_module && fs->m_privateInclude) {
                BOOST_FOREACH(BlockScopeRawPtr bs, m_providedDefs) {
                  fs->m_providedDefs.insert(bs);
                }
              }
              fs->analyzeIncludesHelper(ar);
            }
            BOOST_FOREACH(BlockScopeRawPtr bs, fs->m_providedDefs) {
              m_providedDefs.insert(bs);
            }
            continue;
          }
        }
      }
      hoistOnly = true;
    }
  }
  m_includeState = 2;
}

void FileScope::analyzeIncludes(AnalysisResultPtr ar) {
  if (!m_privateInclude && !m_includeState) {
    analyzeIncludesHelper(ar);
  }
}


void FileScope::visit(AnalysisResultPtr ar,
                      void (*cb)(AnalysisResultPtr, StatementPtr, void*),
                      void *data)
{
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
  FunctionStatementPtr f
    (new FunctionStatement(BlockScopePtr(), LocationPtr(),
                           ModifierExpressionPtr(),
                           false, pseudoMainName(),
                           ExpressionListPtr(), "", st, 0, "",
                           ExpressionListPtr()));
  f->setFileLevel();
  FunctionScopePtr pseudoMain(
    new HPHP::FunctionScope(ar, true,
                            pseudoMainName().c_str(),
                            f, false, 0, 0,
                            ModifierExpressionPtr(),
                            m_attributes[0], "",
                            shared_from_this(),
                            vector<UserAttributePtr>(),
                            true));
  f->setBlockScope(pseudoMain);
  FunctionScopePtr &fs = m_functions[pseudoMainName()];
  always_assert(!fs);
  fs = pseudoMain;
  m_pseudoMain = pseudoMain;
  return pseudoMain;
}

string FileScope::outputFilebase() const {
  string file = m_fileName;
  string out;
  if (file.size() > 4 && file.substr(file.length() - 4) == ".php") {
    out = file.substr(0, file.length() - 4);
  } else {
    out = file + ".nophp";
  }
  return Option::MangleFilename(out, false);
}

static void getFuncScopesSet(BlockScopeRawPtrQueue &v,
                             const StringToFunctionScopePtrMap &funcMap) {
  for (StringToFunctionScopePtrMap::const_iterator
         iter = funcMap.begin(), end = funcMap.end();
       iter != end; ++iter) {
    FunctionScopePtr f = iter->second;
    if (f->isUserFunction()) {
      v.push_back(f);
    }
  }
}

void FileScope::getScopesSet(BlockScopeRawPtrQueue &v) {
  const StringToClassScopePtrVecMap &classes = getClasses();
  for (StringToClassScopePtrVecMap::const_iterator iter = classes.begin(),
         end = classes.end(); iter != end; ++iter) {
    for (ClassScopePtrVec::const_iterator it = iter->second.begin(),
           e = iter->second.end(); it != e; ++it) {
      ClassScopePtr cls = *it;
      if (cls->isUserClass()) {
        v.push_back(cls);
        getFuncScopesSet(v, cls->getFunctions());
      }
    }
  }

  getFuncScopesSet(v, getFunctions());
  if (const StringToFunctionScopePtrVecMap *redec = m_redeclaredFunctions) {
    for (StringToFunctionScopePtrVecMap::const_iterator iter = redec->begin(),
           end = redec->end(); iter != end; ++iter) {
      FunctionScopePtrVec::const_iterator i = iter->second.begin(),
        e = iter->second.end();
      v.insert(v.end(), ++i, e);
    }
  }
}

void FileScope::getClassesFlattened(ClassScopePtrVec &classes) const {
  for (StringToClassScopePtrVecMap::const_iterator it = m_classes.begin();
       it != m_classes.end(); ++it) {
    BOOST_FOREACH(ClassScopePtr cls, it->second) {
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

