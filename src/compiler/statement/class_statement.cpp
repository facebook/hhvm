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

#include <compiler/statement/class_statement.h>
#include <util/parser/hphp.tab.hpp>
#include <compiler/expression/expression_list.h>
#include <compiler/statement/statement_list.h>
#include <compiler/expression/scalar_expression.h>
#include <compiler/analysis/class_scope.h>
#include <compiler/analysis/file_scope.h>
#include <compiler/analysis/function_scope.h>
#include <compiler/analysis/analysis_result.h>
#include <compiler/statement/method_statement.h>
#include <compiler/statement/class_variable.h>
#include <compiler/analysis/variable_table.h>
#include <compiler/analysis/constant_table.h>
#include <util/util.h>
#include <compiler/statement/interface_statement.h>
#include <compiler/statement/use_trait_statement.h>
#include <compiler/option.h>
#include <sstream>
#include <algorithm>

using namespace HPHP;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

ClassStatement::ClassStatement
(STATEMENT_CONSTRUCTOR_PARAMETERS,
 int type, const string &name, const string &parent,
 ExpressionListPtr base, const string &docComment, StatementListPtr stmt,
 ExpressionListPtr attrList)
  : InterfaceStatement(STATEMENT_CONSTRUCTOR_PARAMETER_VALUES(ClassStatement),
                       name, base, docComment, stmt, attrList),
    m_type(type), m_ignored(false) {
  m_parent = Util::toLower(parent);
  m_originalParent = parent;
}

StatementPtr ClassStatement::clone() {
  ClassStatementPtr stmt(new ClassStatement(*this));
  stmt->m_stmt = Clone(m_stmt);
  stmt->m_base = Clone(m_base);
  return stmt;
}

///////////////////////////////////////////////////////////////////////////////
// parser functions

void ClassStatement::onParse(AnalysisResultConstPtr ar, FileScopePtr fs) {
  ClassScope::KindOf kindOf = ClassScope::KindOfObjectClass;
  switch (m_type) {
  case T_CLASS:     kindOf = ClassScope::KindOfObjectClass;   break;
  case T_ABSTRACT:  kindOf = ClassScope::KindOfAbstractClass; break;
  case T_FINAL:     kindOf = ClassScope::KindOfFinalClass;    break;
  case T_TRAIT:     kindOf = ClassScope::KindOfTrait;         break;
  default:
    ASSERT(false);
  }

  vector<string> bases;
  if (!m_originalParent.empty()) {
    bases.push_back(m_originalParent);
  }
  if (m_base) m_base->getOriginalStrings(bases);

  vector<UserAttributePtr> attrs;
  if (m_attrList) {
    for (int i = 0; i < m_attrList->getCount(); ++i) {
      UserAttributePtr a =
        dynamic_pointer_cast<UserAttribute>((*m_attrList)[i]);
      attrs.push_back(a);
    }
  }

  StatementPtr stmt = dynamic_pointer_cast<Statement>(shared_from_this());
  ClassScopePtr classScope(new ClassScope(kindOf, m_originalName,
                                          m_originalParent,
                                          bases, m_docComment,
                                          stmt, attrs));
  setBlockScope(classScope);
  if (!fs->addClass(ar, classScope)) {
    m_ignored = true;
    return;
  }

  if (Option::PersistenceHook) {
    classScope->setPersistent(Option::PersistenceHook(classScope, fs));
  }

  if (m_stmt) {
    MethodStatementPtr constructor;

    // flatten continuation StatementList into MethodStatements
    for (int i = 0; i < m_stmt->getCount(); i++) {
      StatementListPtr stmts =
        dynamic_pointer_cast<StatementList>((*m_stmt)[i]);
      if (stmts) {
        m_stmt->removeElement(i);
        for (int j = 0; j < stmts->getCount(); j++) {
          m_stmt->insertElement((*stmts)[j], i + j);
        }
      }
    }

    for (int i = 0; i < m_stmt->getCount(); i++) {
      MethodStatementPtr meth =
        dynamic_pointer_cast<MethodStatement>((*m_stmt)[i]);
      if (meth && meth->getName() == "__construct") {
        constructor = meth;
        break;
      }
    }
    for (int i = 0; i < m_stmt->getCount(); i++) {
      if (!constructor) {
        MethodStatementPtr meth =
          dynamic_pointer_cast<MethodStatement>((*m_stmt)[i]);
        if (meth && meth->getName() == classScope->getName()
            && !classScope->isTrait()) {
          // class-name constructor
          constructor = meth;
          classScope->setAttribute(ClassScope::ClassNameConstructor);
        }
      }
      IParseHandlerPtr ph = dynamic_pointer_cast<IParseHandler>((*m_stmt)[i]);
      ph->onParseRecur(ar, classScope);
    }
    if (constructor && constructor->getModifiers()->isStatic()) {
      constructor->parseTimeFatal(Compiler::InvalidAttribute,
                                  "Constructor %s::%s() cannot be static",
                                  classScope->getOriginalName().c_str(),
                                  constructor->getOriginalName().c_str());
    }
  }
}

