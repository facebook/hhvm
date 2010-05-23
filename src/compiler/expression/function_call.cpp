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

#include <compiler/expression/function_call.h>
#include <util/util.h>
#include <compiler/expression/scalar_expression.h>
#include <compiler/analysis/code_error.h>
#include <compiler/analysis/function_scope.h>
#include <compiler/analysis/dependency_graph.h>
#include <compiler/statement/statement.h>
#include <compiler/analysis/class_scope.h>
#include <compiler/expression/expression_list.h>
#include <compiler/expression/array_pair_expression.h>

using namespace HPHP;
using namespace std;
using namespace boost;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

FunctionCall::FunctionCall
(EXPRESSION_CONSTRUCTOR_PARAMETERS,
 ExpressionPtr nameExp, const std::string &name, ExpressionListPtr params,
 ExpressionPtr classExp)
  : Expression(EXPRESSION_CONSTRUCTOR_PARAMETER_VALUES),
    m_nameExp(nameExp), m_params(params), m_class(classExp),
    m_valid(false), m_validClass(false),
    m_extraArg(0), m_variableArgument(false), m_voidReturn(false),
    m_voidWrapper(false), m_allowVoidReturn(false), m_redeclared(false),
    m_redeclaredClass(false), m_derivedFromRedeclaring(false),
    m_argArrayId(-1) {

  if (m_nameExp &&
      m_nameExp->getKindOf() == Expression::KindOfScalarExpression) {
    ASSERT(m_name.empty());
    ScalarExpressionPtr c = dynamic_pointer_cast<ScalarExpression>(m_nameExp);
    m_origName = c->getString();
    c->toLower(true /* func call*/);
    m_name = c->getString();
    ASSERT(!m_name.empty());
  } else {
    m_origName = name;
    m_name = Util::toLower(name);
  }

  if (m_class && m_class->is(KindOfScalarExpression)) {
    ScalarExpressionPtr s(dynamic_pointer_cast<ScalarExpression>(m_class));
    const string &className = s->getString();
    m_className = Util::toLower(className);
    if (m_className == "static") {
      m_className.clear();
    } else {
      m_origClassName = className;
      m_class.reset();
    }
  }
}

void FunctionCall::reset() {
  m_valid = false;
  m_validClass = false;
  m_extraArg = 0;
  m_variableArgument = false;
  m_voidWrapper = false;
}

ConstructPtr FunctionCall::getNthKid(int n) const {
  switch (n) {
    case 0:
      return m_nameExp;
    case 1:
      return m_params;
    case 2:
      return m_class;
    default:
      ASSERT(false);
      break;
  }
  return ConstructPtr();
}

int FunctionCall::getKidCount() const {
  return 3;
}

void FunctionCall::setNthKid(int n, ConstructPtr cp) {
  switch (n) {
    case 0:
      m_nameExp = boost::dynamic_pointer_cast<Expression>(cp);
      break;
    case 1:
      m_params = boost::dynamic_pointer_cast<ExpressionList>(cp);
      break;
    case 2:
      m_class = boost::dynamic_pointer_cast<Expression>(cp);
      break;
    default:
      ASSERT(false);
      break;
  }
}

ExpressionPtr FunctionCall::preOptimize(AnalysisResultPtr ar) {
  if (m_class) ar->preOptimize(m_class);
  ar->preOptimize(m_nameExp);
  ar->preOptimize(m_params);
  return ExpressionPtr();
}

ExpressionPtr FunctionCall::postOptimize(AnalysisResultPtr ar) {
  if (m_class) ar->postOptimize(m_class);
  ar->postOptimize(m_nameExp);
  ar->postOptimize(m_params);
  optimizeArgArray(ar);
  return ExpressionPtr();
}

///////////////////////////////////////////////////////////////////////////////

TypePtr FunctionCall::checkParamsAndReturn(AnalysisResultPtr ar,
                                           TypePtr type, bool coerce,
                                           FunctionScopePtr func) {
  ConstructPtr self = shared_from_this();
  ar->getDependencyGraph()->add(DependencyGraph::KindOfFunctionCall,
                                ar->getName(), getText(),
                                self, func->getFullName(), func->getStmt());
  TypePtr frt = func->getReturnType();
  if (!frt) {
    m_voidReturn = true;
    setActualType(TypePtr());
    if (!type->is(Type::KindOfAny)) {
      if (!m_allowVoidReturn && ar->isSecondPass() && !func->isAbstract()) {
        ar->getCodeError()->record(self, CodeError::UseVoidReturn, self);
      }
      m_voidWrapper = true;
    }
  } else {
    m_voidReturn = false;
    m_voidWrapper = false;
    type = checkTypesImpl(ar, type, frt, coerce);
  }
  m_extraArg = func->inferParamTypes(ar, self, m_params, m_valid);
  m_variableArgument = func->isVariableArgument();
  if (m_valid) {
    m_implementedType.reset();
  } else {
    m_implementedType = Type::Variant;
  }

  return type;
}

void FunctionCall::outputCPP(CodeGenerator &cg, AnalysisResultPtr ar) {
  bool staticClassName = false;
  if (!m_className.empty() && m_cppTemp.empty() &&
      m_origClassName != "self" &&
      m_origClassName != "parent") {
    cg.printf("STATIC_CLASS_NAME_CALL(%s, ", m_className.c_str());
    if (m_voidReturn) m_voidWrapper = true;
    staticClassName = true;
  }

  bool wrap = m_voidWrapper && m_cppTemp.empty();
  if (wrap) {
    cg.printf("(");
    // void wrapper means void return, means we can't put ref around the call
    clearContext(RefValue);
  }
  Expression::outputCPP(cg, ar);
  if (wrap) {
    cg.printf(", null)");
  }

  if (staticClassName) {
    cg.printf(")");
  }
}

void FunctionCall::setFunctionAndClassScope(FunctionScopePtr fsp,
                                            ClassScopePtr csp) {
  m_funcScope = fsp;
  m_classScope = csp;
}

void FunctionCall::optimizeArgArray(AnalysisResultPtr ar) {
  if (m_extraArg <= 0) return;
  int paramCount = m_params->getOutputCount();
  int iMax = paramCount - m_extraArg;
  bool isScalar = true;
  for (int i = iMax; i < paramCount; i++) {
    ExpressionPtr param = (*m_params)[i];
    if (!param->isScalar()) {
      isScalar = false;
      break;
    }
  }
  if (isScalar) {
    ExpressionPtr argArrayPairs =
      ExpressionListPtr(new ExpressionList(getLocation(),
                                           Expression::KindOfExpressionList));
    for (int i = iMax; i < paramCount; i++) {
      ExpressionPtr param = (*m_params)[i];
      argArrayPairs->addElement(ArrayPairExpressionPtr(
        new ArrayPairExpression(param->getLocation(),
                                Expression::KindOfArrayPairExpression,
                                ExpressionPtr(), param, false)));
    }
    m_argArrayId = ar->registerScalarArray(argArrayPairs);
  }
}
