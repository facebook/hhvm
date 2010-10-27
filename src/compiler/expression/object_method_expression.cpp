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

#include <compiler/expression/object_method_expression.h>
#include <compiler/expression/scalar_expression.h>
#include <compiler/expression/expression_list.h>
#include <compiler/analysis/code_error.h>
#include <compiler/analysis/class_scope.h>
#include <compiler/analysis/function_scope.h>
#include <compiler/analysis/dependency_graph.h>
#include <compiler/statement/statement.h>
#include <util/util.h>
#include <util/hash.h>
#include <compiler/option.h>
#include <compiler/expression/simple_variable.h>
#include <compiler/analysis/variable_table.h>
#include <compiler/parser/parser.h>

using namespace HPHP;
using namespace std;
using namespace boost;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

ObjectMethodExpression::ObjectMethodExpression
(EXPRESSION_CONSTRUCTOR_PARAMETERS,
 ExpressionPtr object, ExpressionPtr method, ExpressionListPtr params)
  : FunctionCall(EXPRESSION_CONSTRUCTOR_PARAMETER_VALUES,
                 method, "", params, ExpressionPtr()), m_object(object),
    m_invokeFewArgsDecision(true), m_bindClass(true) {
  m_object->setContext(Expression::ObjectContext);
  m_object->clearContext(Expression::LValue);
}

ExpressionPtr ObjectMethodExpression::clone() {
  ObjectMethodExpressionPtr exp(new ObjectMethodExpression(*this));
  FunctionCall::deepCopy(exp);
  exp->m_object = Clone(m_object);
  return exp;
}

ClassScopePtr ObjectMethodExpression::resolveClass(AnalysisResultPtr ar,
                                                   string &name) {
  ClassScopePtr cls = ar->findClass(name, AnalysisResult::MethodName);
  if (cls) {
    addUserClass(ar, cls->getName());
    return cls;
  }
  string construct("__construct");
  cls = ar->findClass(construct, AnalysisResult::MethodName);
  if (cls && name == cls->getName()) {
    name = "__construct";
    cls->setAttribute(ClassScope::ClassNameConstructor);
    return cls;
  }
  return ClassScopePtr();
}

///////////////////////////////////////////////////////////////////////////////
// parser functions

///////////////////////////////////////////////////////////////////////////////
// static analysis functions

void ObjectMethodExpression::analyzeProgram(AnalysisResultPtr ar) {
  Expression::analyzeProgram(ar);

  m_params->analyzeProgram(ar);
  m_object->analyzeProgram(ar);
  m_nameExp->analyzeProgram(ar);

  if (ar->getPhase() == AnalysisResult::AnalyzeAll) {
    FunctionScopePtr func = m_funcScope;
    if (!func && m_object->isThis() && !m_name.empty()) {
      ClassScopePtr cls = getClassScope();
      if (cls) {
        m_classScope = cls;
        m_funcScope = func = cls->findFunction(ar, m_name, true, true);
        if (!func) {
          cls->addMissingMethod(m_name);
        }
      }
    }

    markRefParams(func, m_name, canInvokeFewArgs());
  }
}

bool ObjectMethodExpression::canInvokeFewArgs() {
  // We can always change out minds about saying yes, but once we say
  // no, it sticks.
  if (m_invokeFewArgsDecision && m_params &&
      m_params->getCount() > Option::InvokeFewArgsCount) {
    m_invokeFewArgsDecision = false;
  }
  return m_invokeFewArgsDecision;
}

ConstructPtr ObjectMethodExpression::getNthKid(int n) const {
  switch (n) {
    case 0:
      return m_object;
    default:
      return FunctionCall::getNthKid(n-1);
  }
  ASSERT(false);
}

int ObjectMethodExpression::getKidCount() const {
  return FunctionCall::getKidCount() + 1;
}

void ObjectMethodExpression::setNthKid(int n, ConstructPtr cp) {
  switch (n) {
    case 0:
      m_object = boost::dynamic_pointer_cast<Expression>(cp);
      break;
    default:
      FunctionCall::setNthKid(n-1, cp);
      break;
  }
}

