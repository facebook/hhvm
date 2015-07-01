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

#include "hphp/compiler/statement/interface_statement.h"
#include <vector>
#include "hphp/compiler/expression/scalar_expression.h"
#include "hphp/compiler/expression/expression_list.h"
#include "hphp/compiler/expression/parameter_expression.h"
#include "hphp/compiler/analysis/class_scope.h"
#include "hphp/compiler/analysis/file_scope.h"
#include "hphp/compiler/analysis/analysis_result.h"
#include "hphp/compiler/analysis/function_scope.h"
#include "hphp/compiler/statement/statement_list.h"
#include "hphp/compiler/analysis/variable_table.h"
#include "hphp/util/text-util.h"
#include "hphp/compiler/option.h"
#include "hphp/compiler/code_model_enums.h"
#include "hphp/compiler/parser/parser.h"

using namespace HPHP;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

InterfaceStatement::InterfaceStatement
(STATEMENT_CONSTRUCTOR_BASE_PARAMETERS,
 const std::string &name, ExpressionListPtr base,
 const std::string &docComment, StatementListPtr stmt,
 ExpressionListPtr attrList)
  : Statement(STATEMENT_CONSTRUCTOR_BASE_PARAMETER_VALUES),
    m_originalName(name),
    m_base(base),
    m_docComment(docComment),
    m_stmt(stmt),
    m_attrList(attrList) {
}

InterfaceStatement::InterfaceStatement
(STATEMENT_CONSTRUCTOR_PARAMETERS,
 const std::string &name, ExpressionListPtr base,
 const std::string &docComment, StatementListPtr stmt,
 ExpressionListPtr attrList)
  : InterfaceStatement(
    STATEMENT_CONSTRUCTOR_PARAMETER_VALUES(InterfaceStatement),
    name, base, docComment, stmt, attrList) {
}

StatementPtr InterfaceStatement::clone() {
  InterfaceStatementPtr stmt(new InterfaceStatement(*this));
  stmt->m_stmt = Clone(m_stmt);
  stmt->m_base = Clone(m_base);
  return stmt;
}

bool InterfaceStatement::hasImpl() const {
  return true;
}

int InterfaceStatement::getRecursiveCount() const {
  return m_stmt ? m_stmt->getRecursiveCount() : 0;
}
///////////////////////////////////////////////////////////////////////////////
// parser functions

void InterfaceStatement::onParse(AnalysisResultConstPtr ar,
                                 FileScopePtr scope) {
  vector<string> bases;
  if (m_base) m_base->getStrings(bases);

  for (auto &b : bases) {
    ar->parseOnDemandByClass(b);
  }

  StatementPtr stmt = dynamic_pointer_cast<Statement>(shared_from_this());

  vector<UserAttributePtr> attrs;
  if (m_attrList) {
    for (int i = 0; i < m_attrList->getCount(); ++i) {
      UserAttributePtr a =
        dynamic_pointer_cast<UserAttribute>((*m_attrList)[i]);
      attrs.push_back(a);
    }
  }

  auto classScope =
    std::make_shared<ClassScope>(
      scope, ClassScope::KindOf::Interface, m_originalName, "", bases,
      m_docComment, stmt, attrs);

  setBlockScope(classScope);
  scope->addClass(ar, classScope);

  classScope->setPersistent(false);

  if (m_stmt) {
    for (int i = 0; i < m_stmt->getCount(); i++) {
      IParseHandlerPtr ph = dynamic_pointer_cast<IParseHandler>((*m_stmt)[i]);
      ph->onParseRecur(ar, scope, classScope);
    }
    checkArgumentsToPromote(scope, ExpressionListPtr(), T_INTERFACE);
  }
}

