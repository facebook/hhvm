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

#include <lib/expression/class_constant_expression.h>
#include <lib/analysis/class_scope.h>
#include <lib/analysis/constant_table.h>
#include <lib/analysis/code_error.h>
#include <lib/analysis/dependency_graph.h>
#include <util/util.h>
#include <lib/option.h>
#include <lib/analysis/variable_table.h>
#include <lib/expression/scalar_expression.h>
#include <lib/expression/constant_expression.h>

using namespace HPHP;
using namespace std;
using namespace boost;

///////////////////////////////////////////////////////////////////////////////

// constructors/destructors

ClassConstantExpression::ClassConstantExpression
(EXPRESSION_CONSTRUCTOR_PARAMETERS,
 const std::string &className, const std::string &varName)
  : Expression(EXPRESSION_CONSTRUCTOR_PARAMETER_VALUES),
    m_varName(varName), m_valid(false), m_redeclared(false),
    m_visited(false) {
  m_className = Util::toLower(className);
}

ExpressionPtr ClassConstantExpression::clone() {
  ClassConstantExpressionPtr exp(new ClassConstantExpression(*this));
  Expression::deepCopy(exp);
  return exp;
}

///////////////////////////////////////////////////////////////////////////////
// parser functions

///////////////////////////////////////////////////////////////////////////////
// static analysis functions

void ClassConstantExpression::analyzeProgram(AnalysisResultPtr ar) {
  addUserClass(ar, m_className);
}

ExpressionPtr ClassConstantExpression::preOptimize(AnalysisResultPtr ar) {
  if (ar->getPhase() < AnalysisResult::FirstPreOptimize) {
    return ExpressionPtr();
  }
  if (m_redeclared) return ExpressionPtr();
  ClassScopePtr cls = ar->resolveClass(m_className);
  if (!cls || cls->isRedeclaring()) return ExpressionPtr();
  ConstantTablePtr constants = cls->getConstants();
  if (constants->isExplicitlyDeclared(m_varName)) {
    ConstructPtr decl = constants->getValue(m_varName);
    if (decl) {
      ExpressionPtr value = dynamic_pointer_cast<Expression>(decl);
      if (!m_visited) {
        m_visited = true;
        ar->pushScope(cls);
        ExpressionPtr optExp = value->preOptimize(ar);
        ar->popScope();
        m_visited = false;
        if (optExp) value = optExp;
      }
      if (value->isScalar()) {
        // inline the value
        if (value->is(Expression::KindOfScalarExpression)) {
          ScalarExpressionPtr exp =
            dynamic_pointer_cast<ScalarExpression>(Clone(value));
          exp->setComment(getText());
          return exp;
        } else if (value->is(Expression::KindOfConstantExpression)) {
          // inline the value
          ConstantExpressionPtr exp =
            dynamic_pointer_cast<ConstantExpression>(Clone(value));
          exp->setComment(getText());
          return exp;
        }
      }
    }
  }
  return ExpressionPtr();
}

ExpressionPtr ClassConstantExpression::postOptimize(AnalysisResultPtr ar) {
  return ExpressionPtr();
}

TypePtr ClassConstantExpression::inferTypes(AnalysisResultPtr ar,
                                            TypePtr type, bool coerce) {
  m_valid = false;
  ConstructPtr self = shared_from_this();
  ClassScopePtr cls = ar->resolveClass(m_className);
  if (!cls || cls->isRedeclaring()) {
    if (cls) {
      m_redeclared = true;
      ar->getScope()->getVariables()->
        setAttribute(VariableTable::NeedGlobalPointer);
    }
    if (!cls && ar->isFirstPass()) {
      ar->getCodeError()->record(self, CodeError::UnknownClass, self);
    }
    return type;
  }
  if (cls->getConstants()->isDynamic(m_varName)) {
    ar->getScope()->getVariables()->
      setAttribute(VariableTable::NeedGlobalPointer);
  }
  if (cls->getConstants()->isExplicitlyDeclared(m_varName)) {
    string name = m_className + "::" + m_varName;
    ConstructPtr decl = cls->getConstants()->getDeclaration(m_varName);
    if (decl) { // No decl means an extension class.
      ar->getDependencyGraph()->add(DependencyGraph::KindOfConstant,
                                    ar->getName(),
                                    name, shared_from_this(), name, decl);
    }
    m_valid = true;
  }
  bool present;
  TypePtr t = cls->checkConst(m_varName, type, coerce, ar,
                              shared_from_this(), present);
  if (present) {
    m_valid = true;
  }

  return t;
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void ClassConstantExpression::outputPHP(CodeGenerator &cg,
                                        AnalysisResultPtr ar) {
  cg.printf("%s::%s", m_className.c_str(), m_varName.c_str());
}

void ClassConstantExpression::outputCPPImpl(CodeGenerator &cg,
                                            AnalysisResultPtr ar) {
  const char *globals = "g";
  if (cg.getContext() == CodeGenerator::CppParameterDefaultValueDecl ||
      cg.getContext() == CodeGenerator::CppParameterDefaultValueImpl) {
    globals = cg.getGlobals();
  }
  if (m_valid) {
    ClassScopePtr foundCls;
    string trueClassName;
    for (ClassScopePtr cls = ar->findClass(m_className);
         cls; cls = cls->getParentScope(ar)) {
      if (cls->getConstants()->isPresent(m_varName)) {
        foundCls = cls;
        trueClassName = cls->getName();
        break;
      }
    }
    ASSERT(!trueClassName.empty());
    ConstructPtr decl = foundCls->getConstants()->getValue(m_varName);
    if (decl) {
      decl->outputCPP(cg, ar);
      if (cg.getContext() == CodeGenerator::CppImplementation ||
          cg.getContext() == CodeGenerator::CppParameterDefaultValueImpl) {
        cg.printf("(%s::%s)", m_className.c_str(), m_varName.c_str());
      } else {
        cg.printf("/* %s::%s */", m_className.c_str(), m_varName.c_str());
      }
    } else {
      if (foundCls->getConstants()->isDynamic(m_varName)) {
        cg.printf("%s%s::lazy_initializer(%s)->", Option::ClassPrefix,
                  trueClassName.c_str(), globals);
      }
      cg.printf("%s%s_%s", Option::ClassConstantPrefix, trueClassName.c_str(),
                m_varName.c_str());
    }
  } else if (m_redeclared) {
    cg.printf("%s->%s%s->os_constant(\"%s\")", globals,
              Option::ClassStaticsObjectPrefix,
              m_className.c_str(), m_varName.c_str());
  } else {
    cg.printf("throw_fatal(\"unknown class constant %s::%s\")",
              m_className.c_str(), m_varName.c_str());
  }
}
