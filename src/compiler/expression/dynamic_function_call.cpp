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

#include <compiler/expression/dynamic_function_call.h>
#include <compiler/analysis/code_error.h>
#include <compiler/expression/expression_list.h>
#include <compiler/expression/scalar_expression.h>
#include <compiler/analysis/function_scope.h>
#include <compiler/analysis/class_scope.h>
#include <util/util.h>
#include <compiler/option.h>
#include <compiler/analysis/variable_table.h>

using namespace HPHP;
using namespace std;
using namespace boost;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

DynamicFunctionCall::DynamicFunctionCall
(EXPRESSION_CONSTRUCTOR_PARAMETERS,
 ExpressionPtr name, ExpressionListPtr params, ExpressionPtr cls)
  : FunctionCall(EXPRESSION_CONSTRUCTOR_PARAMETER_VALUES,
                 name, "", params, cls) {
}

ExpressionPtr DynamicFunctionCall::clone() {
  DynamicFunctionCallPtr exp(new DynamicFunctionCall(*this));
  FunctionCall::deepCopy(exp);
  return exp;
}

///////////////////////////////////////////////////////////////////////////////
// parser functions

///////////////////////////////////////////////////////////////////////////////
// static analysis functions

void DynamicFunctionCall::analyzeProgram(AnalysisResultPtr ar) {
  if (m_class) {
    m_class->analyzeProgram(ar);
  } else {
    addUserClass(ar, m_className);
  }
  m_nameExp->analyzeProgram(ar);
  if (m_params) {
    m_params->markParams(false);
    m_params->analyzeProgram(ar);
  }
}

TypePtr DynamicFunctionCall::inferTypes(AnalysisResultPtr ar, TypePtr type,
                                        bool coerce) {
  reset();
  ConstructPtr self = shared_from_this();
  if (!m_className.empty()) {
    ClassScopePtr cls = resolveClass(getScope());
    if (!cls) {
      if (isRedeclared()) {
        getScope()->getVariables()->
          setAttribute(VariableTable::NeedGlobalPointer);
      } else if (getScope()->isFirstPass()) {
        Compiler::Error(Compiler::UnknownClass, self);
      }
    } else {
      m_classScope = cls;
    }
  }

  ar->containsDynamicFunctionCall();

  if (m_class) {
    m_class->inferAndCheck(ar, Type::Any, false);
  }
  m_nameExp->inferAndCheck(ar, Type::String, false);
  if (m_params) {
    for (int i = 0; i < m_params->getCount(); i++) {
      (*m_params)[i]->inferAndCheck(ar, Type::Variant, true);
    }
  }
  return Type::Variant;
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions
void DynamicFunctionCall::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  outputLineMap(cg, ar);

  if (m_class || !m_className.empty()) {
    StaticClassName::outputPHP(cg, ar);
    cg_printf("::");
    m_nameExp->outputPHP(cg, ar);
  } else {
    const char *prefix = Option::IdPrefix.c_str();
    if (cg.getOutput() == CodeGenerator::TrimmedPHP &&
        cg.usingStream(CodeGenerator::PrimaryStream) &&
        !m_nameExp->is(Expression::KindOfScalarExpression)) {
      cg_printf("${%sdynamic_load($%stmp = (", prefix, prefix);
      m_nameExp->outputPHP(cg, ar);
      cg_printf("), '%stmp'", prefix);
      cg_printf(")}");
    } else {
      m_nameExp->outputPHP(cg, ar);
    }
  }

  cg_printf("(");
  if (m_params) m_params->outputPHP(cg, ar);
  cg_printf(")");
}

