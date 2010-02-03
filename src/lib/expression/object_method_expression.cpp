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

#include <lib/expression/object_method_expression.h>
#include <lib/expression/scalar_expression.h>
#include <lib/expression/expression_list.h>
#include <lib/analysis/code_error.h>
#include <lib/analysis/class_scope.h>
#include <lib/analysis/function_scope.h>
#include <lib/analysis/dependency_graph.h>
#include <lib/statement/statement.h>
#include <util/util.h>
#include <util/hash.h>
#include <lib/option.h>
#include <lib/expression/simple_variable.h>
#include <lib/analysis/variable_table.h>

using namespace HPHP;
using namespace std;
using namespace boost;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

ObjectMethodExpression::ObjectMethodExpression
(EXPRESSION_CONSTRUCTOR_PARAMETERS,
 ExpressionPtr object, ExpressionPtr method, ExpressionListPtr params)
  : FunctionCall(EXPRESSION_CONSTRUCTOR_PARAMETER_VALUES,
                 method, "", params, NULL), m_object(object),
    m_invokeFewArgsDecision(true) {
  m_object->setContext(Expression::ObjectContext);
  m_object->clearContext(Expression::LValue);
}

ExpressionPtr ObjectMethodExpression::clone() {
  ObjectMethodExpressionPtr exp(new ObjectMethodExpression(*this));
  Expression::deepCopy(exp);
  exp->m_params = Clone(m_params);
  exp->m_object = Clone(m_object);
  exp->m_nameExp = Clone(m_nameExp);
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
  cls = ar->findClass(construct,
                      AnalysisResult::MethodName);
  if (cls && name == cls->getName()) {
    name = "__construct";
    cls->setAttribute(ClassScope::classNameConstructor);
    return cls;
  }
  return ClassScopePtr();
}

///////////////////////////////////////////////////////////////////////////////
// parser functions

///////////////////////////////////////////////////////////////////////////////
// static analysis functions

void ObjectMethodExpression::analyzeProgram(AnalysisResultPtr ar) {
  m_params->controlOrder(m_object->hasEffect() ? 2 : 1);
  m_params->analyzeProgramStart(ar);
  m_object->analyzeProgram(ar);
  m_params->analyzeProgramEnd(ar);
  m_nameExp->analyzeProgram(ar);


  if (ar->getPhase() == AnalysisResult::AnalyzeAll &&
      m_object->isThis() && !m_name.empty()) {
    ClassScopePtr cls = ar->getClassScope();
    if (cls && !cls->findFunction(ar, m_name, true)) {
      cls->addMissingMethod(m_name);
    }
  }
}

bool ObjectMethodExpression::canInvokeFewArgs() {
  // We can always change out minds about saying yes, but once we say
  // no, it sticks.
  if (m_invokeFewArgsDecision && m_params &&
      (m_params->getCount() > Option::InvokeFewArgsCount ||
       m_params->controllingOrder())) {
    m_invokeFewArgsDecision = false;
  }
  return m_invokeFewArgsDecision;
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
  bool highParamCount = !canInvokeFewArgs();
  Context newContext = highParamCount ? Expression::RefValue :
    (Expression::Context)(Expression::RefValue | Expression::NoRefWrapper);
  for (int i = 0; i < m_params->getCount(); i++) {
    ExpressionPtr param = (*m_params)[i];
    if ((param->getContext() & Expression::RefValue) == 0) {
      param->setContext(newContext);
      param->setContext(Expression::InvokeArgument);
    }
    param->inferAndCheck(ar, Type::Variant, false);
  }
  m_params->resetOutputCount();
}

