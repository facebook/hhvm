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
    m_originalType(type), m_name(name), m_ref(ref), m_hasRTTI(false),
    m_defaultValue(defaultValue) {
  m_type = Util::toLower(type);
  if (m_defaultValue) {
    m_defaultValue->setContext(InParameterExpression);
  }
}

ExpressionPtr ParameterExpression::clone() {
  ParameterExpressionPtr exp(new ParameterExpression(*this));
  Expression::deepCopy(exp);
  exp->m_defaultValue = Clone(m_defaultValue);
  return exp;
}

///////////////////////////////////////////////////////////////////////////////
// parser functions

void ParameterExpression::parseHandler(AnalysisResultConstPtr ar,
                                       FunctionScopePtr func,
                                       ClassScopePtr cls) {
  if (cls && !m_type.empty()) {
    if (m_type == "self") {
      m_type = cls->getName();
    } else if (m_type == "parent") {
      if (!cls->getParent().empty()) {
        m_type = cls->getParent();
      }
    }
  }
  func->getVariables()->addParam(m_name, TypePtr(), ar, ExpressionPtr());
}

void ParameterExpression::defaultToNull(AnalysisResultPtr ar) {
  ASSERT(!m_defaultValue);
  m_defaultValue = CONSTANT("null");
}

///////////////////////////////////////////////////////////////////////////////
// static analysis functions

void ParameterExpression::analyzeProgram(AnalysisResultPtr ar) {
  if (m_defaultValue) m_defaultValue->analyzeProgram(ar);

  if (ar->getPhase() == AnalysisResult::AnalyzeFinal) {
    if (!m_type.empty()) {
      addUserClass(ar, m_type);
    }
    // Have to use non const ref params for magic methods
    FunctionScopePtr fs = getFunctionScope();
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

TypePtr ParameterExpression::getTypeSpec(AnalysisResultPtr ar,
                                         bool forInference) {
  const Type::TypePtrMap &types = Type::GetTypeHintTypes();
  Type::TypePtrMap::const_iterator iter;

  TypePtr ret;
  if (m_type.empty() || (forInference && m_defaultValue)) {
    ret = Type::Some;
  } else if ((iter = types.find(m_type)) != types.end()) {
    ret = iter->second;
  } else {
    if (forInference) {
      ClassScopePtr cls = ar->findClass(m_type);
      if (Option::SystemGen ||
          !cls || cls->isRedeclaring() || cls->derivedByDynamic()) {
        if (!cls && getScope()->isFirstPass()) {
          ConstructPtr self = shared_from_this();
          Compiler::Error(Compiler::UnknownClass, self);
        }
        ret = Type::Variant;
      }
    }
    if (!ret) {
      ret = Type::CreateObjectType(m_type);
    }
  }
  // we still want the above to run, so to record errors and infer defaults
  if (m_ref && forInference) {
    ret = Type::Variant;
  }

  return ret;
}

TypePtr ParameterExpression::inferTypes(AnalysisResultPtr ar, TypePtr type,
                                        bool coerce) {
  ASSERT(type->is(Type::KindOfSome) || type->is(Type::KindOfAny));
  TypePtr ret = getTypeSpec(ar, true);

  VariableTablePtr variables = getScope()->getVariables();
  // Functions that can be called dynamically have to have
  // variant parameters, even if they have a type hint
  if (getFunctionScope()->isDynamic() ||
      getFunctionScope()->isRedeclaring() ||
      getFunctionScope()->isVirtual()) {
    if (!Option::HardTypeHints || !ret->isExactType()) {
      variables->forceVariant(ar, m_name, VariableTable::AnyVars);
      ret = Type::Variant;
    }
  }

  if (m_defaultValue && !m_ref) {
    ret = m_defaultValue->inferAndCheck(ar, ret, false);
  }

  // parameters are like variables, but we need to remember these are
  // parameters so when variable table is generated, they are not generated
  // as declared variables.
  if (getScope()->isFirstPass()) {
    ret = variables->add(m_name, ret, false, ar,
                         shared_from_this(), ModifierExpressionPtr());
  } else {
    int p;
    ret = variables->checkVariable(m_name, ret, true, ar,
                                   shared_from_this(), p);
    if (ret->is(Type::KindOfSome)) {
      // This is probably too conservative. The problem is that
      // a function never called will have parameter types of Any.
      // Functions that it calls won't be able to accept variant unless
      // it is forced here.
      variables->forceVariant(ar, m_name, VariableTable::AnyVars);
      ret = Type::Variant;
    }
  }
  return ret;
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void ParameterExpression::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  if (!m_type.empty()) cg_printf("%s ", m_originalType.c_str());
  if (m_ref) cg_printf("&");
  cg_printf("$%s", m_name.c_str());
  if (m_defaultValue) {
    cg_printf(" = ");
    m_defaultValue->outputPHP(cg, ar);
  }
}

void ParameterExpression::outputCPPImpl(CodeGenerator &cg,
                                        AnalysisResultPtr ar) {
  FunctionScopePtr func = getFunctionScope();
  VariableTablePtr variables = func->getVariables();
  Symbol *sym = variables->getSymbol(m_name);
  assert(sym && sym->isParameter());

  CodeGenerator::Context context = cg.getContext();
  bool typedWrapper = (context == CodeGenerator::CppTypedParamsWrapperImpl ||
                       context == CodeGenerator::CppTypedParamsWrapperDecl);

  TypePtr paramType =
    typedWrapper && func->getParamTypeSpec(sym->getParameterIndex()) ?
    Type::Variant : func->getParamType(sym->getParameterIndex());

  bool isCVarRef = false;
  if (cg.getContext() == CodeGenerator::CppStaticMethodWrapper ||
      typedWrapper ||
      (!variables->isLvalParam(m_name) &&
       !variables->getAttribute(VariableTable::ContainsDynamicVariable) &&
       !variables->getAttribute(VariableTable::ContainsExtract) &&
       !m_ref)) {
    if (paramType->is(Type::KindOfVariant) ||
        paramType->is(Type::KindOfSome)) {
      cg_printf("CVarRef");
      isCVarRef = true;
    }
    else if (paramType->is(Type::KindOfArray)) cg_printf("CArrRef");
    else if (paramType->is(Type::KindOfString)) cg_printf("CStrRef");
    else paramType->outputCPPDecl(cg, ar, getScope());
  } else {
    paramType->outputCPPDecl(cg, ar, getScope());
  }

  cg_printf(" %s%s", Option::VariablePrefix, m_name.c_str());
  if (m_defaultValue && sym->getParameterIndex() >= func->getMinParamCount()) {
    bool comment = context == CodeGenerator::CppTypedParamsWrapperImpl ||
      context == CodeGenerator::CppImplementation ||
      (context == CodeGenerator::CppDeclaration && func->isInlined());
    if (comment) {
      cg_printf(" // ");
    }
    cg_printf(" = ");
    ConstantExpressionPtr con =
      dynamic_pointer_cast<ConstantExpression>(m_defaultValue);
    if (isCVarRef && con && con->isNull()) {
      cg_printf("null_variant");
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
      cg_printf("\n");
    }
  }
}
