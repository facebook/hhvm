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

#include <compiler/expression/class_constant_expression.h>
#include <compiler/analysis/class_scope.h>
#include <compiler/analysis/file_scope.h>
#include <compiler/analysis/constant_table.h>
#include <compiler/analysis/code_error.h>
#include <util/hash.h>
#include <util/util.h>
#include <compiler/option.h>
#include <compiler/analysis/variable_table.h>
#include <compiler/expression/scalar_expression.h>
#include <compiler/expression/constant_expression.h>

using namespace HPHP;
using namespace std;
using namespace boost;

///////////////////////////////////////////////////////////////////////////////

// constructors/destructors

ClassConstantExpression::ClassConstantExpression
(EXPRESSION_CONSTRUCTOR_PARAMETERS,
 ExpressionPtr classExp, const std::string &varName)
  : Expression(EXPRESSION_CONSTRUCTOR_PARAMETER_VALUES),
    StaticClassName(classExp), m_varName(varName), m_defScope(NULL),
    m_valid(false) {
}

ExpressionPtr ClassConstantExpression::clone() {
  ClassConstantExpressionPtr exp(new ClassConstantExpression(*this));
  Expression::deepCopy(exp);
  exp->m_class = Clone(m_class);
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
    }
    addUserClass(ar, m_className);
  }
}

ConstructPtr ClassConstantExpression::getNthKid(int n) const {
  switch (n) {
    case 0:
      return m_class;
    default:
      ASSERT(false);
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
      m_class = boost::dynamic_pointer_cast<Expression>(cp);
      break;
    default:
      ASSERT(false);
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
  if (!cls || (cls->isVolatile() && !isPresent())) return ExpressionPtr();

  ConstantTablePtr constants = cls->getConstants();
  ClassScopePtr defClass = cls;
  ConstructPtr decl = constants->getValueRecur(ar, m_varName, defClass);
  if (decl) {
    BlockScope::s_constMutex.lock();
    ExpressionPtr value = dynamic_pointer_cast<Expression>(decl);
    BlockScope::s_constMutex.unlock();
    if (value->isScalar()) {
      ExpressionPtr rep = Clone(value, getScope());
      bool annotate = Option::FlAnnotate;
      Option::FlAnnotate = false; // avoid nested comments on getText
      rep->setComment(getText());
      Option::FlAnnotate = annotate;
      rep->setLocation(getLocation());
      if (!value->is(KindOfScalarExpression)) {
        value->getScope()->addUse(getScope(), BlockScope::UseKindConstRef);
      }
      return replaceValue(rep);
    }
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

  ClassScopePtr cls = resolveClass();
  if (!cls) {
    if (isRedeclared()) {
      getScope()->getVariables()->
        setAttribute(VariableTable::NeedGlobalPointer);
    } else if (getScope()->isFirstPass()) {
      Compiler::Error(Compiler::UnknownClass, self);
    }
    return Type::Variant;
  }

  if (cls->getConstants()->isDynamic(m_varName) || !isPresent()) {
    getScope()->getVariables()->
      setAttribute(VariableTable::NeedGlobalPointer);
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
  TypePtr t = cls->checkConst(m_varName, type, coerce, ar,
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
  int64 val =
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
// code generation functions

void ClassConstantExpression::outputPHP(CodeGenerator &cg,
                                        AnalysisResultPtr ar) {
  StaticClassName::outputPHP(cg, ar);
  cg_printf("::%s", m_varName.c_str());
}

void ClassConstantExpression::outputCPPImpl(CodeGenerator &cg,
                                            AnalysisResultPtr ar) {
  if (m_class) {
    cg_printf("get_class_constant(");
    if (m_class->is(KindOfScalarExpression)) {
      ASSERT(strcasecmp(dynamic_pointer_cast<ScalarExpression>(m_class)->
                        getString().c_str(), "static") == 0);
      cg_printf("FrameInjection::GetStaticClassName(fi.getThreadInfo())"
                ".data()");
    } else {
      cg_printf("get_static_class_name(");
      m_class->outputCPP(cg, ar);
      cg_printf(").data()");
    }
    cg_printf(", \"%s\")", m_varName.c_str());
    return;
  }

  bool outsideClass = !isPresent();
  if (m_valid) {
    string trueClassName;

    ASSERT(m_defScope);
    ClassScope *cls = dynamic_cast<ClassScope*>(m_defScope);
    trueClassName = cls->getName();
    ASSERT(!trueClassName.empty());
    if (outsideClass) {
      cls->outputVolatileCheckBegin(cg, ar, getScope(), m_origClassName);
    }
    if (cls->getConstants()->isDynamic(m_varName)) {
      cg_printf("%s%s::lazy_initializer(%s)->", Option::ClassPrefix,
                cls->getId(cg).c_str(), cg.getGlobals(ar));
      if (cg.isFileOrClassHeader()) {
        if (getClassScope()) {
          getClassScope()->addUsedClassFullHeader(trueClassName);
        } else {
          getFileScope()->addUsedClassFullHeader(trueClassName);
        }
      }
    } else if (cg.isFileOrClassHeader()) {
      if (getClassScope()) {
        getClassScope()->addUsedClassConstHeader(trueClassName, m_varName);
      } else {
        getFileScope()->addUsedClassConstHeader(trueClassName, m_varName);
      }
    }
    cg_printf("%s%s_%s", Option::ClassConstantPrefix, cls->getId(cg).c_str(),
              m_varName.c_str());
    if (outsideClass) {
      cls->outputVolatileCheckEnd(cg);
    }
  } else if (isRedeclared()) {
    if (outsideClass) {
      ClassScope::OutputVolatileCheckBegin(cg, ar, getScope(), m_origClassName);
    }
    cg_printf("%s->%s%s->os_constant(\"%s\")", cg.getGlobals(ar),
              Option::ClassStaticsObjectPrefix,
              m_className.c_str(), m_varName.c_str());
    if (outsideClass) {
      ClassScope::OutputVolatileCheckEnd(cg);
    }
  } else if (m_defScope) { // !m_valid && m_defScope -> derives from redeclaring
    cg_printf("%s%s::os_constant(\"%s\")", Option::ClassPrefix,
              m_defScope->getId(cg).c_str(), m_varName.c_str());
  } else {
    cg_printf("throw_fatal(\"unknown class constant %s::%s\")",
              m_className.c_str(), m_varName.c_str());
  }
}
