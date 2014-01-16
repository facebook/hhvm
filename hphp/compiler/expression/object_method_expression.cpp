/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/compiler/expression/object_method_expression.h"
#include "hphp/compiler/expression/scalar_expression.h"
#include "hphp/compiler/expression/expression_list.h"
#include "hphp/compiler/analysis/code_error.h"
#include "hphp/compiler/analysis/class_scope.h"
#include "hphp/compiler/analysis/file_scope.h"
#include "hphp/compiler/analysis/function_scope.h"
#include "hphp/compiler/statement/statement.h"
#include "hphp/util/util.h"
#include "hphp/util/hash.h"
#include "hphp/compiler/option.h"
#include "hphp/compiler/expression/simple_variable.h"
#include "hphp/compiler/analysis/variable_table.h"
#include "hphp/compiler/parser/parser.h"

using namespace HPHP;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

ObjectMethodExpression::ObjectMethodExpression
(EXPRESSION_CONSTRUCTOR_PARAMETERS,
 ExpressionPtr object, ExpressionPtr method, ExpressionListPtr params)
  : FunctionCall(
      EXPRESSION_CONSTRUCTOR_PARAMETER_VALUES(ObjectMethodExpression),
      method, "", false, params, ExpressionPtr()),
    m_object(object), m_bindClass(true) {
  m_object->setContext(Expression::ObjectContext);
  m_object->clearContext(Expression::LValue);
  m_object->clearContext(Expression::AccessContext);
}

ExpressionPtr ObjectMethodExpression::clone() {
  ObjectMethodExpressionPtr exp(new ObjectMethodExpression(*this));
  FunctionCall::deepCopy(exp);
  exp->m_object = Clone(m_object);
  return exp;
}

///////////////////////////////////////////////////////////////////////////////
// parser functions

///////////////////////////////////////////////////////////////////////////////
// static analysis functions

void ObjectMethodExpression::analyzeProgram(AnalysisResultPtr ar) {
  FunctionCall::analyzeProgram(ar);
  m_object->analyzeProgram(ar);

  if (ar->getPhase() == AnalysisResult::AnalyzeAll) {
    FunctionScopePtr func = m_funcScope;
    if (!func && m_object->isThis() && !m_name.empty()) {
      ClassScopePtr cls = getClassScope();
      if (cls) {
        m_classScope = cls;
        func = cls->findFunction(ar, m_name, true, true);
        if (func &&
            !cls->isInterface() &&
            !(func->isVirtual() &&
              (func->isAbstract() ||
               (func->hasOverride() &&
                cls->getAttribute(ClassScope::NotFinal))) &&
              !func->isPerfectVirtual())) {
          m_funcScope = func;
          func->addCaller(getScope());
        }
      }
    }

    markRefParams(func, m_name, canInvokeFewArgs());
  }

  // This is OK because AnalyzeFinal is guaranteed to run for a CPP
  // target, regardless of opts (and we only need the following
  // for CPP targets)
  if (ar->getPhase() == AnalysisResult::AnalyzeFinal) {
    // necessary because we set the expected type of m_object to
    // Type::Some during type inference.
    TypePtr at(m_object->getActualType());
    TypePtr it(m_object->getImplementedType());
    if (!m_object->isThis() && at && at->is(Type::KindOfObject)) {
      if (at->isSpecificObject() && it && Type::IsMappedToVariant(it)) {
        // fast-cast inference
        ClassScopePtr scope(ar->findClass(at->getName()));
        if (scope) {
          // add a dependency to m_object's class type
          // to allow the fast cast to succeed
          addUserClass(ar, at->getName());
        }
      }
      m_object->setExpectedType(at);
    }
  }
}

ConstructPtr ObjectMethodExpression::getNthKid(int n) const {
  if (!n) return m_object;
  return FunctionCall::getNthKid(n);
}

void ObjectMethodExpression::setNthKid(int n, ConstructPtr cp) {
  if (!n) {
    m_object = dynamic_pointer_cast<Expression>(cp);
  } else {
    FunctionCall::setNthKid(n, cp);
  }
}

TypePtr ObjectMethodExpression::inferTypes(AnalysisResultPtr ar,
                                           TypePtr type, bool coerce) {
  assert(false);
  return TypePtr();
}

