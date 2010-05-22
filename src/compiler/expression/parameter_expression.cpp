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

#include <compiler/expression/parameter_expression.h>
#include <compiler/analysis/function_scope.h>
#include <compiler/analysis/file_scope.h>
#include <compiler/analysis/variable_table.h>
#include <compiler/analysis/class_scope.h>
#include <compiler/analysis/code_error.h>
#include <util/util.h>
#include <compiler/option.h>
#include <compiler/expression/constant_expression.h>

using namespace HPHP;
using namespace std;
using namespace boost;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

ParameterExpression::ParameterExpression
(EXPRESSION_CONSTRUCTOR_PARAMETERS,
 const std::string &type, const std::string &name, bool ref,
 ExpressionPtr defaultValue)
  : Expression(EXPRESSION_CONSTRUCTOR_PARAMETER_VALUES),
    m_name(name), m_ref(ref), m_hasRTTI(false), m_defaultValue(defaultValue) {
  m_type = Util::toLower(type);
  if (m_defaultValue) {
    m_defaultValue->setContext(InParameterExpression);
  }
}

ExpressionPtr ParameterExpression::clone() {
  ParameterExpressionPtr exp(new ParameterExpression(*this));
  Expression::deepCopy(exp);
  return exp;
}

///////////////////////////////////////////////////////////////////////////////
// parser functions

void ParameterExpression::defaultToNull(AnalysisResultPtr ar) {
  ASSERT(!m_defaultValue);
  m_defaultValue = CONSTANT("null");
}

///////////////////////////////////////////////////////////////////////////////
// static analysis functions

void ParameterExpression::analyzeProgram(AnalysisResultPtr ar) {
  if (!m_type.empty()) addUserClass(ar, m_type);

  if (m_defaultValue) m_defaultValue->analyzeProgram(ar);

  if (ar->isFirstPass()) {
    // Have to use non const ref params for magic methods
    FunctionScopePtr fs = ar->getFunctionScope();
    if (fs->isMagicMethod() || fs->getName() == "offsetget") {
      fs->getVariables()->addLvalParam(m_name);
    }
  }
}

ConstructPtr ParameterExpression::getNthKid(int n) const {
  switch (n) {
    case 0:
      return m_defaultValue;
    default:
      ASSERT(false);
      break;
  }
  return ConstructPtr();
}

int ParameterExpression::getKidCount() const {
  return 1;
}

void ParameterExpression::setNthKid(int n, ConstructPtr cp) {
  switch (n) {
    case 0:
      m_defaultValue = boost::dynamic_pointer_cast<Expression>(cp);
      break;
    default:
      break;
  }
}

ExpressionPtr ParameterExpression::preOptimize(AnalysisResultPtr ar) {
  ar->preOptimize(m_defaultValue);
  return ExpressionPtr();
}

ExpressionPtr ParameterExpression::postOptimize(AnalysisResultPtr ar) {
  ar->postOptimize(m_defaultValue);
  return ExpressionPtr();
}