bool DynamicFunctionCall::preOutputCPP(CodeGenerator &cg, AnalysisResultPtr ar,
    int state) {
  bool nonStatic = !m_class && m_className.empty();
  if (!nonStatic && !m_class && !m_classScope && !isRedeclared())
    return FunctionCall::preOutputCPP(cg, ar, state);
  // Short circuit out if inExpression() returns false
  if (!cg.inExpression()) return true;

  if (m_class) {
    m_class->preOutputCPP(cg, ar, state);
  }

  m_nameExp->preOutputCPP(cg, ar, state);

  cg.wrapExpressionBegin();
  m_ciTemp = cg.createNewLocalId(shared_from_this());
  bool lsb = false;

  if (!m_classScope && !m_className.empty() && m_cppTemp.empty() &&
      !isSelf() && ! isParent() && !isStatic()) {
    // Create a temporary to hold the class name, in case it is not a
    // StaticString.
    m_clsNameTemp = cg.createNewLocalId(shared_from_this());
    cg_printf("CStrRef clsName%d(", m_clsNameTemp);
    cg_printString(m_origClassName, ar, shared_from_this());
    cg_printf(");\n");
  }

  if (nonStatic) {
    cg_printf("const CallInfo *cit%d;\n", m_ciTemp);
    cg_printf("void *vt%d;\n", m_ciTemp);
    cg_printf("get_call_info_or_fail(cit%d, vt%d, ", m_ciTemp, m_ciTemp);
  } else {
    cg_printf("MethodCallPackage mcp%d;\n", m_ciTemp);
    if (m_class) {
      if (m_class->is(KindOfScalarExpression)) {
        ASSERT(strcasecmp(dynamic_pointer_cast<ScalarExpression>(m_class)->
                          getString().c_str(), "static") == 0);
        cg_printf("mcp%d.staticMethodCall(", m_ciTemp);
        cg_printf("\"static\"");
        lsb = true;
      } else {
        cg_printf("mcp%d.dynamicNamedCall(", m_ciTemp);
        m_class->outputCPP(cg, ar);
      }
      cg_printf(", ");
    } else if (m_classScope) {
      cg_printf("mcp%d.staticMethodCall(\"%s\", ", m_ciTemp,
                m_classScope->getId(cg).c_str());
    } else {
      cg_printf("mcp%d.staticMethodCall(\"%s\", ", m_ciTemp,
                m_className.c_str());
    }
  }

  if (m_nameExp->is(Expression::KindOfSimpleVariable)) {
    m_nameExp->outputCPP(cg, ar);
  } else {
    cg_printf("(");
    m_nameExp->outputCPP(cg, ar);
    cg_printf(")");
  }

  if (!nonStatic) {
    cg_printf(");\n");
    if (lsb) cg_printf("mcp%d.lateStaticBind(info);\n");
    cg_printf("const CallInfo *&cit%d = mcp%d.ci;\n", m_ciTemp, m_ciTemp);
    if (m_classScope) {
      cg_printf("%s%s::%sget_call_info(mcp%d",
                Option::ClassPrefix, m_classScope->getId(cg).c_str(),
                Option::ObjectStaticPrefix, m_ciTemp, m_className.c_str());
    } else if (isRedeclared()) {
      cg_printf("g->%s%s->%sget_call_info(mcp%d",
                Option::ClassStaticsObjectPrefix, m_className.c_str(),
                Option::ObjectStaticPrefix, m_ciTemp, m_className.c_str());
    }
  }
  if (nonStatic || !m_class) {
    cg_printf(");\n");
  }
  if (m_params && m_params->getCount() > 0) {
    cg.pushCallInfo(m_ciTemp);
    m_params->preOutputCPP(cg, ar, state);
    cg.popCallInfo();
  }

  cg.pushCallInfo(m_ciTemp);
  preOutputStash(cg, ar, state);
  cg.popCallInfo();
  if (!(state & FixOrder)) {
    cg_printf("id(%s);\n", cppTemp().c_str());
  }
  return true;
}

void DynamicFunctionCall::outputCPPImpl(CodeGenerator &cg,
                                        AnalysisResultPtr ar) {
  if (m_class || !m_className.empty()) {
    if (m_class || m_classScope || isRedeclared()) {
      cg.printf("(cit%d->getMeth())(mcp%d, ", m_ciTemp, m_ciTemp);
      if (m_params && m_params->getCount() > 0) {
        cg.pushCallInfo(m_ciTemp);
        FunctionScopePtr dummy;
        FunctionScope::OutputCPPArguments(m_params, dummy, cg, ar, -1, false);
        cg.popCallInfo();
      } else {
        cg_printf("Array()");
      }
      cg_printf(")");
      return;
    } else {
      cg_printf("throw_fatal(\"unknown class %s\")", m_className.c_str());
      cg_printf(")");
      return;
    }
  } else {
    //cg.printf("invoke(");
    cg_printf("(cit%d->getFunc())(vt%d, ", m_ciTemp, m_ciTemp);
    if (m_params && m_params->getCount() > 0) {
      cg.pushCallInfo(m_ciTemp);
      FunctionScopePtr dummy;
      FunctionScope::OutputCPPArguments(m_params, dummy, cg, ar, -1, false);
      cg.popCallInfo();
    } else {
      cg_printf("Array()");
    }
    cg_printf(")");
    return;
  }
  if (m_nameExp->is(Expression::KindOfSimpleVariable)) {
    m_nameExp->outputCPP(cg, ar);
  } else {
    cg_printf("(");
    m_nameExp->outputCPP(cg, ar);
    cg_printf(")");
  }
  cg_printf(", ");
  if (m_params && m_params->getCount() > 0) {
    FunctionScopePtr dummy;
    FunctionScope::OutputCPPArguments(m_params, dummy, cg, ar, -1, false);
  } else {
    cg_printf("Array()");
  }
  if (m_class) {
    cg_printf(")");
  } else {
    cg_printf(", -1)");
  }
}