StatementPtr ClassStatement::addClone(StatementPtr origStmt) {
  ASSERT(m_stmt);
  StatementPtr newStmt = Clone(origStmt);
  MethodStatementPtr newMethStmt =
    dynamic_pointer_cast<MethodStatement>(newStmt);
  if (newMethStmt) {
    newMethStmt->setClassName(m_name);
    newMethStmt->setOriginalClassName(m_originalName);
  }
  m_stmt->addElement(newStmt);
  return newStmt;
}

///////////////////////////////////////////////////////////////////////////////
// static analysis functions

string ClassStatement::getName() const {
  return string("Class ") + getScope()->getName();
}

void ClassStatement::analyzeProgram(AnalysisResultPtr ar) {
  vector<string> bases;
  if (!m_parent.empty()) bases.push_back(m_parent);
  if (m_base) m_base->getStrings(bases);
  for (unsigned int i = 0; i < bases.size(); i++) {
    string className = bases[i];
    addUserClass(ar, bases[i]);
  }

  checkVolatile(ar);

  if (m_stmt) {
    m_stmt->analyzeProgram(ar);
  }

  if (ar->getPhase() != AnalysisResult::AnalyzeAll) return;

  ar->recordClassSource(m_name, m_loc, getFileScope()->getName());
  for (unsigned int i = 0; i < bases.size(); i++) {
    ClassScopePtr cls = ar->findClass(bases[i]);
    if (cls) {
      if ((!cls->isInterface() && (m_parent.empty() || i > 0 )) ||
          (cls->isInterface() && (!m_parent.empty() && i == 0 )) ||
          (cls->isTrait())) {
        Compiler::Error(Compiler::InvalidDerivation,
                        shared_from_this(),
                        "You are extending " + cls->getOriginalName() + 
                          " which is an interface or a trait");
      }
      if (cls->isUserClass()) {
        cls->addUse(getScope(), BlockScope::UseKindParentRef);
      }
    }
  }
}

void ClassStatement::inferTypes(AnalysisResultPtr ar) {
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void ClassStatement::getAllParents(AnalysisResultConstPtr ar,
                                   std::vector<std::string> &names) {
  if (!m_parent.empty()) {
    ClassScopePtr cls = ar->findClass(m_parent);
    if (cls) {
      if (!cls->isRedeclaring()) {
        cls->getAllParents(ar, names);
      }
      names.push_back(m_originalParent);
    }
  }

  if (m_base) {
    vector<string> bases;
    m_base->getStrings(bases);
    for (unsigned int i = 0; i < bases.size(); i++) {
      ClassScopePtr cls = ar->findClass(bases[i]);
      if (cls) {
        cls->getAllParents(ar, names);
        names.push_back(cls->getOriginalName());
      }
    }
  }
}

void ClassStatement::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  ClassScopeRawPtr classScope = getClassScope();
  if (!classScope->isUserClass()) return;

  if (m_type == T_TRAIT) {
    cg_printf("trait %s", m_originalName.c_str());
  } else {
    switch (m_type) {
      case T_CLASS:                              break;
      case T_ABSTRACT: cg_printf("abstract ");   break;
      case T_FINAL:    cg_printf("final ");      break;
      default:
        ASSERT(false);
    }
    cg_printf("class %s", m_originalName.c_str());
  }

  if (!m_parent.empty()) {
    cg_printf(" extends %s", m_originalParent.c_str());
  }

  if (m_base) {
    cg_printf(" implements ");
    m_base->outputPHP(cg, ar);
  }

  cg_indentBegin(" {\n");
  classScope->outputPHP(cg, ar);
  if (m_stmt) m_stmt->outputPHP(cg, ar);
  cg_indentEnd("}\n");
}

