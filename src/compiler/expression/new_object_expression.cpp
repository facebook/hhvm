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

#include <compiler/expression/new_object_expression.h>
#include <compiler/expression/scalar_expression.h>
#include <compiler/expression/expression_list.h>
#include <compiler/analysis/code_error.h>
#include <compiler/analysis/class_scope.h>
#include <compiler/analysis/function_scope.h>
#include <compiler/option.h>
#include <compiler/analysis/variable_table.h>

using namespace HPHP;
using namespace std;
using namespace boost;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

NewObjectExpression::NewObjectExpression
(EXPRESSION_CONSTRUCTOR_PARAMETERS,
 ExpressionPtr variable, ExpressionListPtr params)
  : FunctionCall(EXPRESSION_CONSTRUCTOR_PARAMETER_VALUES,
                 variable, "", params, variable),
    m_dynamic(false) {
  /*
    StaticClassName takes care of parent & self properly, so
    we pass in variable as the class name.
    But NewObjectExpression is written to use the class as the
    function name, so clear it here, to take care of the dynamic
    case.
    Also set m_noStatic, to prevent errors in code gen due to
    m_className being set.
  */
  m_class.reset();
  m_noStatic = true;
}

ExpressionPtr NewObjectExpression::clone() {
  NewObjectExpressionPtr exp(new NewObjectExpression(*this));
  FunctionCall::deepCopy(exp);
  return exp;
}

///////////////////////////////////////////////////////////////////////////////
// parser functions

///////////////////////////////////////////////////////////////////////////////
// static analysis functions

void NewObjectExpression::analyzeProgram(AnalysisResultPtr ar) {
  FunctionCall::analyzeProgram(ar);

  if (ar->getPhase() == AnalysisResult::AnalyzeAll ||
      ar->getPhase() == AnalysisResult::AnalyzeFinal) {
    FunctionScopePtr func;
    if (!m_name.empty()) {
      addUserClass(ar, m_name);
      if (ClassScopePtr cls = resolveClass()) {
        m_name = m_className;
        func = cls->findConstructor(ar, true);
      }
    }

    if (m_params) {
      markRefParams(func, "", canInvokeFewArgs());
    }
  }
}

TypePtr NewObjectExpression::inferTypes(AnalysisResultPtr ar, TypePtr type,
                                        bool coerce) {
  reset();
  m_classScope.reset();
  m_funcScope.reset();
  ConstructPtr self = shared_from_this();
  if (!m_name.empty()) {
    ClassScopePtr cls = resolveClass();
    m_name = m_className;

    if (!cls) {
      if (isRedeclared()) {
        getScope()->getVariables()->
          setAttribute(VariableTable::NeedGlobalPointer);
      } else if (getScope()->isFirstPass()) {
        Compiler::Error(Compiler::UnknownClass, self);
      }
      if (m_params) m_params->inferAndCheck(ar, Type::Any, false);
      return Type::Object;
    }
    if (cls->isVolatile() && !isPresent()) {
      getScope()->getVariables()->
        setAttribute(VariableTable::NeedGlobalPointer);
    }
    m_dynamic = cls->derivesFromRedeclaring();
    bool valid = true;
    FunctionScopePtr func = cls->findConstructor(ar, true);
    if (!func) {
      if (m_params) {
        if (!m_dynamic && m_params->getCount()) {
          if (getScope()->isFirstPass()) {
            Compiler::Error(Compiler::BadConstructorCall, self);
          }
          m_params->setOutputCount(0);
        }
        m_params->inferAndCheck(ar, Type::Some, false);
      }
    } else {
      m_extraArg = func->inferParamTypes(ar, self, m_params,
                                         valid);
      m_variableArgument = func->isVariableArgument();
    }
    if (valid) {
      m_classScope = cls;
      m_funcScope = func;
    }
    if (!valid || m_dynamic) {
      m_implementedType = Type::Object;
    } else {
      m_implementedType.reset();
    }
    return Type::CreateObjectType(m_name);
  } else {
    ar->containsDynamicClass();
    if (m_params) {
      m_params->markParams(canInvokeFewArgs());
    }
  }

  m_implementedType.reset();
  m_nameExp->inferAndCheck(ar, Type::String, false);
  if (m_params) m_params->inferAndCheck(ar, Type::Any, false);
  return Type::Object;
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void NewObjectExpression::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  outputLineMap(cg, ar);

  cg_printf("new ");
  m_nameExp->outputPHP(cg, ar);
  cg_printf("(");
  if (m_params) m_params->outputPHP(cg, ar);
  cg_printf(")");
}

void NewObjectExpression::preOutputStash(CodeGenerator &cg,
                                         AnalysisResultPtr ar, int state) {
  if (!m_receiverTemp.empty()) {
    TypePtr e = getExpectedType();
    if (!e || !Type::IsCastNeeded(ar, getActualType(), e)) {
      bool unused = isUnused();
      setUnused(true);
      outputCPPImpl(cg, ar);
      setUnused(unused);
      setCPPTemp(m_receiverTemp);
      cg_printf(";\n");
      return;
    }
  }
  optimizeArgArray(ar);
  FunctionCall::preOutputStash(cg, ar, state);
}

