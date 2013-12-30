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

#include "hphp/compiler/statement/class_statement.h"
#include "hphp/parser/hphp.tab.hpp"
#include "hphp/compiler/expression/expression_list.h"
#include "hphp/compiler/statement/statement_list.h"
#include "hphp/compiler/expression/scalar_expression.h"
#include "hphp/compiler/expression/parameter_expression.h"
#include "hphp/compiler/analysis/class_scope.h"
#include "hphp/compiler/analysis/file_scope.h"
#include "hphp/compiler/analysis/function_scope.h"
#include "hphp/compiler/analysis/analysis_result.h"
#include "hphp/compiler/statement/method_statement.h"
#include "hphp/compiler/statement/class_variable.h"
#include "hphp/compiler/analysis/variable_table.h"
#include "hphp/compiler/analysis/constant_table.h"
#include "hphp/util/util.h"
#include "hphp/compiler/statement/interface_statement.h"
#include "hphp/compiler/statement/use_trait_statement.h"
#include "hphp/compiler/statement/trait_require_statement.h"
#include "hphp/compiler/code_model_enums.h"
#include "hphp/compiler/option.h"
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
    assert(false);
  }

  vector<string> bases;
  if (!m_originalParent.empty()) {
    bases.push_back(m_originalParent);
  }
  if (m_base) m_base->getOriginalStrings(bases);

  for (auto &b : bases) {
    ar->parseOnDemandByClass(Util::toLower(b));
  }

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
  assert(m_stmt);
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

void ClassStatement::outputCodeModel(CodeGenerator &cg) {
  auto numProps = 3;
  if (m_attrList != nullptr) numProps++;
  if (m_type == T_ABSTRACT || m_type == T_FINAL) numProps++;
  if (!m_parent.empty()) numProps++;
  if (m_base != nullptr) numProps++;
  if (m_stmt != nullptr) numProps++;
  if (!m_docComment.empty()) numProps++;

  cg.printObjectHeader("TypeStatement", numProps);
  if (m_attrList != nullptr) {
    cg.printPropertyHeader("attributes");
    cg.printExpressionVector(m_attrList);
  }
  if (m_type == T_ABSTRACT) {
    cg.printPropertyHeader("modifiers");
    cg.printModifierVector("abstract");
  } else if (m_type == T_FINAL) {
    cg.printPropertyHeader("modifiers");
    cg.printModifierVector("final");
  }
  cg.printPropertyHeader("kind");
  if (m_type == T_TRAIT) {
    cg.printValue(PHP_TRAIT);
  } else {
    cg.printValue(PHP_CLASS);
  }
  cg.printPropertyHeader("name");
  cg.printValue(m_originalName);
  //TODO: type parameters (task 3262469)
  if (!m_parent.empty()) {
    cg.printPropertyHeader("baseClass");
    cg.printTypeExpression(m_originalParent);
  }
  if (m_base != nullptr) {
    cg.printPropertyHeader("interfaces");
    cg.printExpressionVector(m_base);
  }
  if (m_stmt != nullptr) {
    cg.printPropertyHeader("block");
    cg.printAsBlock(m_stmt);
  }
  cg.printPropertyHeader("sourceLocation");
  cg.printLocation(this->getLocation());
  if (!m_docComment.empty()) {
    cg.printPropertyHeader("comments");
    cg.printValue(m_docComment);
  }
  cg.printObjectFooter();
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
        assert(false);
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
  return true;
}