bool ClassStatement::hasImpl() const {
  ClassScopeRawPtr cls = getClassScope();
  return cls->isVolatile() ||
    cls->getVariables()->getAttribute(VariableTable::ContainsDynamicStatic) ||
    (hhvm && Option::OutputHHBC);
}

void ClassStatement::outputCPPClassDecl(CodeGenerator &cg,
                                        AnalysisResultPtr ar,
                                        const char *clsName,
                                        const char *originalName,
                                        const char *parent) {
  ClassScopeRawPtr classScope = getClassScope();
  VariableTablePtr variables = classScope->getVariables();
  ConstantTablePtr constants = classScope->getConstants();
  const char *sweep =
    classScope->isUserClass() && !classScope->isSepExtension() ?
    "_NO_SWEEP" : "";

  if (variables->hasAllJumpTables() &&
      classScope->hasAllJumpTables()) {
    cg_printf("DECLARE_CLASS%s(%s, %s, %s)\n",
              sweep, clsName,
              CodeGenerator::EscapeLabel(originalName).c_str(), parent);
    return;
  }

  // Now we start to break down DECLARE_CLASS into lines of code that could
  // be generated differently...

  cg_printf("DECLARE_CLASS_COMMON%s(%s, %s)\n", sweep,
            clsName, CodeGenerator::EscapeLabel(originalName).c_str());
}

void ClassStatement::GetCtorAndInitInfo(
    StatementPtr s, bool &needsCppCtor, bool &needsInit) {
  if (!s) return;
  switch (s->getKindOf()) {
  case Statement::KindOfStatementList:
    {
      StatementListPtr stmts = static_pointer_cast<StatementList>(s);
      for (int i = 0; i < stmts->getCount(); i++) {
        GetCtorAndInitInfo((*stmts)[i], needsCppCtor, needsInit);
      }
    }
    break;
  case Statement::KindOfClassVariable:
    {
      ClassVariablePtr cv = static_pointer_cast<ClassVariable>(s);
      cv->getCtorAndInitInfo(needsCppCtor, needsInit);
    }
    break;
  default: break;
  }
}

void ClassStatement::getCtorAndInitInfo(bool &needsCppCtor, bool &needsInit) {
  needsCppCtor = needsInit = false;
  ClassScopeRawPtr classScope = getClassScope();
  if (!m_parent.empty()) {
    if (classScope->derivesFromRedeclaring() ==
        ClassScope::DirectFromRedeclared) {
      needsInit = true;
    }
  }
  GetCtorAndInitInfo(m_stmt, needsCppCtor, needsInit);
  // exception is special
  if (!needsInit && m_name == "exception") needsInit = true;
}

