/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#include <lib/statement/class_statement.h>
#include <lib/parser/hphp.tab.hpp>
#include <lib/expression/expression_list.h>
#include <lib/statement/statement_list.h>
#include <lib/expression/scalar_expression.h>
#include <lib/analysis/class_scope.h>
#include <lib/analysis/file_scope.h>
#include <lib/analysis/function_scope.h>
#include <lib/analysis/analysis_result.h>
#include <lib/analysis/dependency_graph.h>
#include <lib/statement/method_statement.h>
#include <lib/analysis/variable_table.h>
#include <lib/analysis/constant_table.h>
#include <util/util.h>
#include <lib/statement/interface_statement.h>
#include <lib/option.h>
#include <sstream>

using namespace HPHP;
using namespace std;
using namespace boost;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

ClassStatement::ClassStatement
(STATEMENT_CONSTRUCTOR_PARAMETERS,
 int type, const std::string &name, const std::string &parent,
 ExpressionListPtr base, StatementListPtr stmt)
  : InterfaceStatement(STATEMENT_CONSTRUCTOR_PARAMETER_VALUES,
                       name, base, stmt), m_type(type) {
  m_parent = Util::toLower(parent);
}

StatementPtr ClassStatement::clone() {
  ClassStatementPtr stmt(new ClassStatement(*this));
  stmt->m_stmt = Clone(m_stmt);
  stmt->m_base = Clone(m_base);
  return stmt;
}

///////////////////////////////////////////////////////////////////////////////
// parser functions

void ClassStatement::onParse(AnalysisResultPtr ar) {
  ClassScope::KindOf kindOf = ClassScope::KindOfObjectClass;
  switch (m_type) {
  case T_CLASS:     kindOf = ClassScope::KindOfObjectClass;   break;
  case T_ABSTRACT:  kindOf = ClassScope::KindOfAbstractClass; break;
  case T_FINAL:     kindOf = ClassScope::KindOfFinalClass;    break;
  default:
    ASSERT(false);
  }

  vector<string> bases;
  if (!m_parent.empty()) bases.push_back(m_parent);
  if (m_base) m_base->getStrings(bases);
  StatementPtr stmt = dynamic_pointer_cast<Statement>(shared_from_this());
  ClassScopePtr classScope(new ClassScope(kindOf, m_originalName, m_parent,
                                          bases, stmt, ar->getFileScope()));
  m_classScope = classScope;
  ar->getFileScope()->addClass(ar, classScope);
  ar->recordClassSource(m_name, ar->getFileScope()->getName());

  if (m_stmt) {
    ar->pushScope(classScope);
    bool seenConstruct = false;
    for (int i = 0; i < m_stmt->getCount(); i++) {
      MethodStatementPtr meth =
        dynamic_pointer_cast<MethodStatement>((*m_stmt)[i]);
      if (meth && meth->getName() == "__construct") {
        seenConstruct = true;
        break;
      }
    }
    for (int i = 0; i < m_stmt->getCount(); i++) {
      if (!seenConstruct) {
        MethodStatementPtr meth =
          dynamic_pointer_cast<MethodStatement>((*m_stmt)[i]);
        if (meth && classScope && meth->getName() == classScope->getName()
         && !meth->getModifiers()->isStatic()) {
          // class-name constructor
          classScope->setAttribute(ClassScope::classNameConstructor);
        }
      }
      IParseHandlerPtr ph = dynamic_pointer_cast<IParseHandler>((*m_stmt)[i]);
      ph->onParse(ar);
    }
    ar->popScope();
  }
}

///////////////////////////////////////////////////////////////////////////////
// static analysis functions

std::string ClassStatement::getName() const {
  return string("Class ") + m_classScope.lock()->getName();
}