ExpressionPtr ObjectMethodExpression::preOptimize(AnalysisResultPtr ar) {
  ar->preOptimize(m_object);
  return FunctionCall::preOptimize(ar);
}

ExpressionPtr ObjectMethodExpression::postOptimize(AnalysisResultPtr ar) {
  ar->postOptimize(m_object);
  return FunctionCall::postOptimize(ar);
}

TypePtr ObjectMethodExpression::inferTypes(AnalysisResultPtr ar,
                                           TypePtr type, bool coerce) {
  ASSERT(false);
  return TypePtr();
}

void ObjectMethodExpression::setInvokeParams(AnalysisResultPtr ar) {
  FunctionScope::RefParamInfoPtr info = FunctionScope::GetRefParamInfo(m_name);
  if (info || m_name.empty()) {
    for (int i = m_params->getCount(); i--; ) {
      if (!info || info->isRefParam(i)) {
        m_params->markParam(i, canInvokeFewArgs());
      }
    }
  }
  // If we cannot find information of the so-named function, it might not
  // exist, or it might go through __call(), either of which cannot have
  // reference parameters.
  for (int i = 0; i < m_params->getCount(); i++) {
    (*m_params)[i]->inferAndCheck(ar, Type::Variant, false);
  }
  m_params->resetOutputCount();
}