void NewObjectExpression::outputCPPImpl(CodeGenerator &cg,
                                        AnalysisResultPtr ar) {
  string &cname = isSelf() || isParent() ? m_name : m_origName;
  bool outsideClass = !isPresent();
  if (!m_name.empty() && m_classScope && !m_dynamic) {
    ClassScopePtr cls = m_classScope;
    if (m_receiverTemp.empty()) {
      if (outsideClass) {
        cls->outputVolatileCheckBegin(cg, ar, getScope(), cname);
      }
      cg_printf("%s%s((NEWOBJ(%s%s)())->create(",
                Option::SmartPtrPrefix, cls->getId(cg).c_str(),
                Option::ClassPrefix, cls->getId(cg).c_str());
    } else {
      cg_printf("(%s->create(", m_receiverTemp.c_str());
    }

    FunctionScope::OutputCPPArguments(m_params, m_funcScope, cg, ar, m_extraArg,
                                      m_variableArgument, m_argArrayId,
                                      m_argArrayHash, m_argArrayIndex);
    if (m_receiverTemp.empty()) {
      cg_printf("))");
      if (outsideClass) {
        cls->outputVolatileCheckEnd(cg);
      }
    } else {
      cg_printf(")");
      if (!isUnused()) {
        cg_printf(", %s", m_receiverTemp.c_str());
      }
      cg_printf(")");
    }
  } else {
    bool wrap = false;
    wrap = m_actualType && m_actualType->is(Type::KindOfVariant) &&
      !m_expectedType && !m_implementedType;
    if (wrap) {
      cg_printf("((Variant)");
    }
    cg_printf("id(obj%d)", m_objectTemp);
    if (wrap) {
      cg_printf(")");
    }
  }
}

bool NewObjectExpression::preOutputCPP(CodeGenerator &cg, AnalysisResultPtr ar,
                                       int state) {
  string &cname = isSelf() || isParent() ? m_name : m_origName;
  if (m_name.empty() || !m_classScope || m_dynamic) {
    // Short circuit out if inExpression() returns false
    if (!cg.inExpression()) return true;

    if (m_nameExp) m_nameExp->preOutputCPP(cg, ar, 0);
    cg.wrapExpressionBegin();
    m_ciTemp = cg.createNewLocalId(shared_from_this());
    m_objectTemp = cg.createNewLocalId(shared_from_this());
    cg_printf("Object obj%d(", m_objectTemp);
    if (isRedeclared()) {
      bool outsideClass = !isPresent();
      if (outsideClass) {
        ClassScope::OutputVolatileCheckBegin(cg, ar, getScope(), cname);
      }
      cg_printf("g->%s%s->createOnly()", Option::ClassStaticsObjectPrefix,
                m_name.c_str());
      if (outsideClass) {
        ClassScope::OutputVolatileCheckEnd(cg);
      }
    } else {
      cg_printf("create_object_only(");
      if (isStatic()) {
        cg_printf("FrameInjection::GetStaticClassName(fi.getThreadInfo())");
      } else if (!cname.empty()) {
        cg_printf("\"%s\"", cname.c_str());
      } else if (m_nameExp->is(Expression::KindOfSimpleVariable)) {
        m_nameExp->outputCPP(cg, ar);
      } else {
        cg_printf("(");
        m_nameExp->outputCPP(cg, ar);
        cg_printf(")");
      }
      cg_printf(")");
    }
    cg_printf(");\n");
    cg_printf("MethodCallPackage mcp%d;\n", m_ciTemp);
    cg_printf("mcp%d.construct(obj%d);\n", m_ciTemp, m_objectTemp);
    cg_printf("const CallInfo *cit%d = mcp%d.ci;\n", m_ciTemp, m_ciTemp);

    int pcount = m_params ? m_params->getCount() : 0;
    if (pcount > 0) {
      cg.pushCallInfo(m_ciTemp);
      m_params->preOutputCPP(cg, ar, 0);
      cg.popCallInfo();
    }

    cg_printf("(cit%d->", m_ciTemp);
    outputDynamicCall(cg, ar, true);
    cg_printf(";\n");

    if (state & FixOrder) {
      cg.pushCallInfo(m_ciTemp);
      preOutputStash(cg, ar, state);
      cg.popCallInfo();
    }

    return true;
  } else {
    bool tempRcvr = true;

    bool paramEffect = false;
    if (m_params && m_params->getCount() > 0) {
      for (int i = m_params->getCount(); i--; ) {
        if (!(*m_params)[i]->isScalar()) {
          paramEffect = true;
          break;
        }
      }
    }

    if (!paramEffect) {
      tempRcvr = false;
    }

    if (tempRcvr && cg.inExpression()) {
      bool outsideClass = !isPresent();
      cg.wrapExpressionBegin();
      m_receiverTemp = genCPPTemp(cg, ar);
      cg_printf("%s%s %s = ",
                Option::SmartPtrPrefix, m_classScope->getId(cg).c_str(),
                m_receiverTemp.c_str());
      if (outsideClass) {
        m_classScope->outputVolatileCheckBegin(cg, ar, getScope(), cname);
      }
      cg_printf("NEWOBJ(%s%s)()",
                Option::ClassPrefix, m_classScope->getId(cg).c_str());
      if (outsideClass) {
        m_classScope->outputVolatileCheckEnd(cg);
      }
      cg_printf(";\n");
    }

    bool tempParams = FunctionCall::preOutputCPP(cg, ar, state);
    return tempRcvr || tempParams;
  }
}