void ClassStatement::outputCPPImpl(CodeGenerator &cg, AnalysisResultPtr ar) {
  if (cg.getContext() == CodeGenerator::NoContext) {
    InterfaceStatement::outputCPPImpl(cg, ar);
    return;
  }

  ClassScopeRawPtr classScope = getClassScope();
  if (cg.getContext() != CodeGenerator::CppForwardDeclaration) {
    printSource(cg);
  }

  string clsNameStr = classScope->getId();
  const char *clsName = clsNameStr.c_str();

  switch (cg.getContext()) {
  case CodeGenerator::CppDeclaration:
    {
      if (Option::GenerateCPPMacros) {
        classScope->outputForwardDeclaration(cg);
      }
      classScope->outputCPPGlobalTableWrappersDecl(cg, ar);

      bool system = cg.getOutput() == CodeGenerator::SystemCPP;
      ClassScopePtr parCls;
      if (!m_parent.empty()) {
        parCls = ar->findClass(m_parent);
        if (parCls && parCls->isRedeclaring()) parCls.reset();
      }
      if (Option::GenerateCppLibCode) {
        cg.printDocComment(classScope->getDocComment());
      }
      cg_printf("class %s%s", Option::ClassPrefix, clsName);
      if (!m_parent.empty() && classScope->derivesDirectlyFrom(m_parent)) {
        if (!parCls) {
          cg_printf(" : public DynamicObjectData");
        } else {
          cg_printf(" : public %s%s", Option::ClassPrefix,
                    parCls->getId().c_str());
        }
      } else {
        if (classScope->derivesFromRedeclaring()) {
          cg_printf(" : public DynamicObjectData");
        } else if (system) {
          cg_printf(" : public ExtObjectData");
        } else {
          cg_printf(" : public ObjectData");
        }
      }
      if (m_base && Option::UseVirtualDispatch) {
        for (int i = 0; i < m_base->getCount(); i++) {
          ScalarExpressionPtr exp =
            dynamic_pointer_cast<ScalarExpression>((*m_base)[i]);
          const char *intf = exp->getString().c_str();
          ClassScopePtr intfClassScope = ar->findClass(intf);
          if (intfClassScope && !intfClassScope->isRedeclaring() &&
              classScope->derivesDirectlyFrom(intf) &&
              (!parCls || !parCls->derivesFrom(ar, intf, true, false))) {
            string id = intfClassScope->getId();
            cg_printf(", public %s%s", Option::ClassPrefix, id.c_str());
          }
        }
      }
      cg_indentBegin(" {\n");
      cg_printf("public:\n");

      cg.printSection("Properties");
      if (classScope->getVariables()->outputCPPPropertyDecl(
            cg, ar, classScope->derivesFromRedeclaring())) {
        cg.printSection("Destructor");
        cg_printf("~%s%s() NEVER_INLINE {}", Option::ClassPrefix, clsName);
      }

      if (Option::GenerateCppLibCode) {
        cg.printSection("Methods");
        classScope->outputMethodWrappers(cg, ar);
        cg.printSection(">>>>>>>>>> Internal Implementation <<<<<<<<<<");
        cg_printf("// NOTE: Anything below is subject to change. "
                  "Use everything above instead.\n");
      }

      cg.printSection("Class Map");

      bool hasEmitCppCtor = false;
      bool needsCppCtor = classScope->needsCppCtor();
      bool needsInit    = classScope->needsInitMethod();

      bool disableDestructor =
        !classScope->canSkipCreateMethod(ar) ||
        (!classScope->derivesFromRedeclaring() &&
         !classScope->hasAttribute(ClassScope::HasDestructor, ar));

      if (Option::GenerateCPPMacros) {
        bool dyn = classScope->derivesFromRedeclaring() ==
          ClassScope::DirectFromRedeclared;
        bool idyn = parCls && classScope->derivesFromRedeclaring() ==
          ClassScope::IndirectFromRedeclared;
        bool redec = classScope->isRedeclaring();

        if (!parCls && !m_parent.empty()) {
          assert(dyn);
        }

        if (!classScope->derivesFromRedeclaring()) {
          outputCPPClassDecl(cg, ar, clsName, m_originalName.c_str(),
                             parCls ? parCls->getId().c_str()
                                    : "ObjectData");
        } else {
          cg_printf("DECLARE_DYNAMIC_CLASS(%s, %s, %s)\n", clsName,
                    m_originalName.c_str(),
                    dyn || !parCls ? "DynamicObjectData" :
                    parCls->getId().c_str());
        }

        if (classScope->checkHasPropTable(ar)) {
          cg_printf("static const ClassPropTable %sprop_table;\n",
                    Option::ObjectStaticPrefix);
        }

        bool hasGet = classScope->getAttribute(
          ClassScope::HasUnknownPropGetter);
        bool hasSet = classScope->getAttribute(
          ClassScope::HasUnknownPropSetter);
        bool hasIsset = classScope->getAttribute(
          ClassScope::HasUnknownPropTester);
        bool hasUnset = classScope->getAttribute(
          ClassScope::HasPropUnsetter);
        bool hasCall = classScope->getAttribute(
          ClassScope::HasUnknownMethodHandler);
        bool hasCallStatic = classScope->getAttribute(
          ClassScope::HasUnknownStaticMethodHandler);

        bool hasRootParam =
          classScope->derivedByDynamic() && (redec || dyn || idyn);
        string lateInit = "";
        if (redec && classScope->derivedByDynamic()) {
          if (!dyn && !idyn && (!parCls || parCls->isUserClass())) {
            cg_printf("private: ObjectData* root;\n");
            cg_printf("public:\n");
            cg_printf("virtual ObjectData *getRoot() { return root; }\n");
            lateInit = "root(r ? r : this)";
          }
        }

        string callbacks = Option::ClassStaticsCallbackPrefix + clsNameStr;
        string conInit = "";
        if (dyn) {
          conInit = "DynamicObjectData(cb, \"" +
            CodeGenerator::EscapeLabel(m_parent) + "\", ";
          if (hasRootParam) {
            conInit += "r)";
          } else {
            conInit += "this)";
          }
        } else if (parCls) {
          conInit = string(Option::ClassPrefix) + parCls->getId() + "(";
          if (parCls->derivedByDynamic() &&
              (parCls->isRedeclaring() ||
               parCls->derivesFromRedeclaring() != ClassScope::FromNormal)) {
            if (hasRootParam) {
              conInit += "r ? r : ";
            }
            conInit += "this, ";
          }
          conInit += "cb)";
        } else {
          if (system) {
            conInit = "ExtObjectData(cb)";
          } else {
            if (hasRootParam) {
              conInit = "ObjectData(cb, r)";
            } else {
              conInit = "ObjectData(cb, false)";
            }
          }
        }

        cg_printf("%s%s(%sconst ObjectStaticCallbacks *cb = &%s%s) : %s",
                  Option::ClassPrefix,
                  clsName,
                  hasRootParam ? "ObjectData* r = NULL," : "",
                  callbacks.c_str(),
                  redec ? ".oscb" : "",
                  conInit.c_str());

        if (needsCppCtor) {
          cg_printf(", ");
          cg.setContext(CodeGenerator::CppConstructor);
          ASSERT(!cg.hasInitListFirstElem());
          m_stmt->outputCPP(cg, ar);
          cg.clearInitListFirstElem();
          cg.setContext(CodeGenerator::CppDeclaration);
        }
        if (!lateInit.empty()) {
          cg_printf(", %s", lateInit.c_str());
        }

        cg_indentBegin(" {%s",
                       hasGet || hasSet || hasIsset || hasUnset ||
                       hasCall || hasCallStatic || disableDestructor ||
                       hasRootParam ? "\n" : "");
        if (hasRootParam) {
          cg_printf("setId(r);\n");
        }
        if (hasGet) cg_printf("setAttribute(UseGet);\n");
        if (hasSet) cg_printf("setAttribute(UseSet);\n");
        if (hasIsset) cg_printf("setAttribute(UseIsset);\n");
        if (hasUnset) cg_printf("setAttribute(UseUnset);\n");
        if (hasCall) cg_printf("setAttribute(HasCall);\n");
        if (hasCallStatic) cg_printf("setAttribute(HasCallStatic);\n");
        if (disableDestructor) {
          cg_printf("if (!hhvm) setAttribute(NoDestructor);\n");
        }
        cg_indentEnd("}\n");
        hasEmitCppCtor = true;
      }

      if (needsCppCtor && !hasEmitCppCtor) {
        cg_printf("%s%s() : ", Option::ClassPrefix, clsName);
        cg.setContext(CodeGenerator::CppConstructor);
        ASSERT(!cg.hasInitListFirstElem());
        m_stmt->outputCPP(cg, ar);
        cg.clearInitListFirstElem();
        cg.setContext(CodeGenerator::CppDeclaration);
        cg_printf(" {%s}\n",
                  disableDestructor ?
                  " if (!hhvm) setAttribute(NoDestructor); " : "");
      }

      if (needsInit) {
        cg_printf("void init();\n");
      }

      // doCall
      if (classScope->getAttribute(ClassScope::HasUnknownMethodHandler)) {
        cg_printf("Variant doCall(Variant v_name, Variant v_arguments, "
                  "bool fatal);\n");
      }

      if (classScope->getAttribute(ClassScope::HasInvokeMethod)) {
        FunctionScopePtr func =
          classScope->findFunction(ar, "__invoke", false);
        ASSERT(func);
        if (!func->isAbstract()) {
          cg_printf("const CallInfo *"
                    "t___invokeCallInfoHelper(void *&extra);\n");
        }
      }

      if (classScope->isRedeclaring() &&
          !classScope->derivesFromRedeclaring() &&
          classScope->derivedByDynamic()) {
        cg_printf("Variant doRootCall(Variant v_name, Variant v_arguments, "
                  "bool fatal);\n");
      }

      if (m_stmt) m_stmt->outputCPP(cg, ar);
      {
        std::set<string> done;
        classScope->outputCPPStaticMethodWrappers(cg, ar, done, clsName);
      }
      if (Option::GenerateCPPMacros) {
        classScope->outputCPPJumpTableDecl(cg, ar);
      }
      cg_indentEnd("};\n");

      classScope->outputCPPDynamicClassDecl(cg);

      if (m_stmt) {
        cg.setContext(CodeGenerator::CppClassConstantsDecl);
        m_stmt->outputCPP(cg, ar);
        cg.setContext(CodeGenerator::CppDeclaration);
      }
    }
    break;
  case CodeGenerator::CppImplementation:
    {
      if (m_stmt) {
        cg.setContext(CodeGenerator::CppClassConstantsImpl);
        m_stmt->outputCPP(cg, ar);
        cg.setContext(CodeGenerator::CppImplementation);
      }

      classScope->outputCPPSupportMethodsImpl(cg, ar);

      bool needsInit = classScope->needsInitMethod();
      if (needsInit) {
        cg_indentBegin("void %s%s::init() {\n",
                       Option::ClassPrefix, clsName);
        if (!m_parent.empty()) {
          if (classScope->derivesFromRedeclaring() ==
              ClassScope::DirectFromRedeclared) {
            cg_printf("parent->init();\n");
          } else {
            ClassScopePtr parCls = ar->findClass(m_parent);
            cg_printf("%s%s::init();\n", Option::ClassPrefix,
                      parCls->getId().c_str());
          }
        }
        if (classScope->getVariables()->
            getAttribute(VariableTable::NeedGlobalPointer)) {
          cg.printDeclareGlobals();
        }
        cg.setContext(CodeGenerator::CppInitializer);
        if (m_stmt) m_stmt->outputCPP(cg, ar);

        // This is lame. Exception base class needs to prepare stacktrace
        // outside of its PHP constructor. Every subclass of exception also
        // needs this stacktrace, so we're adding an artificial __init__ in
        // exception.php and calling it here.
        if (m_name == "exception") {
          cg_printf("{CountableHelper h(this); t___init__();}\n");
        }

        cg_indentEnd("}\n");
      }

      cg.setContext(CodeGenerator::CppImplementation);
      if (m_stmt) m_stmt->outputCPP(cg, ar);
    }
    break;
  case CodeGenerator::CppFFIDecl:
  case CodeGenerator::CppFFIImpl:
    if (m_stmt) m_stmt->outputCPP(cg, ar);
    break;
  case CodeGenerator::JavaFFI:
    {
      if (classScope->isRedeclaring()) break;

      // TODO support PHP namespaces, once HPHP supports it
      string packageName = Option::JavaFFIRootPackage;
      string packageDir = packageName;
      Util::replaceAll(packageDir, ".", "/");

      string outputDir = ar->getOutputPath() + "/" + Option::FFIFilePrefix +
        packageDir + "/";
      Util::mkdir(outputDir);

      // uses a different cg to generate a separate file for each PHP class
      // also, uses the original capitalized class name
      string clsFile = outputDir + getOriginalName() + ".java";
      std::ofstream fcls(clsFile.c_str());
      CodeGenerator cgCls(&fcls, CodeGenerator::FileCPP);
      cgCls.setContext(CodeGenerator::JavaFFI);

      cgCls.printf("package %s;\n\n", packageName.c_str());
      cgCls.printf("import hphp.*;\n\n");

      printSource(cgCls);

      string clsModifier;
      switch (m_type) {
      case T_CLASS:
        break;
      case T_ABSTRACT:
        clsModifier = "abstract ";
        break;
      case T_FINAL:
        clsModifier = "final ";
        break;
      }
      cgCls.printf("public %sclass %s ", clsModifier.c_str(),
                   getOriginalName().c_str());

      ClassScopePtr parCls;
      if (!m_parent.empty()) parCls = ar->findClass(m_parent);
      if (!m_parent.empty() && classScope->derivesDirectlyFrom(m_parent)
          && parCls && parCls->isUserClass() && !parCls->isRedeclaring()) {
        // system classes are not supported in static FFI translation
        // they shouldn't appear as superclasses as well
        cgCls.printf("extends %s", parCls->getOriginalName().c_str());
      }
      else {
        cgCls.printf("extends HphpObject");
      }
      if (m_base) {
        bool first = true;
        for (int i = 0; i < m_base->getCount(); i++) {
          ScalarExpressionPtr exp =
            dynamic_pointer_cast<ScalarExpression>((*m_base)[i]);
          const char *intf = exp->getString().c_str();
          ClassScopePtr intfClassScope = ar->findClass(intf);
          if (intfClassScope && classScope->derivesFrom(ar, intf, false, false)
           && intfClassScope->isUserClass()) {
            if (first) {
              cgCls.printf(" implements ");
              first = false;
            }
            else {
              cgCls.printf(", ");
            }
            cgCls.printf(intfClassScope->getOriginalName().c_str());
          }
        }
      }

      cgCls.indentBegin(" {\n");

      // constructor for initializing the variant pointer
      cgCls.printf("protected %s(long ptr) { super(ptr); }\n\n",
                   getOriginalName().c_str());

      FunctionScopePtr cons = classScope->findConstructor(ar, true);
      if (cons && !cons->isAbstract() || m_type != T_ABSTRACT) {
        // if not an abstract class and not having an explicit constructor,
        // adds a default constructor
        outputJavaFFIConstructor(cgCls, ar, cons);
      }

      if (m_stmt) m_stmt->outputCPP(cgCls, ar);
      cgCls.indentEnd("}\n");

      fcls.close();
    }
    break;
  case CodeGenerator::JavaFFICppDecl:
  case CodeGenerator::JavaFFICppImpl:
    {
      if (classScope->isRedeclaring()) break;

      if (m_stmt) m_stmt->outputCPP(cg, ar);
      FunctionScopePtr cons = classScope->findConstructor(ar, true);
      if (cons && !cons->isAbstract() || m_type != T_ABSTRACT) {
        outputJavaFFICPPCreator(cg, ar, cons);
      }
    }
    break;
  default:
    ASSERT(false);
    break;
  }
}

