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

#include <lib/statement/interface_statement.h>
#include <lib/expression/scalar_expression.h>
#include <lib/expression/expression_list.h>
#include <lib/analysis/class_scope.h>
#include <lib/analysis/file_scope.h>
#include <lib/analysis/analysis_result.h>
#include <lib/analysis/dependency_graph.h>
#include <lib/statement/statement_list.h>
#include <util/util.h>
#include <lib/option.h>

using namespace HPHP;
using namespace std;
using namespace boost;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

InterfaceStatement::InterfaceStatement
(STATEMENT_CONSTRUCTOR_PARAMETERS,
 const std::string &name, ExpressionListPtr base, StatementListPtr stmt)
  : Statement(STATEMENT_CONSTRUCTOR_PARAMETER_VALUES),
    m_originalName(name), m_base(base), m_stmt(stmt) {
  m_name = Util::toLower(name);
  if (m_base) m_base->toLower();
}

StatementPtr InterfaceStatement::clone() {
  InterfaceStatementPtr stmt(new InterfaceStatement(*this));
  stmt->m_stmt = Clone(m_stmt);
  stmt->m_base = Clone(m_base);
  return stmt;
}

BlockScopePtr InterfaceStatement::getScope() {
  return m_classScope.lock();
}
int InterfaceStatement::getRecursiveCount() const {
  return m_stmt ? m_stmt->getRecursiveCount() : 0;
}
///////////////////////////////////////////////////////////////////////////////
// parser functions

void InterfaceStatement::onParse(AnalysisResultPtr ar) {
  vector<string> bases;
  if (m_base) m_base->getStrings(bases);

  StatementPtr stmt = dynamic_pointer_cast<Statement>(shared_from_this());
  ClassScopePtr classScope(new ClassScope(ClassScope::KindOfInterface,
                                          m_name, "", bases, stmt,
                                          ar->getFileScope()));
  m_classScope = classScope;
  ar->getFileScope()->addClass(ar, classScope);
  ar->getDependencyGraph()->addParent(DependencyGraph::KindOfProgramUserClass,
                                      "", m_originalName, stmt);

  if (m_stmt) {
    ar->pushScope(classScope);
    for (int i = 0; i < m_stmt->getCount(); i++) {
      IParseHandlerPtr ph = dynamic_pointer_cast<IParseHandler>((*m_stmt)[i]);
      ph->onParse(ar);
    }
    ar->popScope();
  }
}

///////////////////////////////////////////////////////////////////////////////
// static analysis functions

std::string InterfaceStatement::getName() const {
  return string("Interface ") + m_classScope.lock()->getName();
}