void ObjectMethodExpression::setInvokeParams(AnalysisResultPtr ar) {
  FunctionScope::FunctionInfoPtr info;
  if (Option::WholeProgram) {
    info = FunctionScope::GetFunctionInfo(m_name);
  }
  if (!Option::WholeProgram || info || m_name.empty()) {
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
}

ExpressionPtr ObjectMethodExpression::preOptimize(AnalysisResultConstPtr ar) {
  if (ar->getPhase() < AnalysisResult::FirstPreOptimize) {
    return ExpressionPtr();
  }

  if (m_classScope && m_funcScope &&
      (!m_funcScope->isVirtual() ||
       (Option::WholeProgram && !m_funcScope->hasOverride()))) {

    if (Option::DynamicInvokeFunctions.size()) {
      if (Option::DynamicInvokeFunctions.find(
            m_classScope->getName() + "::" + m_funcScope->getName()) !=
          Option::DynamicInvokeFunctions.end()) {
        setNoInline();
      }
    }

    return inliner(ar, m_object, "");
  }

  return ExpressionPtr();
}

TypePtr ObjectMethodExpression::inferAndCheck(AnalysisResultPtr ar,
                                              TypePtr type, bool coerce) {
  assert(type);
  IMPLEMENT_INFER_AND_CHECK_ASSERT(getScope());
  resetTypes();
  reset();

  ConstructPtr self = shared_from_this();
  TypePtr objectType = m_object->inferAndCheck(ar, Type::Some, false);
  m_valid = true;
  m_bindClass = true;

  if (m_name.empty()) {
    m_nameExp->inferAndCheck(ar, Type::Some, false);
    setInvokeParams(ar);
    // we have to use a variant to hold dynamic value
    return checkTypesImpl(ar, type, Type::Variant, coerce);
  }

  ClassScopePtr cls;
  if (objectType && !objectType->getName().empty()) {
    if (m_classScope && !strcasecmp(objectType->getName().c_str(),
                                    m_classScope->getName().c_str())) {
      cls = m_classScope;
    } else {
      cls = ar->findExactClass(shared_from_this(), objectType->getName());
    }
  }

  if (!cls) {
    m_classScope.reset();
    m_funcScope.reset();

    m_valid = false;
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
      if (!cls->isTrait() &&
          !cls->getAttribute(ClassScope::MayHaveUnknownMethodHandler) &&
          !cls->getAttribute(ClassScope::HasUnknownMethodHandler) &&
          !cls->getAttribute(ClassScope::InheritsUnknownMethodHandler)) {
        if (ar->classMemberExists(m_name, AnalysisResult::MethodName)) {
          if (!Option::AllDynamic) {
            setDynamicByIdentifier(ar, m_name);
          }
        } else {
          Compiler::Error(Compiler::UnknownObjectMethod, self);
        }
      }

      m_valid = false;
      setInvokeParams(ar);
      return checkTypesImpl(ar, type, Type::Variant, coerce);
    }
    m_funcScope = func;
    func->addCaller(getScope(), !type->is(Type::KindOfAny));
  }

  bool valid = true;
  m_bindClass = func->isStatic();

  // use $this inside a static function
  if (m_object->isThis()) {
    FunctionScopePtr localfunc = getFunctionScope();
    if (localfunc->isStatic()) {
      if (getScope()->isFirstPass()) {
        Compiler::Error(Compiler::MissingObjectContext, self);
      }
      valid = false;
    }
  }

  // invoke() will return Variant
  if (cls->isInterface() ||
      (func->isVirtual() &&
       (!Option::WholeProgram || func->isAbstract() ||
        (func->hasOverride() && cls->getAttribute(ClassScope::NotFinal))) &&
       !func->isPerfectVirtual())) {
    valid = false;
  }

  if (!valid) {
    setInvokeParams(ar);
    checkTypesImpl(ar, type, Type::Variant, coerce);
    m_valid = false; // so we use invoke() syntax
    if (!Option::AllDynamic) {
      func->setDynamic();
    }
    assert(m_actualType);
    return m_actualType;
  }

  assert(func);
  return checkParamsAndReturn(ar, type, coerce, func, false);
}

///////////////////////////////////////////////////////////////////////////////

void ObjectMethodExpression::outputCodeModel(CodeGenerator &cg) {
  cg.printObjectHeader("ObjectMethodCallExpression",
      m_params == nullptr ? 3 : 4);
  cg.printPropertyHeader("object");
  m_object->outputCodeModel(cg);
  if (m_nameExp->is(Expression::KindOfScalarExpression)) {
    cg.printPropertyHeader("methodName");
  } else {
    cg.printPropertyHeader("methodExpression");
  }
  m_nameExp->outputCodeModel(cg);
  if (m_params != nullptr) {
    cg.printPropertyHeader("arguments");
    cg.printExpressionVector(m_params);
  }
  cg.printPropertyHeader("sourceLocation");
  cg.printLocation(this->getLocation());
  cg.printObjectFooter();
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void ObjectMethodExpression::outputPHP(CodeGenerator &cg,
                                       AnalysisResultPtr ar) {
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