void InterfaceStatement::checkArgumentsToPromote(
  FileScopeRawPtr scope, ExpressionListPtr promotedParams, int type) {
  if (!m_stmt) {
    return;
  }
  for (int i = 0; i < m_stmt->getCount(); i++) {
    MethodStatementPtr meth =
      dynamic_pointer_cast<MethodStatement>((*m_stmt)[i]);
    if (meth && meth->isNamed("__construct")) {
      ExpressionListPtr params = meth->getParams();
      if (params) {
        for (int i = 0; i < params->getCount(); i++) {
          ParameterExpressionPtr param =
            dynamic_pointer_cast<ParameterExpression>((*params)[i]);
          if (param->getModifier() != 0) {
            if (type == T_TRAIT || type == T_INTERFACE) {
              param->parseTimeFatal(scope,
                                    Compiler::InvalidAttribute,
                                    "Constructor parameter promotion "
                                    "not allowed on traits or interfaces");
            }
            if (promotedParams) {
              promotedParams->addElement(param);
            }
          }
        }
      }
      return; // nothing else to look at
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// static analysis functions

int InterfaceStatement::getLocalEffects() const {
  ClassScopeRawPtr classScope = getClassScope();
  return classScope->isVolatile() ? OtherEffect | CanThrow : NoEffect;
}

std::string InterfaceStatement::getName() const {
  return string("Interface ") + m_originalName;
}

bool InterfaceStatement::checkVolatileBases(AnalysisResultConstPtr ar) {
  ClassScopeRawPtr classScope = getClassScope();
  assert(!classScope->isVolatile());
  const vector<string> &bases = classScope->getBases();
  for (vector<string>::const_iterator it = bases.begin();
       it != bases.end(); ++it) {
    ClassScopePtr base = ar->findClass(*it);
    if (base && base->isVolatile()) return true;
  }
  return false;
}

void InterfaceStatement::checkVolatile(AnalysisResultConstPtr ar) {
  ClassScopeRawPtr classScope = getClassScope();
  // redeclared classes/interfaces are automatically volatile
  if (!classScope->isVolatile()) {
     if (checkVolatileBases(ar)) {
       // if any base is volatile, the class is volatile
       classScope->setVolatile();
     }
  }
}

void InterfaceStatement::analyzeProgram(AnalysisResultPtr ar) {
  ClassScopeRawPtr classScope = getClassScope();
  if (m_stmt) {
    m_stmt->analyzeProgram(ar);
  }

  checkVolatile(ar);

  if (ar->getPhase() != AnalysisResult::AnalyzeAll) return;
  vector<string> bases;
  if (m_base) m_base->getStrings(bases);
  for (unsigned int i = 0; i < bases.size(); i++) {
    ClassScopePtr cls = ar->findClass(bases[i]);
    if (cls) {
      if (!cls->isInterface()) {
        Compiler::Error(
          Compiler::InvalidDerivation,
          shared_from_this(),
          cls->getOriginalName() + " must be an interface");
      }
      if (cls->isUserClass()) {
        cls->addUse(classScope, BlockScope::UseKindParentRef);
      }
    }
  }
}

ConstructPtr InterfaceStatement::getNthKid(int n) const {
  switch (n) {
    case 0:
      return m_stmt;
    case 1:
      return m_base;
    default:
      assert(false);
      break;
  }
  return ConstructPtr();
}

int InterfaceStatement::getKidCount() const {
  return 2;
}

void InterfaceStatement::setNthKid(int n, ConstructPtr cp) {
  switch (n) {
    case 0:
      m_stmt = dynamic_pointer_cast<StatementList>(cp);
      break;
    case 1:
      m_base = dynamic_pointer_cast<ExpressionList>(cp);
      break;
    default:
      assert(false);
      break;
  }
}

StatementPtr InterfaceStatement::preOptimize(AnalysisResultConstPtr ar) {
  if (ar->getPhase() >= AnalysisResult::AnalyzeAll) {
    checkVolatile(ar);
  }
  return StatementPtr();
}

///////////////////////////////////////////////////////////////////////////////

void InterfaceStatement::outputCodeModel(CodeGenerator &cg) {
  auto numProps = 4;
  if (m_attrList != nullptr) numProps++;
  if (m_base != nullptr) numProps++;
  if (!m_docComment.empty()) numProps++;

  cg.printObjectHeader("TypeStatement", numProps);
  if (m_attrList != nullptr) {
    cg.printPropertyHeader("attributes");
    cg.printExpressionVector(m_attrList);
  }
  cg.printPropertyHeader("kind");
  cg.printValue(PHP_INTERFACE);
  cg.printPropertyHeader("name");
  cg.printValue(m_originalName);
  //TODO: type parameters (task 3262469)
  if (m_base != nullptr) {
    cg.printPropertyHeader("interfaces");
    cg.printExpressionVector(m_base);
  }
  cg.printPropertyHeader("block");
  cg.printAsEnclosedBlock(m_stmt);
  cg.printPropertyHeader("sourceLocation");
  cg.printLocation(this);
  if (!m_docComment.empty()) {
    cg.printPropertyHeader("comments");
    cg.printValue(m_docComment);
  }
  cg.printObjectFooter();
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void InterfaceStatement::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  ClassScopeRawPtr classScope = getClassScope();

  if (cg.getOutput() == CodeGenerator::InlinedPHP ||
      cg.getOutput() == CodeGenerator::TrimmedPHP) {
    if (!classScope->isUserClass()) {
      return;
    }
  }

  cg_printf("interface %s", m_originalName.c_str());
  if (m_base) {
    cg_printf(" extends ");
    m_base->outputPHP(cg, ar);
  }
  cg_indentBegin(" {\n");
  classScope->outputPHP(cg, ar);
  if (m_stmt) m_stmt->outputPHP(cg, ar);
  cg_indentEnd("}\n");
}
