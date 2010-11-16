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
    StaticClassName(classExp), m_nameExp(nameExp), m_params(params),
    m_valid(false), m_validClass(false),
    m_extraArg(0), m_variableArgument(false), m_voidReturn(false),
    m_voidWrapper(false), m_allowVoidReturn(false), m_redeclared(false),
    m_redeclaredClass(false), m_derivedFromRedeclaring(false),
    m_noStatic(false), m_argArrayId(-1), m_argArrayHash(-1),
    m_argArrayIndex(-1) {

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
  m_clsNameTemp = -1;
}

void FunctionCall::reset() {
  m_valid = false;
  m_validClass = false;
  m_extraArg = 0;
  m_variableArgument = false;
  m_voidWrapper = false;
}

bool FunctionCall::isTemporary() const {
  return m_funcScope && !m_funcScope->isRefReturn();
}

void FunctionCall::deepCopy(FunctionCallPtr exp) {
  Expression::deepCopy(exp);
  exp->m_class = Clone(m_class);
  exp->m_params = Clone(m_params);
  exp->m_nameExp = Clone(m_nameExp);
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

void FunctionCall::markRefParams(FunctionScopePtr func,
                                 const std::string &name,
                                 bool canInvokeFewArgs) {
  ExpressionList &params = *m_params;
  if (func) {
    int mpc = func->getMaxParamCount();
    for (int i = params.getCount(); i--; ) {
      ExpressionPtr p = params[i];
      if (i < mpc ? func->isRefParam(i) :
          func->isReferenceVariableArgument()) {
        p->setContext(Expression::RefValue);
      }
    }
  } else if (!m_name.empty()) {
    FunctionScope::RefParamInfoPtr info =
      FunctionScope::GetRefParamInfo(m_name);
    if (info) {
      for (int i = params.getCount(); i--; ) {
        if (info->isRefParam(i)) {
          m_params->markParam(i, canInvokeFewArgs);
        }
      }
    }
    // If we cannot find information of the so-named function, it might not
    // exist, or it might go through __call(), either of which cannot have
    // reference parameters.
  } else {
    for (int i = params.getCount(); i--; ) {
      m_params->markParam(i, canInvokeFewArgs);
    }
  }
}

ExpressionPtr FunctionCall::preOptimize(AnalysisResultPtr ar) {
  if (m_class) updateClassName();
  return ExpressionPtr();
}

ExpressionPtr FunctionCall::postOptimize(AnalysisResultPtr ar) {
  if (m_class) updateClassName();
  optimizeArgArray(ar);
  return ExpressionPtr();
}

///////////////////////////////////////////////////////////////////////////////

TypePtr FunctionCall::checkParamsAndReturn(AnalysisResultPtr ar,
                                           TypePtr type, bool coerce,
                                           FunctionScopePtr func,
                                           bool arrayParams) {
  ConstructPtr self = shared_from_this();
  ar->getDependencyGraph()->add(DependencyGraph::KindOfFunctionCall,
                                ar->getName(), getText(),
                                self, func->getFullName(), func->getStmt());
  TypePtr frt = func->getReturnType();
  if (!frt) {
    m_voidReturn = true;
    setActualType(TypePtr());
    if (!type->is(Type::KindOfAny)) {
      if (!m_allowVoidReturn && !func->isFirstPass() && !func->isAbstract()) {
        Compiler::Error(Compiler::UseVoidReturn, self);
      }
      m_voidWrapper = true;
    }
  } else {
    m_voidReturn = false;
    m_voidWrapper = false;
    type = checkTypesImpl(ar, type, frt, coerce);
  }
  if (arrayParams) {
    m_extraArg = 0;
    (*m_params)[0]->inferAndCheck(ar, Type::Array, false);
  } else {
    m_extraArg = func->inferParamTypes(ar, self, m_params, m_valid);
  }
  m_variableArgument = func->isVariableArgument();
  if (m_valid) {
    m_implementedType.reset();
  } else {
    m_implementedType = Type::Variant;
  }

  return type;
}

bool FunctionCall::preOutputCPP(CodeGenerator &cg, AnalysisResultPtr ar,
                                int state) {
  if (isUnused() ||
      m_className.empty() ||
      m_origClassName == "self" ||
      m_origClassName == "parent") {
    return Expression::preOutputCPP(cg, ar, state);
  }

  if (!ar->inExpression()) {
    return true;
  }
  Expression::preOutputCPP(cg, ar, state & ~FixOrder);
  ar->wrapExpressionBegin(cg);
  if (m_classScope) {
    string className = m_classScope->getId(cg);
    cg_printf("FrameInjection::SetStaticClassName(info, "
              "%s%s::s_class_name);\n",
              Option::ClassPrefix, className.c_str());
  } else {
    m_clsNameTemp = cg.createNewId(shared_from_this());
    cg_printf("CStrRef clsName%d(", m_clsNameTemp);
    cg_printString(m_origClassName, ar, shared_from_this());
    cg_printf(");\n");
    cg_printf("FrameInjection::SetStaticClassName(info, clsName%d);\n",
              m_clsNameTemp);
  }
  m_noStatic = true;
  preOutputStash(cg, ar, FixOrder);
  m_noStatic = false;
  cg_printf("FrameInjection::ResetStaticClassName(info);\n");

  if (!(state & FixOrder)) {
    cg_printf("id(%s);\n", cppTemp().c_str());
  }

  return true;
}

void FunctionCall::outputCPP(CodeGenerator &cg, AnalysisResultPtr ar) {
  bool staticClassName = false;
  if (!m_noStatic && !m_className.empty() && m_cppTemp.empty() &&
      m_origClassName != "self" && m_origClassName != "parent" &&
      m_origClassName != "static") {
    if (!m_className.empty()) {
      cg_printf("STATIC_CLASS_NAME_CALL(");
      if (m_classScope) {
        string className = m_classScope->getId(cg);
        cg_printf("%s%s::s_class_name, ",
                  Option::ClassPrefix, className.c_str());
      } else {
        ASSERT(m_clsNameTemp >= 0);
        cg_printf("clsName%d, ", m_clsNameTemp);
      }
    } else {
      cg_printf("STATIC_CLASS_INVOKE_CALL(mcp%d.getClassName(), ",
          m_ciTemp);
    }
    if (m_voidReturn) m_voidWrapper = true;
    staticClassName = true;
  }

  if (m_voidReturn) clearContext(RefValue);
  bool wrap = m_voidWrapper && m_cppTemp.empty() && !isUnused();
  if (wrap) {
    cg_printf("(");
  }
  Expression::outputCPP(cg, ar);
  if (wrap) {
    cg_printf(", null)");
  }

  if (staticClassName) {
    cg_printf(")");
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
      ExpressionListPtr(new ExpressionList(getScope(), getLocation(),
                                           Expression::KindOfExpressionList));
    for (int i = iMax; i < paramCount; i++) {
      ExpressionPtr param = (*m_params)[i];
      argArrayPairs->addElement(
        ArrayPairExpressionPtr(new ArrayPairExpression(
                                 getScope(), param->getLocation(),
                                 Expression::KindOfArrayPairExpression,
                                 ExpressionPtr(), param, false)));
    }
    string text;
    m_argArrayId =
      ar->registerScalarArray(getFileScope(), argArrayPairs,
                              m_argArrayHash, m_argArrayIndex, text);
  }
}