void ClassStatement::analyzeProgram(AnalysisResultPtr ar) {
  vector<string> bases;
  if (!m_parent.empty()) bases.push_back(m_parent);
  if (m_base) m_base->getStrings(bases);
  for (unsigned int i = 0; i < bases.size(); i++) {
    string className = bases[i];
    addUserClass(ar, bases[i]);
  }

  ClassScopePtr classScope = m_classScope.lock();
  if (hasHphpNote("Volatile")) classScope->setVolatile();
  FunctionScopePtr func = ar->getFunctionScope();
  // redeclared classes are automatically volatile
  if (classScope->isVolatile()) {
    func->getVariables()->setAttribute(VariableTable::NeedGlobalPointer);
  }
  if (m_stmt) {
    ar->pushScope(classScope);
    m_stmt->analyzeProgram(ar);
    ar->popScope();
  }
  DependencyGraphPtr dependencies = ar->getDependencyGraph();
  for (unsigned int i = 0; i < bases.size(); i++) {
    ClassScopePtr cls = ar->findClass(bases[i]);
    if (cls) {
      if (dependencies->checkCircle(DependencyGraph::KindOfClassDerivation,
                                    m_originalName,
                                    cls->getOriginalName())) {
        ClassScopePtr classScope = m_classScope.lock();
        ar->getCodeError()->record(CodeError::InvalidDerivation,
                                   shared_from_this(), ConstructPtr(),
                                   cls->getOriginalName());
        m_parent = "";
        m_base = ExpressionListPtr();
        classScope->clearBases();
      } else if (cls->isUserClass()) {
        dependencies->add(DependencyGraph::KindOfClassDerivation,
                          ar->getName(),
                          m_originalName, shared_from_this(),
                          cls->getOriginalName(), cls->getStmt());
      }
    }
  }
}

StatementPtr ClassStatement::preOptimize(AnalysisResultPtr ar) {
  return InterfaceStatement::preOptimize(ar);
}

StatementPtr ClassStatement::postOptimize(AnalysisResultPtr ar) {
  return InterfaceStatement::postOptimize(ar);
}

void ClassStatement::inferTypes(AnalysisResultPtr ar) {
  if (m_stmt) {
    ClassScopePtr classScope = m_classScope.lock();
    ar->pushScope(classScope);
    m_stmt->inferTypes(ar);
    ar->popScope();
  }
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void ClassStatement::getAllParents(AnalysisResultPtr ar,
                                   std::vector<std::string> &names) {
  if (!m_parent.empty()) {
    ClassScopePtr cls = ar->findClass(m_parent);
    if (cls) {
      cls->getAllParents(ar, names);
      names.push_back(m_parent);
    }
  }

  if (m_base) {
    vector<string> bases;
    m_base->getStrings(bases);
    for (unsigned int i = 0; i < bases.size(); i++) {
      ClassScopePtr cls = ar->findClass(bases[i]);
      if (cls) {
        cls->getAllParents(ar, names);
        names.push_back(bases[i]);
      }
    }
  }
}

void ClassStatement::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  ClassScopePtr classScope = m_classScope.lock();
  if (!classScope->isUserClass()) return;
  if (ar) ar->pushScope(classScope);

  switch (m_type) {
  case T_CLASS:                              break;
  case T_ABSTRACT: cg.printf("abstract ");   break;
  case T_FINAL:    cg.printf("final ");      break;
  default:
    ASSERT(false);
  }
  cg.printf("class %s", m_name.c_str());

  if (!m_parent.empty()) {
    cg.printf(" extends %s", m_parent.c_str());
  }

  if (m_base) {
    cg.printf(" implements ");
    m_base->outputPHP(cg, ar);
  }

  cg.indentBegin(" {\n");
  m_classScope.lock()->outputPHP(cg, ar);
  if (m_stmt) m_stmt->outputPHP(cg, ar);
  cg.indentEnd("}\n");

  if (ar) ar->popScope();
}

bool ClassStatement::hasImpl() const {
  ClassScopePtr cls = m_classScope.lock();
  return cls->isVolatile() ||
    cls->getVariables()->getAttribute(VariableTable::ContainsDynamicStatic);
}