void ClassStatement::outputJavaFFIConstructor(CodeGenerator &cg,
                                              AnalysisResultPtr ar,
                                              FunctionScopePtr cons) {
  int ac = cons ? cons->getMaxParamCount() : 0;
  bool varArgs = cons && cons->isVariableArgument();

  // generates the constructor
  cg_printf("public %s(", getOriginalName().c_str());
  std::ostringstream args;
  std::ostringstream params;
  bool first = true;
  for (int i = 0; i < ac; i++) {
    if (first) {
      first = false;
    }
    else {
      cg_printf(", ");
      args << ", ";
      params << ", ";
    }
    cg_printf("HphpVariant a%d", i);
    args << "a" << i << ".getVariantPtr()";
    params << "long a" << i;
  }
  if (varArgs) {
    if (!first) {
      cg_printf(", ");
      args << ", ";
      params << ", ";
    }
    cg_printf("HphpVariant va");
    args << "va.getVariantPtr()";
    params << "long va";
  }
  cg_indentBegin(") {\n");
  cg_printf("this(create(%s));\n", args.str().c_str());
  cg_indentEnd("}\n\n");

  // generates the native method stub for creating the object
  cg_printf("private static native long create(%s);\n\n",
            params.str().c_str());
}

void ClassStatement::outputJavaFFICPPCreator(CodeGenerator &cg,
                                             AnalysisResultPtr ar,
                                             FunctionScopePtr cons) {
  ClassScopeRawPtr cls = getClassScope();
  string packageName = Option::JavaFFIRootPackage;
  int ac = cons ? cons->getMaxParamCount() : 0;
  bool varArgs = cons && cons->isVariableArgument();
  const char *clsName = getOriginalName().c_str();

  string mangledName = "Java_" + packageName + "_" + clsName + "_create";
  Util::replaceAll(mangledName, ".", "_");

  cg_printf("JNIEXPORT jlong JNICALL\n");
  cg_printf("%s(JNIEnv *env, jclass cls", mangledName.c_str());

  std::ostringstream args;
  bool first = true;
  if (varArgs) {
    args << ac << " + (((Variant *)va)->isNull() ? 0"
               << " : ((Variant *)va)->getArrayData()->size())";
    first = false;
  }
  for (int i = 0; i < ac; i++) {
    if (first) first = false;
    else {
      args << ", ";
    }
    cg_printf(", jlong a%d", i);
    args << "*(Variant *)a" << i;
  }
  if (varArgs) {
    if (!first) {
      args << ", ";
    }
    cg_printf(", jlong va");
    args << "((Variant *)va)->toArray()";
  }

  if (cg.getContext() == CodeGenerator::JavaFFICppDecl) {
    // java_stubs.h
    cg_printf(");\n\n");
    return;
  }

  cg_indentBegin(") {\n");
  cg_printf("ObjectData *obj = (NEWOBJ(%s%s)())->create(%s);\n",
            Option::ClassPrefix, cls->getId().c_str(), args.str().c_str());
  cg_printf("return (jlong)(NEW(Variant)(obj));\n");
  cg_indentEnd("}\n\n");
}
