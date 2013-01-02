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
  ASSERT(!m_attributes.empty());
  m_attributes.back() |= attr;
}

int FileScope::popAttribute() {
  ASSERT(!m_attributes.empty());
  int ret = m_attributes.back();
  m_attributes.pop_back();
  return ret;
}

int FileScope::getGlobalAttribute() const {
  ASSERT(m_attributes.size() == 1);
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

bool FileScope::canUseDummyPseudoMain(AnalysisResultConstPtr ar) const {
  if (!m_pseudoMain) {
    return false;
  }
  ASSERT(!m_pseudoMain->isVolatile());
  if (!Option::GenerateDummyPseudoMain ||
      Option::KeepStatementsWithNoEffect) {
    return false;
  }
  ExpressionPtr exp = getEffectiveImpl(ar);
  Variant val;
  return (exp && exp->getScalarValue(val) && val.same(true));
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
  if (byInlined) m_usedIncludesInline.insert(file);
}
void FileScope::addClassDependency(AnalysisResultPtr ar,
                                   const string &classname) {
  ar->addClassDependency(shared_from_this(), classname);
}
void FileScope::addFunctionDependency(AnalysisResultPtr ar,
                                      const string &funcname, bool byInlined) {
  ar->addFunctionDependency(shared_from_this(), funcname);
  if (byInlined) m_usedFuncsInline.insert(funcname);
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
                           false, pseudoMainName(),
                           ExpressionListPtr(), st, 0, "",
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

void FileScope::outputCPPHelper(CodeGenerator &cg, AnalysisResultPtr ar,
                                bool classes /* = true */) {
  if (classes) {
    for (StringToClassScopePtrVecMap::iterator it = m_classes.begin();
         it != m_classes.end(); ++it) {
      BOOST_FOREACH(ClassScopePtr cls, it->second) {
        cls->getStmt()->outputCPP(cg, ar);
      }
    }
  }
  for (StringToFunctionScopePtrMap::iterator it = m_functions.begin();
       it != m_functions.end(); ++it) {
    FunctionScopePtr func = it->second;
    if (func->isLocalRedeclaring()) {
      BOOST_FOREACH(func, m_redeclaredFunctions->find(it->first)->second) {
        func->getStmt()->outputCPP(cg, ar);
      }
    } else {
      func->getStmt()->outputCPP(cg, ar);
    }
  }
}

void FileScope::outputCPPImpl(CodeGenerator &cg, AnalysisResultPtr ar) {
  ASSERT(cg.getContext() == CodeGenerator::CppImplementation);

  cg.setContext(CodeGenerator::CppConstantsDecl);
  getConstants()->outputCPP(cg, ar);

  cg.setContext(CodeGenerator::CppImplementation);
  cg_printf("/* preface starts */\n");

  for (StringToFunctionScopePtrMap::iterator it = m_functions.begin();
       it != m_functions.end(); ++it) {
    FunctionScopePtr func = it->second;
    if (func->isLocalRedeclaring()) {
      BOOST_FOREACH(func, m_redeclaredFunctions->find(it->first)->second) {
        func->outputCPPPreface(cg, ar);
      }
    } else {
      func->outputCPPPreface(cg, ar);
    }
  }
  BOOST_FOREACH(FunctionScopeRawPtr closure, m_usedClosures) {
    closure->outputCPPPreface(cg, ar);
  }

  for (StringToClassScopePtrVecMap::iterator it = m_classes.begin();
       it != m_classes.end(); ++it) {
    BOOST_FOREACH(ClassScopePtr cls, it->second) {
      const FunctionScopePtrVec &funcs = cls->getFunctionsVec();
      for (int i = 0, size = funcs.size(); i < size; ++i) {
        FunctionScopePtr func = funcs[i];
        func->outputCPPPreface(cg, ar);
      }
    }
  }

  cg_printf("/* preface finishes */\n");

  outputCPPHelper(cg, ar);

  if (Option::GenerateCPPMacros) {
    bool hasRedec;
    outputCPPJumpTableSupport(cg, ar, m_redeclaredFunctions, hasRedec);
    if (Option::EnableEval >= Option::LimitedEval) {
      outputCPPJumpTableEvalSupport(cg, ar, m_redeclaredFunctions, hasRedec);
    }
    outputCPPCallInfoTableSupport(cg, ar, m_redeclaredFunctions, hasRedec);
    outputCPPHelperClassAllocSupport(cg, ar, m_redeclaredFunctions);
    for (StringToClassScopePtrVecMap::iterator it = m_classes.begin();
         it != m_classes.end(); ++it) {
      BOOST_FOREACH(ClassScopePtr cls, it->second) {
        if (!cls->isInterface()) {
          cls->outputCPPDynamicClassImpl(cg, ar);
        }
      }
    }
  }
}

void FileScope::outputCPPPseudoMain(CodeGenerator &cg, AnalysisResultPtr ar) {
  ASSERT(cg.getContext() == CodeGenerator::CppPseudoMain);
  outputCPPHelper(cg, ar, false);
}

void FileScope::outputCPPForwardDeclarations(CodeGenerator &cg,
                                             AnalysisResultPtr ar) {
  cg.setContext(CodeGenerator::CppForwardDeclaration);

  BOOST_FOREACH(ClassScopeRawPtr cls, m_usedClassesFullHeader) {
    if (cls->isUserClass()) {
      cg_printInclude(cls->getHeaderFilename());
    }
  }

  bool first = true;
  BOOST_FOREACH(const string &str, m_usedLiteralStringsHeader) {
    int index = -1;
    int stringId = cg.checkLiteralString(str, index, ar, BlockScopePtr());
    always_assert(index != -1);
    string lisnam = ar->getLiteralStringName(stringId, index);
    if (!cg.ensureInNamespace() && first) cg_printf("\n");
    first = false;
    if (Option::UseStaticStringProxy) {
      string proxyNam = ar->getLiteralStringName(stringId, index, true);
      cg_printf("extern StaticStringProxy %s;\n", proxyNam.c_str());
      cg_printf("#ifndef %s\n", lisnam.c_str());
      cg_printf("#define %s (*(StaticString *)(&%s))\n",
                lisnam.c_str(), proxyNam.c_str());
      cg_printf("#endif\n");
    } else {
      cg_printf("extern StaticString %s;\n", lisnam.c_str());
    }
  }

  first = true;
  BOOST_FOREACH(const int64 &val, m_usedScalarVarIntegersHeader) {
    int index = -1;
    int hash = ar->checkScalarVarInteger(val, index);
    always_assert(index != -1);
    string name = ar->getScalarVarIntegerName(hash, index);
    if (!cg.ensureInNamespace() && first) cg_printf("\n");
    first = false;
    cg_printf("extern const VarNR &%s;\n", name.c_str());
  }

  first = true;
  BOOST_FOREACH(const double &val, m_usedScalarVarDoublesHeader) {
    int index = -1;
    int hash = ar->checkScalarVarDouble(val, index);
    always_assert(index != -1);
    string name = ar->getScalarVarDoubleName(hash, index);
    if (!cg.ensureInNamespace() && first) cg_printf("\n");
    first = false;
    cg_printf("extern const VarNR &%s;\n", name.c_str());
  }

  first = true;
  BOOST_FOREACH(const string &str, m_usedLitVarStringsHeader) {
    int index = -1;
    int stringId = cg.checkLiteralString(str, index, ar, BlockScopePtr());
    always_assert(index != -1);
    string lisnam = ar->getLitVarStringName(stringId, index);
    if (!cg.ensureInNamespace() && first) cg_printf("\n");
    first = false;
    if (Option::UseStaticStringProxy) {
      string proxyName = ar->getLitVarStringName(stringId, index, true);
      cg_printf("extern VariantProxy %s;\n", proxyName.c_str());
      cg_printf("#ifndef %s\n", lisnam.c_str());
      cg_printf("#define %s (*(Variant *)&%s)\n",
                lisnam.c_str(), proxyName.c_str());
      cg_printf("#endif\n");
    } else {
      cg_printf("extern VarNR %s;\n", lisnam.c_str());
    }
  }

  first = true;
  BOOST_FOREACH(const string &str, m_usedDefaultValueScalarArrays) {
    int index = -1;
    int hash = ar->checkScalarArray(str, index);
    always_assert(hash != -1 && index != -1);
    string name = ar->getScalarArrayName(hash, index);
    if (!cg.ensureInNamespace() && first) cg_printf("\n");
    first = false;
    cg_printf("extern StaticArray %s;\n", name.c_str());
  }

  first = true;
  BOOST_FOREACH(const string &str, m_usedDefaultValueScalarVarArrays) {
    int index = -1;
    int hash = ar->checkScalarArray(str, index);
    always_assert(hash != -1 && index != -1);
    string name = ar->getScalarVarArrayName(hash, index);
    if (!cg.ensureInNamespace() && first) cg_printf("\n");
    first = false;
    cg_printf("extern VarNR %s;\n", name.c_str());
  }

  first = true;
  BOOST_FOREACH(const string &str, m_usedConstsHeader) {
    BlockScopeConstPtr block = ar->findConstantDeclarer(str);
    always_assert(block);
    ConstantTableConstPtr constants = block->getConstants();
    if (!cg.ensureInNamespace() && first) cg_printf("\n");
    first = false;
    constants->outputSingleConstant(cg, ar, str);
  }


  first = true;
  BOOST_FOREACH(const CodeGenerator::UsedClassConst& item,
                m_usedClassConstsHeader) {
    ClassScopePtr cls = item.first;
    if (!cg.ensureInNamespace() && first) cg_printf("\n");
    first = false;
    cls->getConstants()->outputSingleConstant(cg, ar, item.second);
  }

  BOOST_FOREACH(ClassScopeRawPtr usedClass, m_usedClassesHeader) {
    if (!cg.ensureInNamespace() && first) cg_printf("\n");
    first = false;
    usedClass->outputForwardDeclaration(cg);
  }

  cg.ensureOutOfNamespace();
}

void FileScope::outputCPPDeclarations(CodeGenerator &cg,
                                      AnalysisResultPtr ar) {
  cg.printSection("Declarations");
  cg.setContext(CodeGenerator::CppDeclaration);

  if (cg.getOutput() == CodeGenerator::MonoCPP) {
    cg.namespaceBegin();
    outputCPPHelper(cg, ar);
    cg.namespaceEnd();
  } else {
    std::set<FileScopePtr> done;
    done.insert(shared_from_this());

    // Class declarations cunningly expressed as includes so I don't
    // have to worry about inheritance dependencies.
    for (StringToClassScopePtrVecMap::iterator it = m_classes.begin();
         it != m_classes.end(); ++it) {
      BOOST_FOREACH(ClassScopePtr cls, it->second) {
        cg_printInclude(cls->getHeaderFilename());
      }
    }

    BOOST_FOREACH(string name, m_usedFuncsInline) {
      FunctionScopePtr func = ar->findFunction(name);
      if (func) {
        FileScopePtr fs = func->getContainingFile();
        if (fs && done.find(fs) == done.end()) {
          done.insert(fs);
          cg_printInclude(fs->outputFilebase());
        }
      }
    }
    BOOST_FOREACH(string name, m_usedIncludesInline) {
      FileScopePtr fs = ar->findFileScope(name);
      if (fs && done.find(fs) == done.end()) {
        done.insert(fs);
        cg_printInclude(fs->outputFilebase());
      }
    }

    cg.namespaceBegin();
    cg.setContext(CodeGenerator::CppForwardDeclaration);
    cg.printSection("Includes and Functions", false);
    outputCPPHelper(cg, ar, false); // function forward declarations
    cg.setContext(CodeGenerator::CppDeclaration);
    outputCPPHelper(cg, ar, false); // function declarations (only inline)

    cg.printSection("Constants");
    if (cg.getOutput() != CodeGenerator::MonoCPP) {
      getConstants()->outputCPP(cg, ar, false);
    } else {
      cg_printf("// (omitted in MonoCPP mode)\n");
    }

    cg.namespaceEnd();
  }
}

void FileScope::outputCPPForwardStaticDecl(CodeGenerator &cg,
                                           AnalysisResultPtr ar) {
  string header = outputFilebase() + ".fws.h";
  cg.headerBegin(header);
  cg.namespaceBegin();
  cg.printSection("1. Static Strings", false);
  string str;
  BOOST_FOREACH(str, m_usedLiteralStrings) {
    if (m_usedLiteralStringsHeader.find(str) !=
        m_usedLiteralStringsHeader.end()) {
      continue;
    }
    int index = -1;
    int stringId = cg.checkLiteralString(str, index, ar, BlockScopePtr());
    always_assert(index != -1);
    string lisnam = ar->getLiteralStringName(stringId, index);
    if (Option::UseStaticStringProxy) {
      string proxyNam = ar->getLiteralStringName(stringId, index, true);
      cg_printf("extern StaticStringProxy %s;\n", proxyNam.c_str());
      cg_printf("#ifndef %s\n", lisnam.c_str());
      cg_printf("#define %s (*(StaticString *)(&%s))\n",
                lisnam.c_str(), proxyNam.c_str());
      cg_printf("#endif\n");
    } else {
      cg_printf("extern StaticString %s;\n", lisnam.c_str());
    }
  }
  cg_printf("\n");
  cg.printSection("2. Static Arrays", false);
  BOOST_FOREACH(str, m_usedScalarArrays) {
    if (m_usedDefaultValueScalarArrays.find(str) !=
        m_usedDefaultValueScalarArrays.end()) {
      continue;
    }
    int index = -1;
    int hash = ar->checkScalarArray(str, index);
    always_assert(hash != -1 && index != -1);
    string name = ar->getScalarArrayName(hash, index);
    cg_printf("extern StaticArray %s;\n", name.c_str());
  }
  if (Option::UseScalarVariant) {
    cg_printf("\n");
    cg.printSection("3. Static Variants", false);
    int64 val;
    BOOST_FOREACH(val, m_usedScalarVarIntegers) {
      if (m_usedScalarVarIntegersHeader.find(val) !=
          m_usedScalarVarIntegersHeader.end()) {
        continue;
      }
      int index = -1;
      int hash = ar->checkScalarVarInteger(val, index);
      always_assert(index != -1);
      string name = ar->getScalarVarIntegerName(hash, index);
      cg_printf("extern const VarNR &%s;\n", name.c_str());
    }
    cg_printf("\n");
    double dval;
    BOOST_FOREACH(dval, m_usedScalarVarDoubles) {
      if (m_usedScalarVarDoublesHeader.find(dval) !=
          m_usedScalarVarDoublesHeader.end()) {
        continue;
      }
      int index = -1;
      int hash = ar->checkScalarVarDouble(dval, index);
      always_assert(index != -1);
      string name = ar->getScalarVarDoubleName(hash, index);
      cg_printf("extern const VarNR &%s;\n", name.c_str());
    }
    cg_printf("\n");
    BOOST_FOREACH(str, m_usedLitVarStrings) {
      if (m_usedLitVarStringsHeader.find(str) !=
          m_usedLitVarStringsHeader.end()) {
        continue;
      }
      int index = -1;
      int stringId = cg.checkLiteralString(str, index, ar, BlockScopePtr());
      always_assert(index != -1);
      string lisnam = ar->getLitVarStringName(stringId, index);
      if (Option::UseStaticStringProxy) {
        string proxyName = ar->getLitVarStringName(stringId, index, true);
        cg_printf("extern VariantProxy %s;\n", proxyName.c_str());
        cg_printf("#ifndef %s\n", lisnam.c_str());
        cg_printf("#define %s (*(Variant *)&%s)\n",
                  lisnam.c_str(), proxyName.c_str());
        cg_printf("#endif\n");
      } else {
        cg_printf("extern VarNR %s;\n", lisnam.c_str());
      }
    }
    cg_printf("\n");
    BOOST_FOREACH(str, m_usedScalarVarArrays) {
      if (m_usedDefaultValueScalarVarArrays.find(str) !=
          m_usedDefaultValueScalarVarArrays.end()) {
        continue;
      }
      int index = -1;
      int hash = ar->checkScalarArray(str, index);
      always_assert(hash != -1 && index != -1);
      string name = ar->getScalarVarArrayName(hash, index);
      cg_printf("extern VarNR %s;\n", name.c_str());
    }
    cg_printf("\n");
  }
  cg.namespaceEnd();
  cg_printf("\n");
  cg.headerEnd(header);
}

void FileScope::outputCPPDeclHeader(CodeGenerator &cg, AnalysisResultPtr ar) {
  cg.setFileOrClassHeader(true);
  string header = outputFilebase() + ".h";
  cg.headerBegin(header);
  if (Option::GenerateCppLibCode ||
      cg.getOutput() == CodeGenerator::SystemCPP) {
    cg.printBasicIncludes();
  }
  outputCPPForwardDeclarations(cg, ar);
  outputCPPDeclarations(cg, ar);
  cg.headerEnd(header);
}

void FileScope::outputCPPClassHeaders(AnalysisResultPtr ar,
                                      CodeGenerator::Output output) {
  ClassScopePtr cls;
  for (StringToClassScopePtrVecMap::iterator it = m_classes.begin();
       it != m_classes.end(); ++it) {
    BOOST_FOREACH(ClassScopePtr cls, it->second) {
      cls->outputCPPHeader(ar, output);
    }
  }
}

void FileScope::outputCPPFFI(CodeGenerator &cg,
                             AnalysisResultPtr ar) {
  cg.setContext(CodeGenerator::CppFFIDecl);
  cg.useStream(CodeGenerator::PrimaryStream);
  outputCPPHelper(cg, ar, true);
  cg.useStream(CodeGenerator::ImplFile);
  cg.setContext(CodeGenerator::CppFFIImpl);
  cg_printInclude(outputFilebase() + ".h");
  outputCPPHelper(cg, ar, true);
}

void FileScope::outputHSFFI(CodeGenerator &cg,
                            AnalysisResultPtr ar) {
  cg.setContext(CodeGenerator::HsFFI);
  outputCPPHelper(cg, ar, false);
}

void FileScope::outputJavaFFI(CodeGenerator &cg, AnalysisResultPtr ar) {
  // first, generate methods in HphpMain.java
  for (StringToFunctionScopePtrMap::iterator it = m_functions.begin();
       it != m_functions.end(); ++it) {
    FunctionScopePtr func = it->second;
    if (func->isLocalRedeclaring()) {
      BOOST_FOREACH(func, m_redeclaredFunctions->find(it->first)->second) {
        func->getStmt()->outputCPP(cg, ar);
      }
    } else {
      func->getStmt()->outputCPP(cg, ar);
    }
  }

  // for each php class or interface, generate a Java file
  for (StringToClassScopePtrVecMap::iterator it = m_classes.begin();
       it != m_classes.end(); ++it) {
    BOOST_FOREACH(ClassScopePtr cls, it->second) {
      cls->getStmt()->outputCPP(cg, ar);
    }
  }
}

void FileScope::outputJavaFFICPPStub(CodeGenerator &cg,
                                     AnalysisResultPtr ar) {
  if (cg.getContext() == CodeGenerator::JavaFFICppImpl) {
    // java_stubs.h doesn't need the extra include
    cg_printInclude(outputFilebase() + ".h");
  }
  outputCPPHelper(cg, ar, true);
}

void FileScope::outputSwigFFIStubs(CodeGenerator &cg, AnalysisResultPtr ar) {
  // currently only support toplevel functions
  outputCPPHelper(cg, ar, false);
}