void ClassStatement::outputCPP(CodeGenerator &cg, AnalysisResultPtr ar) {
  ClassScopePtr classScope = m_classScope.lock();
  if (cg.getContext() == CodeGenerator::NoContext) {
    if (classScope->isRedeclaring()) {
      cg.printf("g->%s%s = ClassStaticsPtr(NEW(%s%s)());\n",
                Option::ClassStaticsObjectPrefix, m_name.c_str(),
                Option::ClassStaticsPrefix, classScope->getId().c_str());
    }
    if (classScope->isVolatile()) {
      cg.printf("g->declareClass(\"%s\");\n",
                m_name.c_str());
    }
    return;
  }

  if (cg.getContext() != CodeGenerator::CppForwardDeclaration) {
    printSource(cg);
  }

  ar->pushScope(classScope);
  string clsNameStr = classScope->getId();
  const char *clsName = clsNameStr.c_str();
  bool redeclared = classScope->isRedeclaring();
  switch (cg.getContext()) {
  case CodeGenerator::CppForwardDeclaration:
    if (Option::GenerateCPPMacros) {
      cg.printf("FORWARD_DECLARE_CLASS(%s)\n", clsName);
      if (redeclared) {
        cg.printf("FORWARD_DECLARE_REDECLARED_CLASS(%s)\n", clsName);
      }
    }
    if (m_stmt) {
      cg.setContext(CodeGenerator::CppClassConstantsDecl);
      m_stmt->outputCPP(cg, ar);
      cg.setContext(CodeGenerator::CppForwardDeclaration);
    }
    break;
  case CodeGenerator::CppDeclaration:
    {
      ClassScopePtr parCls;
      if (!m_parent.empty()) parCls = ar->findClass(m_parent);
      cg.printf("class %s%s", Option::ClassPrefix, clsName);
      bool derived = false;
      if (!m_parent.empty() && classScope->derivesFrom(ar, m_parent)) {
        if (parCls->isRedeclaring()) {
          cg.printf(" : public DynamicObjectData");
        } else {
          cg.printf(" : virtual public %s%s", Option::ClassPrefix,
                    parCls->getId().c_str());
        }
        derived = true;
      }
      if (m_base) {
        for (int i = 0; i < m_base->getCount(); i++) {
          ScalarExpressionPtr exp =
            dynamic_pointer_cast<ScalarExpression>((*m_base)[i]);
          const char *intf = exp->getString().c_str();
          ClassScopePtr intfClassScope = ar->findClass(intf);
          if (intfClassScope && classScope->derivesFrom(ar, intf)) {
            // temporary fix for inheriting from a re-declaring class
            string id = intfClassScope->getId();
            if (!derived) {
              derived = true;
              cg.printf(" :");
            } else {
              cg.printf(",");
            }
            cg.printf(" virtual public %s%s", Option::ClassPrefix, id.c_str());
          }
        }
      }
      if (!derived) {
        const char *op = derived ? "," : " :";
        if (classScope->derivesFromRedeclaring()) {
          cg.printf("%s public DynamicObjectData", op);
        } else {
          cg.printf("%s virtual public ObjectData", op);
        }
      }
      cg.indentBegin(" {\n");

      if (Option::GenerateCPPMacros) {
        vector<string> bases;
        getAllParents(ar, bases);

        cg.indentBegin("BEGIN_CLASS_MAP(%s)\n", clsName);
        for (unsigned int i = 0; i < bases.size(); i++) {
          cg.printf("PARENT_CLASS(%s)\n", bases[i].c_str());
        }
        cg.indentEnd("END_CLASS_MAP(%s)\n", clsName);
      }

      if (Option::GenerateCPPMacros) {
        bool dyn = classScope->derivesFromRedeclaring() ==
          ClassScope::DirectFromRedeclared;
        bool idyn = classScope->derivesFromRedeclaring() ==
          ClassScope::IndirectFromRedeclared;
        bool redec = classScope->isRedeclaring();
        if (!classScope->derivesFromRedeclaring()) {
          cg.printf("DECLARE_CLASS(%s, %s, %s)\n", clsName,
                    m_originalName.c_str(),
                    m_parent.empty() ? "ObjectData" : m_parent.c_str());
        } else {
          cg.printf("DECLARE_DYNAMIC_CLASS(%s, %s)\n", clsName,
                    m_originalName.c_str());
        }
        if (cg.getOutput() == CodeGenerator::SystemCPP ||
            Option::EnableEval >= Option::LimitedEval) {
          cg.printf("DECLARE_INVOKES_FROM_EVAL\n");
        }
        if (dyn || idyn || redec) {
          if (redec) {

            cg.indentBegin("Variant %sroot_invoke(const char* s, CArrRef ps, "
                           "int64 h, bool f = true) {\n",
                           Option::ObjectPrefix);
            cg.printf("return root->%sinvoke(s, ps, h, f);\n",
                      Option::ObjectPrefix);
            cg.indentEnd("}\n");
            cg.indentBegin("Variant %sroot_invoke_few_args(const char* s, "
                           "int64 h, int count", Option::ObjectPrefix);
            for (int i = 0; i < Option::InvokeFewArgsCount; i++) {
              cg.printf(", CVarRef a%d = null_variant", i);
            }
            cg.printf(") {\n");
            cg.printf("return root->%sinvoke_few_args(s, h, count",
                      Option::ObjectPrefix);
            for (int i = 0; i < Option::InvokeFewArgsCount; i++) {
              cg.printf(", a%d", i);
            }
            cg.printf(");\n");
            cg.indentEnd("}\n");
            if (!dyn && !idyn) cg.printf("private: ObjectData* root;\n");
            cg.printf("public:\n");
          }

          string conInit = ":";
          if (dyn) {
            conInit += "DynamicObjectData(\"" + m_parent + "\", r)";
          } else if (idyn) {
            conInit += string(Option::ClassPrefix) + parCls->getId() +
              "(r?r:this)";
          } else {
            conInit += "root(r?r:this)";
          }

          cg.printf("%s%s(ObjectData* r = NULL)%s {}\n",
                    Option::ClassPrefix, clsName,
                    conInit.c_str());
        }
      }

      cg.printf("void init();\n",
                Option::ClassPrefix, clsName);

      if (classScope->needLazyStaticInitializer()) {
        cg.printf("static GlobalVariables *lazy_initializer"
                  "(GlobalVariables *g);\n");
      }

      if (!classScope->derivesFromRedeclaring()){
        classScope->getVariables()->outputCPPPropertyDecl(cg, ar);
      }

      if (!classScope->getAttribute(ClassScope::HasConstructor)) {
        FunctionScopePtr func = classScope->findFunction(ar, "__construct",
                                                         false);
        if (func && !func->isAbstract() && !classScope->isInterface()) {
          ar->pushScope(func);
          func->outputCPPCreateDecl(cg, ar);
          ar->popScope();
        }
      }
      if (classScope->getAttribute(ClassScope::HasDestructor)) {
        cg.printf("public: virtual void destruct();\n");
      }

      // doCall
      if (classScope->getAttribute(ClassScope::HasUnknownMethodHandler)) {
        cg.printf("Variant doCall(Variant v_name, Variant v_arguments, "
                  "bool fatal);\n");
      }

      if (m_stmt) m_stmt->outputCPP(cg, ar);
      {
        set<string> done;
        classScope->outputCPPStaticMethodWrappers(cg, ar, done, clsName);
      }

      cg.indentEnd("};\n");

      if (redeclared) {
        cg.indentBegin("class %s%s : public ClassStatics {\n",
                       Option::ClassStaticsPrefix, clsName);
        cg.printf("public:\n");
        cg.printf("DECLARE_OBJECT_ALLOCATION(%s%s);\n",
                  Option::ClassStaticsPrefix, clsName);
        cg.printf("%s%s() : ClassStatics(%d) {}\n",
                  Option::ClassStaticsPrefix, clsName,
                  classScope->getRedeclaringId());
        cg.indentBegin("Variant %sget(const char *s, int64 hash = -1) {\n",
                       Option::ObjectStaticPrefix);
        cg.printf("return %s%s::%sget(s, hash);\n", Option::ClassPrefix,
                  clsName, Option::ObjectStaticPrefix);
        cg.indentEnd("}\n");
        cg.indentBegin("Variant &%slval(const char* s, int64 hash = -1) {\n",
                  Option::ObjectStaticPrefix);
        cg.printf("return %s%s::%slval(s, hash);\n", Option::ClassPrefix,
                  clsName, Option::ObjectStaticPrefix);
        cg.indentEnd("}\n");
        cg.indentBegin("Variant %sinvoke(const char *c, const char *s, "
                       "CArrRef params, int64 hash = -1, bool fatal = true) "
                       "{\n",
                  Option::ObjectStaticPrefix);
        cg.printf("return %s%s::%sinvoke(c, s, params, hash, fatal);\n",
                  Option::ClassPrefix, clsName,
                  Option::ObjectStaticPrefix);
        cg.indentEnd("}\n");
        cg.indentBegin("Object create(CArrRef params, bool init = true, "
                       "ObjectData* root = NULL) {\n");
        cg.printf("return Object(%s%s(NEW(%s%s)(root))->"
                  "dynCreate(params, init));\n",
                  Option::SmartPtrPrefix, clsName,
                  Option::ClassPrefix, clsName);
        cg.indentEnd("}\n");
        cg.indentBegin("Variant %sconstant(const char* s) {\n",
                       Option::ObjectStaticPrefix);
        cg.printf("return %s%s::%sconstant(s);\n", Option::ClassPrefix, clsName,
                  Option::ObjectStaticPrefix);
        cg.indentEnd("}\n");
        cg.indentBegin("Variant %sinvoke_from_eval(const char *c, "
                       "const char *s, Eval::VariableEnvironment &env, "
                       "const Eval::FunctionCallExpression *call, "
                       "int64 hash = -1, bool fatal = true) "
                       "{\n",
                       Option::ObjectStaticPrefix);
        cg.printf("return %s%s::%sinvoke_from_eval(c, s, env, call, hash, "
                  "fatal);\n",
                  Option::ClassPrefix, clsName,
                  Option::ObjectStaticPrefix);
        cg.indentEnd("}\n");
        cg.indentEnd("};\n");
      }
    }
    break;
  case CodeGenerator::CppImplementation:
    if (m_stmt) {
      cg.setContext(CodeGenerator::CppClassConstantsImpl);
      m_stmt->outputCPP(cg, ar);
      cg.setContext(CodeGenerator::CppImplementation);
    }

    classScope->outputCPPSupportMethodsImpl(cg, ar);

    if (redeclared) {
      cg.printf("IMPLEMENT_OBJECT_ALLOCATION(%s%s);\n",
                Option::ClassStaticsPrefix, clsName);
    }

    cg.indentBegin("void %s%s::init() {\n",
                   Option::ClassPrefix, clsName);
    if (!m_parent.empty()) {
      if (classScope->derivesFromRedeclaring() ==
          ClassScope::DirectFromRedeclared) {
        cg.printf("parent->init();\n");
      } else {
        cg.printf("%s%s::init();\n", Option::ClassPrefix, m_parent.c_str());
      }
    }
    cg.setContext(CodeGenerator::CppConstructor);
    if (m_stmt) m_stmt->outputCPP(cg, ar);
    cg.indentEnd("}\n");

    if (classScope->needStaticInitializer()) {
      cg.indentBegin("void %s%s::os_static_initializer() {\n",
                     Option::ClassPrefix, clsName);
      cg.printDeclareGlobals();
      cg.setContext(CodeGenerator::CppStaticInitializer);
      if (m_stmt) m_stmt->outputCPP(cg, ar);
      cg.indentEnd("}\n");
      cg.indentBegin("void %s%s() {\n",
                     Option::ClassStaticInitializerPrefix, clsName);
      cg.printf("%s%s::os_static_initializer();\n",  Option::ClassPrefix,
                clsName);
      cg.indentEnd("}\n");
    }
    if (classScope->needLazyStaticInitializer()) {
      cg.indentBegin("GlobalVariables *%s%s::lazy_initializer("
                     "GlobalVariables *g) {\n", Option::ClassPrefix, clsName);
      cg.indentBegin("if (!g->%s%s) {\n",
                     Option::ClassStaticInitializerFlagPrefix, clsName);
      cg.printf("g->%s%s = true;\n", Option::ClassStaticInitializerFlagPrefix,
                clsName);
      cg.setContext(CodeGenerator::CppLazyStaticInitializer);
      if (m_stmt) m_stmt->outputCPP(cg, ar);
      cg.indentEnd("}\n");
      cg.printf("return g;\n");
      cg.indentEnd("}\n");
    }
    cg.setContext(CodeGenerator::CppImplementation);
    if (m_stmt) m_stmt->outputCPP(cg, ar);

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
      ofstream fcls(clsFile.c_str());
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
      if (!m_parent.empty() && classScope->derivesFrom(ar, m_parent)
       && parCls->isUserClass() && !parCls->isRedeclaring()) {
        // system classes are not supported in static FFI translation
        // they shouldn't appear as superclasses as well
        cgCls.printf("extends %s", parCls->getOriginalName());
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
          if (intfClassScope && classScope->derivesFrom(ar, intf)
           && intfClassScope->isUserClass()) {
            if (first) {
              cgCls.printf(" implements ");
              first = false;
            }
            else {
              cgCls.printf(", ");
            }
            cgCls.printf(intfClassScope->getOriginalName());
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

  ar->popScope();
}

void ClassStatement::outputJavaFFIConstructor(CodeGenerator &cg,
                                              AnalysisResultPtr ar,
                                              FunctionScopePtr cons) {
  int ac = cons ? cons->getMaxParamCount() : 0;
  bool varArgs = cons && cons->isVariableArgument();

  // generates the constructor
  cg.printf("public %s(", getOriginalName().c_str());
  ostringstream args;
  ostringstream params;
  bool first = true;
  for (int i = 0; i < ac; i++) {
    if (first) {
      first = false;
    }
    else {
      cg.printf(", ");
      args << ", ";
      params << ", ";
    }
    cg.printf("HphpVariant a%d", i);
    args << "a" << i << ".getVariantPtr()";
    params << "long a" << i;
  }
  if (varArgs) {
    if (!first) {
      cg.printf(", ");
      args << ", ";
      params << ", ";
    }
    cg.printf("HphpVariant va");
    args << "va.getVariantPtr()";
    params << "long va";
  }
  cg.indentBegin(") {\n");
  cg.printf("this(create(%s));\n", args.str().c_str());
  cg.indentEnd("}\n\n");

  // generates the native method stub for creating the object
  cg.printf("private static native long create(%s);\n\n",
            params.str().c_str());
}

void ClassStatement::outputJavaFFICPPCreator(CodeGenerator &cg,
                                             AnalysisResultPtr ar,
                                             FunctionScopePtr cons) {
  ClassScopePtr cls = m_classScope.lock();
  string packageName = Option::JavaFFIRootPackage;
  int ac = cons ? cons->getMaxParamCount() : 0;
  bool varArgs = cons && cons->isVariableArgument();
  const char *clsName = getOriginalName().c_str();

  string mangledName = "Java_" + packageName + "_" + clsName + "_create";
  Util::replaceAll(mangledName, ".", "_");

  cg.printf("JNIEXPORT jlong JNICALL\n");
  cg.printf("%s(JNIEnv *env, jclass cls", mangledName.c_str());

  ostringstream args;
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
    cg.printf(", jlong a%d", i);
    args << "*(Variant *)a" << i;
  }
  if (varArgs) {
    if (!first) {
      args << ", ";
    }
    cg.printf(", jlong va");
    args << "((Variant *)va)->toArray()";
  }

  if (cg.getContext() == CodeGenerator::JavaFFICppDecl) {
    // java_stubs.h
    cg.printf(");\n\n");
    return;
  }

  cg.indentBegin(") {\n");
  cg.printf("ObjectData *obj = ");
  cg.printf("(NEW(%s%s)())->create(%s);\n",
            Option::ClassPrefix, cls->getId().c_str(), args.str().c_str());
  cg.printf("obj->incRefCount();\n");
  cg.printf("return (jlong)(NEW(Variant)(obj));\n");
  cg.indentEnd("}\n\n");
}
