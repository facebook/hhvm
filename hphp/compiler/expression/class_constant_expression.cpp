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

#include "hphp/compiler/expression/class_constant_expression.h"
#include "hphp/compiler/analysis/class_scope.h"
#include "hphp/compiler/analysis/file_scope.h"
#include "hphp/compiler/analysis/constant_table.h"
#include "hphp/compiler/analysis/code_error.h"
#include "hphp/util/hash.h"
#include "hphp/util/util.h"
#include "hphp/compiler/option.h"
#include "hphp/compiler/analysis/variable_table.h"
#include "hphp/compiler/expression/scalar_expression.h"
#include "hphp/compiler/expression/constant_expression.h"

using namespace HPHP;

///////////////////////////////////////////////////////////////////////////////

// constructors/destructors

ClassConstantExpression::ClassConstantExpression
(EXPRESSION_CONSTRUCTOR_PARAMETERS,
 ExpressionPtr classExp, const std::string &varName)
  : Expression(
      EXPRESSION_CONSTRUCTOR_PARAMETER_VALUES(ClassConstantExpression)),
    StaticClassName(classExp), m_varName(varName), m_defScope(nullptr),
    m_valid(false), m_depsSet(false) {
}

ExpressionPtr ClassConstantExpression::clone() {
  ClassConstantExpressionPtr exp(new ClassConstantExpression(*this));
  Expression::deepCopy(exp);
  exp->m_class = Clone(m_class);
  exp->m_depsSet = false;
  return exp;
}

///////////////////////////////////////////////////////////////////////////////
// parser functions

///////////////////////////////////////////////////////////////////////////////
// static analysis functions

bool ClassConstantExpression::containsDynamicConstant(AnalysisResultPtr ar)
  const {
  if (m_class) return true;
  ClassScopePtr cls = ar->findClass(m_className);
  return !cls || cls->isVolatile() ||
    !cls->getConstants()->isRecursivelyDeclared(ar, m_varName);
}

void ClassConstantExpression::analyzeProgram(AnalysisResultPtr ar) {
  if (m_class) {
    m_class->analyzeProgram(ar);
  } else if (ar->getPhase() >= AnalysisResult::AnalyzeAll) {
    if (ClassScopePtr cls = resolveClass()) {
      ConstructPtr decl = cls->getConstants()->
        getValueRecur(ar, m_varName, cls);
      cls->addUse(getScope(), BlockScope::UseKindConstRef);
      m_depsSet = true;
      if (ar->getPhase() == AnalysisResult::AnalyzeFinal) {
        if (!isPresent()) {
          getScope()->getVariables()->
            setAttribute(VariableTable::NeedGlobalPointer);
        }
      }
    }
    addUserClass(ar, m_className);
  }
}

string ClassConstantExpression::getActualClassName() const {
  if (m_defScope) {
    return static_cast<ClassScope*>(m_defScope)->getId();
  }
  return m_className;
}

ConstructPtr ClassConstantExpression::getNthKid(int n) const {
  switch (n) {
    case 0:
      return m_class;
    default:
      assert(false);
      break;
  }
  return ConstructPtr();
}

int ClassConstantExpression::getKidCount() const {
  return 1;
}

void ClassConstantExpression::setNthKid(int n, ConstructPtr cp) {
  switch (n) {
    case 0:
      m_class = dynamic_pointer_cast<Expression>(cp);
      break;
    default:
      assert(false);
      break;
  }
}