TypePtr ObjectMethodExpression::inferAndCheck(AnalysisResultPtr ar,
                                              TypePtr type, bool coerce) {
  reset();

  ConstructPtr self = shared_from_this();
  TypePtr objectType = m_object->inferAndCheck(ar, Type::Object, false);
  m_valid = true;
  m_bindClass = true;

  if (m_name.empty()) {
    m_nameExp->inferAndCheck(ar, Type::String, false);
    setInvokeParams(ar);
    // we have to use a variant to hold dynamic value
    return checkTypesImpl(ar, type, Type::Variant, coerce);
  }

  ClassScopePtr cls = m_classScope;
  if (objectType && !objectType->getName().empty()) {
    cls = ar->findExactClass(shared_from_this(), objectType->getName());
  }

  if (!cls) {
    if (ar->isFirstPass()) {
      // call resolveClass to mark functions as dynamic
      // but we cant do anything else with the result.
      resolveClass(ar, m_name);
      if (!ar->classMemberExists(m_name, AnalysisResult::MethodName)) {
        Compiler::Error(Compiler::UnknownObjectMethod, self);
      }
    }

    m_classScope.reset();
    m_funcScope.reset();

    setInvokeParams(ar);
    return checkTypesImpl(ar, type, Type::Variant, coerce);
  }

  if (m_classScope != cls) {
    m_classScope = cls;
    m_funcScope.reset();
  }

  FunctionScopePtr func = m_funcScope;
  if (!func) {
    func = cls->findFunction(ar, m_name, true, true);
    if (!func) {
      if (!cls->hasAttribute(ClassScope::HasUnknownMethodHandler, ar)) {
        if (ar->classMemberExists(m_name, AnalysisResult::MethodName)) {
          setDynamicByIdentifier(ar, m_name);
        } else {
          Compiler::Error(Compiler::UnknownObjectMethod, self);
        }
      }

      m_valid = false;
      setInvokeParams(ar);
      return checkTypesImpl(ar, type, Type::Variant, coerce);
    }
    m_funcScope = func;
  }

  bool valid = true;
  m_bindClass = func->isStatic();

  // use $this inside a static function
  if (m_object->isThis()) {
    FunctionScopePtr localfunc = getFunctionScope();
    if (localfunc->isStatic()) {
      if (ar->isFirstPass()) {
        Compiler::Error(Compiler::MissingObjectContext, self);
      }
      valid = false;
    }
  }

  // invoke() will return Variant
  if (!m_object->getType()->isSpecificObject() ||
      (func->isVirtual() && !func->isPerfectVirtual())) {
    valid = false;
  }

  if (!valid) {
    setInvokeParams(ar);
    checkTypesImpl(ar, type, Type::Variant, coerce);
    m_valid = false; // so we use invoke() syntax
    func->setDynamic();
    return m_actualType;
  }

  return checkParamsAndReturn(ar, type, coerce, func, false);
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void ObjectMethodExpression::outputPHP(CodeGenerator &cg,
                                       AnalysisResultPtr ar) {
  outputLineMap(cg, ar);

  m_object->outputPHP(cg, ar);
  cg_printf("->");
  if (m_nameExp->getKindOf() == Expression::KindOfScalarExpression) {
    m_nameExp->outputPHP(cg, ar);
  } else {
    cg_printf("{");
    m_nameExp->outputPHP(cg, ar);
    cg_printf("}");
  }
  cg_printf("(");
  m_params->outputPHP(cg, ar);
  cg_printf(")");
}

bool ObjectMethodExpression::directVariantProxy(AnalysisResultPtr ar) {
  TypePtr actualType = m_object->getActualType();
  if (actualType && actualType->is(Type::KindOfVariant) &&
      (!m_valid || m_name.empty() ||
       !m_object->getType()->isSpecificObject())) {
    if (m_object->is(KindOfSimpleVariable)) {
      SimpleVariablePtr var =
        dynamic_pointer_cast<SimpleVariable>(m_object);
      const std::string &name = var->getName();
      FunctionScopePtr func = getFunctionScope();
      VariableTablePtr variables = func->getVariables();
      if (!variables->isParameter(name) || variables->isLvalParam(name)) {
        return true;
      }
      if (variables->getAttribute(VariableTable::ContainsDynamicVariable) ||
          variables->getAttribute(VariableTable::ContainsExtract)) {
        return true;
      }
    } else {
      return true;
    }
  }
  return false;
}
void ObjectMethodExpression::outputCPPObject(CodeGenerator &cg,
    AnalysisResultPtr ar) {
  bool isThis = m_object->isThis();
  if (isThis && getFunctionScope()->isStatic()) {
    cg_printf("GET_THIS_ARROW()");
  }

  if (!isThis) {
    if (!m_object->getActualType() ||
        (directVariantProxy(ar) && !m_object->hasCPPTemp())) {
      TypePtr expectedType = m_object->getExpectedType();
      ASSERT(!expectedType || expectedType->is(Type::KindOfObject));
      // Clear m_expectedType to avoid type cast (toObject).
      m_object->setExpectedType(TypePtr());
      m_object->outputCPP(cg, ar);
      m_object->setExpectedType(expectedType);
    } else {
      m_object->outputCPP(cg, ar);
    }
  }
}
void ObjectMethodExpression::outputCPPObjectCall(CodeGenerator &cg,
    AnalysisResultPtr ar) {
  outputCPPObject(cg, ar);
  bool isThis = m_object->isThis();
  if (!isThis) {
    if (directVariantProxy(ar) && !m_object->hasCPPTemp()) {
      if (m_bindClass) {
        cg_printf(". BIND_CLASS_DOT ");
      } else {
        cg_printf(".");
      }
    } else {
      string objType;
      TypePtr type = m_object->getType();
      if (type->isSpecificObject() && !m_name.empty() && m_valid) {
        objType = type->getName();
        ClassScopePtr cls = ar->findClass(objType);
        objType = cls->getId(cg);
      } else {
        objType = "ObjectData";
      }
      if (m_bindClass) {
        cg_printf("-> BIND_CLASS_ARROW(%s) ", objType.c_str());
      } else {
        cg_printf("->");
      }
    }
  } else if (m_bindClass && m_classScope) {
    cg_printf(" BIND_CLASS_ARROW(%s) ", m_classScope->getId(cg).c_str());
  }
}

bool ObjectMethodExpression::preOutputCPP(CodeGenerator &cg,
    AnalysisResultPtr ar, int state) {
  if (!m_name.empty() && m_valid && m_object->getType()->isSpecificObject()) {
    return FunctionCall::preOutputCPP(cg, ar, state);
  }
  // Short circuit out if inExpression() returns false
  if (!ar->inExpression()) return true;
  m_ciTemp = cg.createNewId(shared_from_this());

  ar->wrapExpressionBegin(cg);
  bool isThis = m_object->isThis();
  if (!isThis) {
    m_object->preOutputCPP(cg, ar, state);
  }
  if (m_name.empty()) {
    m_nameExp->preOutputCPP(cg, ar, state);
  }
  const MethodSlot *ms = NULL;
  if (!m_name.empty()) {
    ConstructPtr self = shared_from_this();
    ms = ar->getOrAddMethodSlot(m_name, self);
  }
  cg_printf("MethodCallPackage mcp%d;\n", m_ciTemp);
  if (ms) {
    cg_printf("mcp%d.methodCallWithIndex((", m_ciTemp);
  } else {
    cg_printf("mcp%d.methodCall((", m_ciTemp);
  }
  if (isThis) {
    cg_printf("GET_THIS()");
  } else {
    outputCPPObject(cg, ar);
  }
  cg_printf("), ");
  if (!m_name.empty()) {
    uint64 hash = hash_string_i(m_name.c_str());
    cg_printString(m_origName, ar, shared_from_this());
    if (ms) {
      cg_printf(", %s", ms->runObjParam().c_str());
    }
    cg_printf(", 0x%016llXLL);\n", hash);
  } else {
    m_nameExp->outputCPP(cg, ar);
    cg_printf(", -1);\n");
  }
  cg_printf("const CallInfo *cit%d  __attribute__((__unused__)) ="
      " mcp%d.ci;\n", m_ciTemp, m_ciTemp);

  if (m_params && m_params->getCount() > 0) {
    ar->pushCallInfo(m_ciTemp);
    m_params->preOutputCPP(cg, ar, state);
    ar->popCallInfo();
  }

  if (state & FixOrder) {
    ar->pushCallInfo(m_ciTemp);
    preOutputStash(cg, ar, state);
    ar->popCallInfo();
  }
  if (hasCPPTemp() && !(state & FixOrder)) {
    cg_printf("id(%s);\n", cppTemp().c_str());
  }
  return true;
}

void ObjectMethodExpression::outputCPPImpl(CodeGenerator &cg,
    AnalysisResultPtr ar) {

  bool fewParams = canInvokeFewArgs();
  if (!m_name.empty() && m_valid && m_object->getType()->isSpecificObject()) {
    // Static method call
    outputCPPObjectCall(cg, ar);
    cg_printf("%s%s(", Option::MethodPrefix, m_name.c_str());
    FunctionScope::outputCPPArguments(m_params, cg, ar, m_extraArg,
        m_variableArgument, m_argArrayId, m_argArrayHash, m_argArrayIndex);
    cg_printf(")");
  } else {
    cg_printf("(mcp%d.bindClass(info)->", m_ciTemp);
    if (fewParams) {
      cg_printf("getMethFewArgs())(mcp%d, ", m_ciTemp);
      int pcount = m_params ? m_params->getCount() : 0;
      if (pcount) {
        cg_printf("%d, ", pcount);
        ar->pushCallInfo(m_ciTemp);
        FunctionScope::outputCPPArguments(m_params, cg, ar, 0, false);
        ar->popCallInfo();
      } else {
        cg_printf("0");
      }
      for (int i = pcount; i < Option::InvokeFewArgsCount; ++i) {
        cg.printf(", null_variant");
      }
      cg_printf(")");
    } else {
      cg_printf("getMeth())(mcp%d, ", m_ciTemp);
      if (m_params && m_params->getCount()) {
        ar->pushCallInfo(m_ciTemp);
        FunctionScope::outputCPPArguments(m_params, cg, ar, -1, false);
        ar->popCallInfo();
      } else {
        cg_printf("Array()");
      }
      cg_printf(")");
    }
  }
}
