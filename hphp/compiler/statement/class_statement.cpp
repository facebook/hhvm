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
#include "hphp/util/text-util.h"
#include "hphp/compiler/statement/interface_statement.h"
#include "hphp/compiler/statement/use_trait_statement.h"
#include "hphp/compiler/statement/class_require_statement.h"
#include "hphp/compiler/option.h"
#include <sstream>
#include <algorithm>
#include <vector>

using namespace HPHP;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

ClassStatement::ClassStatement
(STATEMENT_CONSTRUCTOR_PARAMETERS,
 int type, const std::string &name, const std::string &parent,
 ExpressionListPtr base, const std::string &docComment, StatementListPtr stmt,
 ExpressionListPtr attrList,
 TypeAnnotationPtr enumBaseTy)
  : InterfaceStatement(STATEMENT_CONSTRUCTOR_PARAMETER_VALUES(ClassStatement),
                       name, base, docComment, stmt, attrList),
    m_type(type), m_ignored(false), m_enumBaseTy(enumBaseTy) {
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
  ClassScope::KindOf kindOf = ClassScope::KindOf::ObjectClass;
  switch (m_type) {
    case T_CLASS:     kindOf = ClassScope::KindOf::ObjectClass;   break;
    case T_ABSTRACT:  kindOf = ClassScope::KindOf::AbstractClass; break;
    case T_STATIC: // Slight hack: see comments in hphp.y
      kindOf = ClassScope::KindOf::UtilClass;     break;
    case T_FINAL:     kindOf = ClassScope::KindOf::FinalClass;    break;
    case T_TRAIT:     kindOf = ClassScope::KindOf::Trait;         break;
    case T_ENUM:      kindOf = ClassScope::KindOf::Enum;          break;
    default:
      assert(false);
  }

  std::vector<std::string> bases;
  if (!m_originalParent.empty()) {
    bases.push_back(m_originalParent);
  }
  if (m_base) m_base->getStrings(bases);

  for (auto &b : bases) {
    ar->parseOnDemandByClass(b);
  }

  std::vector<UserAttributePtr> attrs;
  if (m_attrList) {
      for (int i = 0; i < m_attrList->getCount(); ++i) {
      auto a = dynamic_pointer_cast<UserAttribute>((*m_attrList)[i]);
      attrs.push_back(a);
    }
  }

  auto stmt = dynamic_pointer_cast<Statement>(shared_from_this());
  auto classScope = std::make_shared<ClassScope>(
    fs, kindOf, m_originalName,
    m_originalParent,
    bases, m_docComment,
    stmt, attrs);

  setBlockScope(classScope);
  if (!fs->addClass(ar, classScope)) {
    m_ignored = true;
    return;
  }

  classScope->setPersistent(false);

  if (m_stmt) {
    MethodStatementPtr constructor = nullptr;
    MethodStatementPtr destructor = nullptr;
    MethodStatementPtr clone = nullptr;

    // flatten continuation StatementList into MethodStatements
    for (int i = 0; i < m_stmt->getCount(); i++) {
      auto stmts = dynamic_pointer_cast<StatementList>((*m_stmt)[i]);
      if (stmts) {
        m_stmt->removeElement(i);
        for (int j = 0; j < stmts->getCount(); j++) {
          m_stmt->insertElement((*stmts)[j], i + j);
        }
      }
    }

    for (int i = 0; i < m_stmt->getCount(); i++) {
      auto meth = dynamic_pointer_cast<MethodStatement>((*m_stmt)[i]);
      if (meth) {
        if (meth->isNamed("__construct")) {
          constructor = meth;
          continue;
        }
        if (meth->isNamed("__destruct")) {
          destructor = meth;
          continue;
        }
        if (meth->isNamed("__clone")) {
          clone = meth;
          continue;
        }
      }
      if (constructor && destructor && clone) {
        break;
      }
    }

    for (int i = 0; i < m_stmt->getCount(); i++) {
      if (!constructor) {
        auto meth = dynamic_pointer_cast<MethodStatement>((*m_stmt)[i]);
        if (meth &&
            meth->isNamed(classScope->getOriginalName()) &&
            !classScope->isTrait()) {
          // class-name constructor
          constructor = meth;
          classScope->setAttribute(ClassScope::ClassNameConstructor);
        }
      }
      auto ph = dynamic_pointer_cast<IParseHandler>((*m_stmt)[i]);
      ph->onParseRecur(ar, fs, classScope);
    }
    if (constructor && constructor->getModifiers()->isStatic()) {
      constructor->parseTimeFatal(fs,
                                  Compiler::InvalidAttribute,
                                  "Constructor %s::%s() cannot be static",
                                  classScope->getOriginalName().c_str(),
                                  constructor->getOriginalName().c_str());
    }
    if (destructor && destructor->getModifiers()->isStatic()) {
      destructor->parseTimeFatal(fs,
                                 Compiler::InvalidAttribute,
                                 "Destructor %s::%s() cannot be static",
                                 classScope->getOriginalName().c_str(),
                                 destructor->getOriginalName().c_str());
    }
    if (clone && clone->getModifiers()->isStatic()) {
      clone->parseTimeFatal(fs,
                            Compiler::InvalidAttribute,
                            "Clone method %s::%s() cannot be static",
                            classScope->getOriginalName().c_str(),
                            clone->getOriginalName().c_str());
    }
  }
}

StatementPtr ClassStatement::addClone(StatementPtr origStmt) {
  assert(m_stmt);
  auto newStmt = Clone(origStmt);
  m_stmt->addElement(newStmt);
  return newStmt;
}

///////////////////////////////////////////////////////////////////////////////
// static analysis functions

std::string ClassStatement::getName() const {
  return std::string("Class ") + getOriginalName();
}

void ClassStatement::analyzeProgram(AnalysisResultPtr ar) {
  std::vector<std::string> bases;
  auto const hasParent = !m_originalParent.empty();
  if (hasParent) bases.push_back(m_originalParent);
  if (m_base) m_base->getStrings(bases);

  checkVolatile(ar);

  if (m_stmt) {
    m_stmt->analyzeProgram(ar);
  }

  if (ar->getPhase() != AnalysisResult::AnalyzeAll) return;

  for (unsigned int i = 0; i < bases.size(); i++) {
    ClassScopePtr cls = ar->findClass(bases[i]);
    if (cls) {
      auto const expectClass = hasParent && i == 0;
      if (expectClass == cls->isInterface() || cls->isTrait()) {
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

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void ClassStatement::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  ClassScopeRawPtr classScope = getClassScope();
  if (!classScope->isUserClass()) return;

  if (m_type == T_TRAIT) {
    cg_printf("trait %s", m_originalName.c_str());
  } else {
    switch (m_type) {
      case T_CLASS:                                  break;
      case T_ABSTRACT: cg_printf("abstract ");       break;
      case T_FINAL:    cg_printf("final ");          break;
      case T_STATIC:   cg_printf("abstract final "); break;
      default:
        assert(false);
    }
    cg_printf("class %s", m_originalName.c_str());
  }

  if (!m_originalParent.empty()) {
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
