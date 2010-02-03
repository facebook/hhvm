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

#include <lib/expression/function_call.h>
#include <util/util.h>
#include <lib/expression/scalar_expression.h>
#include <lib/analysis/code_error.h>
#include <lib/analysis/function_scope.h>
#include <lib/analysis/dependency_graph.h>
#include <lib/statement/statement.h>
#include <lib/analysis/class_scope.h>
#include <lib/expression/expression_list.h>

using namespace HPHP;
using namespace std;
using namespace boost;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

FunctionCall::FunctionCall
(EXPRESSION_CONSTRUCTOR_PARAMETERS,
 ExpressionPtr nameExp, const std::string &name, ExpressionListPtr params,
 const std::string *classname)
  : Expression(EXPRESSION_CONSTRUCTOR_PARAMETER_VALUES),
    m_nameExp(nameExp), m_params(params), m_valid(false), m_validClass(false),
    m_extraArg(0), m_variableArgument(false), m_voidReturn(false),
    m_voidWrapper(false), m_allowVoidReturn(false), m_redeclared(false),
    m_redeclaredClass(false), m_derivedFromRedeclaring(false) {

  if (m_nameExp &&
      m_nameExp->getKindOf() == Expression::KindOfScalarExpression) {
    ASSERT(m_name.empty());
    ScalarExpressionPtr c = dynamic_pointer_cast<ScalarExpression>(m_nameExp);
    m_origName = c->getString();
    c->toLower();
    m_name = c->getString();
    ASSERT(!m_name.empty());
  } else {
    m_origName = name;
    m_name = Util::toLower(name);
  }

  if (classname) m_className = Util::toLower(*classname);
}

void FunctionCall::reset() {
  m_valid = false;
  m_validClass = false;
  m_extraArg = 0;
  m_variableArgument = false;
  m_voidWrapper = false;
}

ExpressionPtr FunctionCall::preOptimize(AnalysisResultPtr ar) {
  ar->preOptimize(m_nameExp);
  ar->preOptimize(m_params);
  return ExpressionPtr();
}

ExpressionPtr FunctionCall::postOptimize(AnalysisResultPtr ar) {
  ar->postOptimize(m_nameExp);
  ar->postOptimize(m_params);
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
  m_valid = true;
  m_extraArg = func->inferParamTypes(ar, self, m_params, m_valid);
  m_variableArgument = func->isVariableArgument();

  return type;
}

void FunctionCall::outputCPP(CodeGenerator &cg, AnalysisResultPtr ar) {
  if (m_voidWrapper) {
    cg.printf("(");
  }
  Expression::outputCPP(cg, ar);
  if (m_voidWrapper) {
    cg.printf(", null)");
  }
}

void FunctionCall::setFunctionAndClassScope(FunctionScopePtr fsp,
                                            ClassScopePtr csp) {
  m_funcScope = fsp;
  m_classScope = csp;
}