void InterfaceStatement::analyzeProgram(AnalysisResultPtr ar) {
  ClassScopePtr classScope = m_classScope.lock();
  if (hasHphpNote("Volatile")) classScope->setVolatile();
  if (m_stmt) {
    classScope->setIncludeLevel(ar->getIncludeLevel());
    ar->pushScope(classScope);
    m_stmt->analyzeProgram(ar);
    ar->popScope();
  }
  ar->recordClassSource(m_name, ar->getFileScope()->getName());
  vector<string> bases;
  if (m_base) m_base->getStrings(bases);
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

StatementPtr InterfaceStatement::preOptimize(AnalysisResultPtr ar) {
  ar->preOptimize(m_base);
  if (m_stmt) {
    ClassScopePtr classScope = m_classScope.lock();
    ar->pushScope(classScope);
    ar->preOptimize(m_stmt);
    ar->popScope();
  }
  return StatementPtr();
}

StatementPtr InterfaceStatement::postOptimize(AnalysisResultPtr ar) {
  ar->postOptimize(m_base);
  if (m_stmt) {
    ClassScopePtr classScope = m_classScope.lock();
    ar->pushScope(classScope);
    ar->postOptimize(m_stmt);
    ar->popScope();
  }
  return StatementPtr();
}

void InterfaceStatement::inferTypes(AnalysisResultPtr ar) {
  vector<string> bases;
  if (m_base) m_base->getStrings(bases);
  for (unsigned int i = 0; i < bases.size(); i++) {
    addUserClass(ar, bases[i]);
  }

  if (m_stmt) {
    ClassScopePtr classScope = m_classScope.lock();
    ar->pushScope(classScope);
    m_stmt->inferTypes(ar);
    ar->popScope();
  }
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void InterfaceStatement::getAllParents(AnalysisResultPtr ar,
                                       std::vector<std::string> &names) {
  vector<string> bases;
  if (m_base) {
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

void InterfaceStatement::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  ClassScopePtr classScope = m_classScope.lock();

  if (cg.getOutput() == CodeGenerator::InlinedPHP ||
      cg.getOutput() == CodeGenerator::TrimmedPHP) {
    if (!classScope->isUserClass()) {
      return;
    }
  }

  if (ar) ar->pushScope(classScope);

  cg.printf("interface %s", m_name.c_str());
  if (m_base) {
    cg.printf(" extends ");
    m_base->outputPHP(cg, ar);
  }
  cg.indentBegin(" {\n");
  m_classScope.lock()->outputPHP(cg, ar);
  if (m_stmt) m_stmt->outputPHP(cg, ar);
  cg.indentEnd("}\n");

  if (ar) ar->popScope();
}

void InterfaceStatement::outputCPP(CodeGenerator &cg, AnalysisResultPtr ar) {
  ClassScopePtr classScope = m_classScope.lock();
  if (cg.getContext() == CodeGenerator::NoContext) {
    if (classScope->isVolatile()) {
      cg.printf("get_globals()->declareInterface(\"%s\");\n", m_name.c_str());
    }
    return;
  }

  ar->pushScope(classScope);
  string clsNameStr = classScope->getId();
  const char *clsName = clsNameStr.c_str();

  switch (cg.getContext()) {
  case CodeGenerator::CppForwardDeclaration:
    if (Option::GenerateCPPMacros) {
      cg.printf("FORWARD_DECLARE_CLASS(%s);\n", clsName);
    }
    break;
  case CodeGenerator::CppDeclaration:
    {
      printSource(cg);
      cg.printf("class %s%s", Option::ClassPrefix, clsName);
      bool derived = false;
      if (m_base) {
        for (int i = 0; i < m_base->getCount(); i++) {
          ScalarExpressionPtr exp =
            dynamic_pointer_cast<ScalarExpression>((*m_base)[i]);
          const char *intf = exp->getString().c_str();

          if (classScope->derivesFrom(ar, intf)) {
            if (!derived) {
              derived = true;
              cg.printf(" : virtual public %s%s", Option::ClassPrefix, intf);
            } else {
              cg.printf(", virtual public %s%s", Option::ClassPrefix, intf);
            }
          }
        }
      }
      if (!derived) {
        cg.printf(" : virtual public ObjectData");
      }

      cg.indentBegin(" {\n");
      if (m_stmt) m_stmt->outputCPP(cg, ar);
      cg.indentEnd("};\n");
    }
    break;
  case CodeGenerator::CppImplementation:
    // do nothing
    break;
  case CodeGenerator::CppFFIDecl:
  case CodeGenerator::CppFFIImpl:
    // do nothing
    break;
  case CodeGenerator::JavaFFI:
    {
      // TODO support PHP namespaces, once HPHP supports it
      string packageName = Option::JavaFFIRootPackage;
      string packageDir = packageName;
      Util::replaceAll(packageDir, ".", "/");

      string outputDir = ar->getOutputPath() + "/" + Option::FFIFilePrefix +
        packageDir + "/";
      Util::mkdir(outputDir);

      // uses a different cg to generate a separate file for each PHP class
      string clsFile = outputDir + getOriginalName() + ".java";
      ofstream fcls(clsFile.c_str());
      CodeGenerator cgCls(&fcls, CodeGenerator::FileCPP);
      cgCls.setContext(CodeGenerator::JavaFFIInterface);

      cgCls.printf("package %s;\n\n", packageName.c_str());
      cgCls.printf("import hphp.*;\n\n");

      cgCls.printf("public interface %s", getOriginalName().c_str());
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
              cgCls.printf(" extends ");
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
      if (m_stmt) m_stmt->outputCPP(cgCls, ar);
      cgCls.indentEnd("}\n");

      fcls.close();
    }
    break;
  case CodeGenerator::JavaFFICppDecl:
  case CodeGenerator::JavaFFICppImpl:
    // do nothing
    break;
  default:
    ASSERT(false);
    break;
  }

  ar->popScope();
}