ExpressionPtr ClassConstantExpression::preOptimize(AnalysisResultConstPtr ar) {
  if (ar->getPhase() < AnalysisResult::FirstPreOptimize) {
    return ExpressionPtr();
  }
  if (m_class) {
    updateClassName();
    if (m_class) {
      return ExpressionPtr();
    }
  }

  ClassScopePtr cls = resolveClass();
  if (!cls || (cls->isVolatile() && !isPresent())) {
    if (cls && !m_depsSet) {
      cls->addUse(getScope(), BlockScope::UseKindConstRef);
      m_depsSet = true;
    }
    return ExpressionPtr();
  }

  ConstantTablePtr constants = cls->getConstants();
  ClassScopePtr defClass = cls;
  ConstructPtr decl = constants->getValueRecur(ar, m_varName, defClass);
  if (decl) {
    BlockScope::s_constMutex.lock();
    ExpressionPtr value = dynamic_pointer_cast<Expression>(decl);
    BlockScope::s_constMutex.unlock();

    if (!value->isScalar() &&
        (value->is(KindOfClassConstantExpression) ||
         value->is(KindOfConstantExpression))) {
      std::set<ExpressionPtr> seen;
      do {
        if (!seen.insert(value).second) return ExpressionPtr();
        value = value->preOptimize(ar);
        if (!value) return ExpressionPtr();
      } while (!value->isScalar() &&
               (value->is(KindOfClassConstantExpression) ||
                value->is(KindOfConstantExpression)));
    }

    ExpressionPtr rep = Clone(value, getScope());
    rep->setComment(getText());
    rep->setLocation(getLocation());
    return replaceValue(rep);
  }
  return ExpressionPtr();
}

TypePtr ClassConstantExpression::inferTypes(AnalysisResultPtr ar,
                                            TypePtr type, bool coerce) {
  m_valid = false;
  ConstructPtr self = shared_from_this();

  if (m_class) {
    m_class->inferAndCheck(ar, Type::Any, false);
    return Type::Variant;
  }

  ClassScopePtr cls = resolveClassWithChecks();
  if (!cls) {
    return Type::Variant;
  }

  ClassScopePtr defClass = cls;
  ConstructPtr decl =
    cls->getConstants()->getDeclarationRecur(ar, m_varName, defClass);

  if (decl) { // No decl means an extension class or derived from redeclaring
    cls = defClass;
    m_valid = true;
    if (cls->isUserClass()) {
      cls->addUse(getScope(), BlockScope::UseKindConstRef);
    }
  }

  BlockScope *defScope;
  // checkConst grabs locks for us
  TypePtr t = cls->checkConst(getScope(), m_varName, type,
                              coerce, ar,
                              shared_from_this(),
                              cls->getBases(), defScope);
  if (defScope) {
    m_valid = true;
    m_defScope = defScope;
  } else if (cls->derivesFromRedeclaring()) {
    m_defScope = cls.get();
  }

  return t;
}

unsigned ClassConstantExpression::getCanonHash() const {
  int64_t val =
    hash_string(Util::toLower(m_varName).c_str(), m_varName.size()) -
    hash_string(Util::toLower(m_className).c_str(), m_className.size());
  return ~unsigned(val) ^ unsigned(val >> 32);
}

bool ClassConstantExpression::canonCompare(ExpressionPtr e) const {
  return Expression::canonCompare(e) &&
    m_varName == static_cast<ClassConstantExpression*>(e.get())->m_varName &&
    m_className == static_cast<ClassConstantExpression*>(e.get())->m_className;
}

///////////////////////////////////////////////////////////////////////////////

void ClassConstantExpression::outputCodeModel(CodeGenerator &cg) {
  cg.printObjectHeader("ClassPropertyExpression", 3);
  cg.printPropertyHeader("className");
  StaticClassName::outputCodeModel(cg);
  cg.printPropertyHeader("propertyName");
  cg.printValue(m_varName);
  cg.printPropertyHeader("sourceLocation");
  cg.printLocation(this->getLocation());
  cg.printObjectFooter();
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void ClassConstantExpression::outputPHP(CodeGenerator &cg,
                                        AnalysisResultPtr ar) {
  StaticClassName::outputPHP(cg, ar);
  cg_printf("::%s", m_varName.c_str());
}

bool ClassConstantExpression::isDynamic() const {
  if (!m_valid) return true;
  return m_defScope->getConstants()->isDynamic(m_varName);
}