TypePtr ParameterExpression::inferTypes(AnalysisResultPtr ar, TypePtr type,
                                        bool coerce) {
  ASSERT(type->is(Type::KindOfSome) || type->is(Type::KindOfAny));

  TypePtr ret;
  if (m_type.empty() || m_defaultValue) {
    ret = NEW_TYPE(Some);
  } else if (m_type == "array") {
    ret = Type::Array;
  } else {
    ClassScopePtr cls = ar->findClass(m_type);
    if (!cls || cls->isRedeclaring()) {
      if (!cls && ar->isFirstPass()) {
        ConstructPtr self = shared_from_this();
        ar->getCodeError()->record(self, CodeError::UnknownClass, self);
      }
      ret = NEW_TYPE(Some);
    } else {
      ret = Type::CreateObjectType(m_type);
    }
  }
  if (m_defaultValue) {
    ret = m_defaultValue->inferAndCheck(ar, ret, false);
  }

  // we still want the above to run, so to record errors and infer defaults
  if (m_ref) {
    ret = Type::Variant;
  }

  // parameters are like variables, but we need to remember these are
  // parameters so when variable table is generated, they are not generated
  // as declared variables.
  VariableTablePtr variables = ar->getScope()->getVariables();
  if (ar->isFirstPass()) {
    ret = variables->addParam(m_name, ret, ar, shared_from_this());
  } else {
    // Functions that can be called dynamically have to have
    // variant parameters.
    if (m_type.empty() && (ar->getFunctionScope()->isDynamic() ||
                           ar->getFunctionScope()->isRedeclaring() ||
                           ar->getFunctionScope()->isVirtual())) {
      variables->forceVariant(ar, m_name);
    }
    int p;
    ret = variables->checkVariable(m_name, ret, true, ar, shared_from_this(),
                                   p);
    if (ar->isSecondPass() && ret->is(Type::KindOfSome)) {
      // This is probably too conservative. The problem is that
      // a function never called will have parameter types of Any.
      // Functions that it calls won't be able to accept variant unless
      // it is forced here.
      variables->forceVariant(ar, m_name);
      ret = Type::Variant;
    } else if (ar->getPhase() == AnalysisResult::LastInference &&
               !ret->getName().empty()) {
      addUserClass(ar, ret->getName(), true);
    }
  }
  return ret;
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void ParameterExpression::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  if (!m_type.empty()) cg.printf("%s ", m_type.c_str());
  if (m_ref) cg.printf("&");
  cg.printf("$%s", m_name.c_str());
  if (m_defaultValue) {
    cg.printf(" = ");
    m_defaultValue->outputPHP(cg, ar);
  }
}

void ParameterExpression::outputCPPImpl(CodeGenerator &cg,
                                        AnalysisResultPtr ar) {
  FunctionScopePtr func =
    dynamic_pointer_cast<FunctionScope>(ar->getScope());
  VariableTablePtr variables = func->getVariables();
  TypePtr paramType = func->getParamType(cg.getItemIndex());
  bool isCVarRef = false;
  if (cg.getContext() == CodeGenerator::CppStaticMethodWrapper ||
      (!variables->isLvalParam(m_name) &&
       !variables->getAttribute(VariableTable::ContainsDynamicVariable) &&
       !variables->getAttribute(VariableTable::ContainsExtract) &&
       !m_ref)) {
    if (paramType->is(Type::KindOfVariant) ||
        paramType->is(Type::KindOfSome)) {
      cg.printf("CVarRef");
      isCVarRef = true;
    }
    else if (paramType->is(Type::KindOfArray)) cg.printf("CArrRef");
    else if (paramType->is(Type::KindOfString)) cg.printf("CStrRef");
    else paramType->outputCPPDecl(cg, ar);
  } else {
    paramType->outputCPPDecl(cg, ar);
  }

  cg.printf(" %s%s", Option::VariablePrefix, m_name.c_str());
  if (m_defaultValue) {
    CodeGenerator::Context context = cg.getContext();
    bool comment =  context == CodeGenerator::CppImplementation ||
      (context == CodeGenerator::CppDeclaration && func->isInlined());
    if (comment) {
      cg.printf(" // ");
    }
    cg.printf(" = ");
    ConstantExpressionPtr con =
      dynamic_pointer_cast<ConstantExpression>(m_defaultValue);
    if (isCVarRef && con && con->isNull()) {
      cg.printf("null_variant");
    } else {
      if (comment) {
        cg.setContext(CodeGenerator::CppParameterDefaultValueImpl);
      } else {
        cg.setContext(CodeGenerator::CppParameterDefaultValueDecl);
      }
      m_defaultValue->outputCPP(cg, ar);
      cg.setContext(context);
    }
    if (comment) {
      cg.printf("\n");
    }
  }
}