TypePtr ObjectMethodExpression::inferAndCheck(AnalysisResultPtr ar,
                                              TypePtr type, bool coerce) {
  reset();

  ConstructPtr self = shared_from_this();
  TypePtr objectType = m_object->inferAndCheck(ar, NEW_TYPE(Object), true);
  m_valid = true;

  if (m_name.empty()) {
    // if dynamic property or method, we have nothing to find out
    if (ar->isFirstPass()) {
      ar->getCodeError()->record(self, CodeError::UseDynamicMethod, self);
    }
    m_nameExp->inferAndCheck(ar, Type::String, false);
    setInvokeParams(ar);
    // we have to use a variant to hold dynamic value
    return checkTypesImpl(ar, type, Type::Variant, coerce);
  }

  ClassScopePtr cls;
  if (objectType && !objectType->getName().empty()) {
    // what object-> has told us
    cls = ar->findClass(objectType->getName());
    ASSERT(cls);
  } else {
    // what ->method has told us
    cls = resolveClass(ar, m_name);
    if (!cls) {
      if (ar->isFirstPass() &&
          !ar->classMemberExists(m_name, AnalysisResult::MethodName)) {
        ar->getCodeError()->record(self, CodeError::UnknownObjectMethod, self);
      }

      setInvokeParams(ar);
      return checkTypesImpl(ar, type, Type::Variant, coerce);
    }

    m_object->inferAndCheck(ar, Type::CreateObjectType(cls->getName()), true);
  }

  FunctionScopePtr func = cls->findFunction(ar, m_name, true);
  if (!func) {
    if (ar->isFirstPass() &&
        !cls->hasAttribute(ClassScope::HasUnknownMethodHandler, ar)) {
      if (ar->classMemberExists(m_name, AnalysisResult::MethodName)) {
        // TODO: we could try to find out class derivation is present...
        ar->getCodeError()->record(self, CodeError::DerivedObjectMethod, self);
        // we have to make sure the method is in invoke list
        setDynamicByIdentifier(ar, m_name);
      } else {
        ar->getCodeError()->record(self, CodeError::UnknownObjectMethod, self);
      }
    }

    m_valid = false;
    setInvokeParams(ar);
    return checkTypesImpl(ar, type, Type::Variant, coerce);
  }
  bool valid = true;

  // use $this inside a static function
  if (m_object->isThis()) {
    FunctionScopePtr localfunc = ar->getFunctionScope();
    if (localfunc->isStatic()) {
      if (ar->isFirstPass()) {
        ar->getCodeError()->record(self, CodeError::MissingObjectContext,
                                   self);
      }
      valid = false;
    }
  }

  // invoke() will return Variant
  if (func->isVirtual() || !m_object->getType()->isSpecificObject()) {
    valid = false;
  }

  if (!valid) {
    setInvokeParams(ar);
    checkTypesImpl(ar, type, Type::Variant, coerce);
    m_valid = false; // so we use invoke() syntax
    return m_actualType;
  }

  return checkParamsAndReturn(ar, type, coerce, func);
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void ObjectMethodExpression::outputPHP(CodeGenerator &cg,
                                       AnalysisResultPtr ar) {
  outputLineMap(cg, ar);

  m_object->outputPHP(cg, ar);
  cg.printf("->");
  if (m_nameExp->getKindOf() == Expression::KindOfScalarExpression) {
    m_nameExp->outputPHP(cg, ar);
  } else {
    cg.printf("{");
    m_nameExp->outputPHP(cg, ar);
    cg.printf("}");
  }
  cg.printf("(");
  m_params->outputPHP(cg, ar);
  cg.printf(")");
}

bool ObjectMethodExpression::directVariantProxy(AnalysisResultPtr ar) {
  TypePtr actualType = m_object->getActualType();
  if (actualType && actualType->is(Type::KindOfVariant)) {
    if (m_object->is(KindOfSimpleVariable)) {
      SimpleVariablePtr var =
        dynamic_pointer_cast<SimpleVariable>(m_object);
      const std::string &name = var->getName();
      FunctionScopePtr func =
        dynamic_pointer_cast<FunctionScope>(ar->getScope());
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

void ObjectMethodExpression::outputCPPImpl(CodeGenerator &cg,
                                           AnalysisResultPtr ar) {
  bool linemap = outputLineMap(cg, ar);
  bool isThis = m_object->isThis();

  if (isThis && ar->getFunctionScope()->isStatic()) {
    cg.printf("throw_fatal(\"Using $this when not in object context\")");
    if (linemap) cg.printf(")");
    return;
  }

  stringstream objTmp;
  int objIdx = -1;
  if (m_params) {
    objIdx =
      m_params->outputCPPControlledEvalOrderPre(cg, ar,
                                                isThis ? ExpressionPtr()
                                                : m_object);
    objTmp << Option::EvalOrderTempPrefix << objIdx;
  }
  bool fewParams = canInvokeFewArgs();
  bool tooManyArgs = m_params &&
    m_params->outputCPPTooManyArgsPre(cg, ar, m_name);
  if (!isThis) {
    if (directVariantProxy(ar)) {
      if (objIdx == -1) {
        TypePtr expectedType = m_object->getExpectedType();
        ASSERT(expectedType->is(Type::KindOfObject));
        // Clear m_expectedType to avoid type cast (toObject).
        m_object->setExpectedType(TypePtr());
        m_object->outputCPP(cg, ar);
        m_object->setExpectedType(expectedType);
      } else {
        cg.printf("%s", objTmp.str().c_str());
      }
      cg.printf(".");
    } else {
      if (objIdx == -1) {
        m_object->outputCPP(cg, ar);
      } else {
        TypePtr type = m_object->getType();
        cg.printf("%s.toObject()", objTmp.str().c_str());
        if (type->isSpecificObject()) {
          cg.printf(".getTyped<%s%s>()", Option::ClassPrefix,
                    type->getName().c_str());
        }
      }
      cg.printf("->");
    }
  }

  if (!m_name.empty()) {
    if (m_valid && m_object->getType()->isSpecificObject()) {
      cg.printf("%s%s(", Option::MethodPrefix, m_name.c_str());
      FunctionScope::outputCPPArguments(m_params, cg, ar, m_extraArg,
                                        m_variableArgument);
      cg.printf(")");
      if (tooManyArgs) {
        m_params->outputCPPTooManyArgsPost(cg, ar, m_voidReturn);
      }
    } else {
      if (fewParams) {
        uint64 hash = hash_string_i(m_name.data(), m_name.size());
        cg.printf("%s%sinvoke_few_args(\"%s\"", Option::ObjectPrefix,
                  isThis ? "root_" : "", m_origName.c_str());
        cg.printf(", 0x%016llXLL, ", hash);

        if (m_params && m_params->getCount()) {
          cg.printf("%d, ", m_params->getCount());
          FunctionScope::outputCPPArguments(m_params, cg, ar, 0, false);
        } else {
          cg.printf("0");
        }
        cg.printf(")");
      } else {
        cg.printf("%s%sinvoke(\"%s\"", Option::ObjectPrefix,
                  isThis ? "root_" : "", m_origName.c_str());
        cg.printf(", ");
        if (m_params && m_params->getCount()) {
          FunctionScope::outputCPPArguments(m_params, cg, ar, -1, false);
        } else {
          cg.printf("Array()");
        }
        uint64 hash = hash_string_i(m_name.data(), m_name.size());
        cg.printf(", 0x%016llXLL)", hash);
      }
    }
  } else {
    if (fewParams) {
      cg.printf("%s%sinvoke_few_args(", Option::ObjectPrefix,
                isThis ? "root_" : "");
      m_nameExp->outputCPP(cg, ar);
      cg.printf(", -1LL, ");
      if (m_params && m_params->getCount()) {
        cg.printf("%d, ", m_params->getCount());
        FunctionScope::outputCPPArguments(m_params, cg, ar, 0, false);
      } else {
        cg.printf("0");
      }
      cg.printf(")");
    } else {
      cg.printf("%s%sinvoke((", Option::ObjectPrefix, isThis ? "root_" : "");
      m_nameExp->outputCPP(cg, ar);
      cg.printf(")");
      cg.printf(", ");
      if (m_params && m_params->getCount()) {
        FunctionScope::outputCPPArguments(m_params, cg, ar, -1, false);
      } else {
        cg.printf("Array()");
      }
      cg.printf(", -1LL)");
    }
  }
  if (m_params) m_params->outputCPPControlledEvalOrderPost(cg, ar);
  if (linemap) cg.printf(")");
}
