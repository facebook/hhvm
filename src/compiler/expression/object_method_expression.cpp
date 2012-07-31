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

#include <compiler/expression/object_method_expression.h>
#include <compiler/expression/scalar_expression.h>
#include <compiler/expression/expression_list.h>
#include <compiler/analysis/code_error.h>
#include <compiler/analysis/class_scope.h>
#include <compiler/analysis/file_scope.h>
#include <compiler/analysis/function_scope.h>
#include <compiler/statement/statement.h>
#include <util/util.h>
#include <util/hash.h>
#include <compiler/option.h>
#include <compiler/expression/simple_variable.h>
#include <compiler/analysis/variable_table.h>
#include <compiler/parser/parser.h>

using namespace HPHP;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

ObjectMethodExpression::ObjectMethodExpression
(EXPRESSION_CONSTRUCTOR_PARAMETERS,
 ExpressionPtr object, ExpressionPtr method, ExpressionListPtr params)
  : FunctionCall(
      EXPRESSION_CONSTRUCTOR_PARAMETER_VALUES(ObjectMethodExpression),
      method, "", params, ExpressionPtr()),
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
              (ar->isSystem() || func->isAbstract() ||
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
    m_object = boost::dynamic_pointer_cast<Expression>(cp);
  } else {
    FunctionCall::setNthKid(n, cp);
  }
}

TypePtr ObjectMethodExpression::inferTypes(AnalysisResultPtr ar,
                                           TypePtr type, bool coerce) {
  ASSERT(false);
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
  m_params->resetOutputCount();
}

ExpressionPtr ObjectMethodExpression::preOptimize(AnalysisResultConstPtr ar) {
  if (ar->getPhase() < AnalysisResult::FirstPreOptimize) {
    return ExpressionPtr();
  }

  if (m_classScope && m_funcScope &&
      (!m_funcScope->isVirtual() ||
       (!ar->isSystem() && !m_funcScope->hasOverride()))) {

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
  ASSERT(type);
  IMPLEMENT_INFER_AND_CHECK_ASSERT(getScope());
  resetTypes();
  reset();

  ConstructPtr self = shared_from_this();
  TypePtr objectType = m_object->inferAndCheck(ar, Type::Some, false);
  m_valid = true;
  m_bindClass = true;

  if (m_name.empty()) {
    m_nameExp->inferAndCheck(ar, Type::String, false);
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
       (ar->isSystem() || func->isAbstract() ||
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
    ASSERT(m_actualType);
    return m_actualType;
  }

  ASSERT(func);
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

void ObjectMethodExpression::outputCPPObject(CodeGenerator &cg,
                                             AnalysisResultPtr ar) {
  bool isThis = m_object->isThis();
  if (isThis) {
    TypePtr thisImplType(m_object->getImplementedType());
    TypePtr thisActType (m_object->getActualType());
    bool close = false;
    bool checkedThis = getFunctionScope()->isStatic();
    if (thisImplType) {
      ASSERT(thisActType);
      ASSERT(!Type::SameType(thisActType, thisImplType));
      ClassScopePtr implCls(thisImplType->getClass(ar, getScope()));
      if (implCls &&
          !implCls->derivesFrom(ar, thisActType->getName(), true, false)) {
        // This happens in this case:
        // if ($this instanceof Y) {
        //   ... $this->meth() ...
        // }
        ClassScopePtr cls(thisActType->getClass(ar, getScope()));
        ASSERT(cls && !cls->derivedByDynamic()); // since we don't do type
                                                 // assertions for these
        if (!cls->derivesFrom(ar, thisImplType->getName(), true, false)) {
          ASSERT(cls->derivesFromRedeclaring() &&
                 implCls->derivedByDynamic());
          checkedThis = true;
        }
        cg_printf("static_cast<%s%s*>(",
                  Option::ClassPrefix,
                  cls->getId().c_str());
        close = true;
      }
    }
    if (checkedThis) {
      if (close) {
        cg_printf("GET_THIS_VALID()");
      } else {
        cg_printf("GET_THIS_ARROW()");
      }
    } else {
      if (close) cg_printf("this");
    }
    if (close) {
      cg_printf(")->");
    }
  } else {
    TypePtr t = m_object->getType();
    bool ok = !t || t->is(Type::KindOfObject) || t->is(Type::KindOfVariant);
    if (!ok) cg_printf("Variant(");
    m_object->outputCPP(cg, ar);
    if (!ok) cg_printf(")");
  }
}


void ObjectMethodExpression::outputCPPObjectCall(CodeGenerator &cg,
                                                 AnalysisResultPtr ar) {
  outputCPPObject(cg, ar);
  bool isThis = m_object->isThis();
  if (!isThis) {
    m_object->outputCPPGuardedObjectPtr(cg);
    cg_printf("->");
  }

  if (m_bindClass && m_classScope) {
    cg_printf(" BIND_CLASS_ARROW(%s) ", m_classScope->getId().c_str());
  }
}

bool ObjectMethodExpression::preOutputCPP(CodeGenerator &cg,
                                          AnalysisResultPtr ar, int state) {
  if (!m_name.empty() && m_valid && m_object->getType()->isSpecificObject()) {
    if (m_object->hasEffect()) {
      if (cg.inExpression()) {
        m_object->preOutputCPP(cg, ar, FixOrder);
        if (m_params) m_params->preOutputCPP(cg, ar, 0);
        if (state & FixOrder) {
          preOutputStash(cg, ar, state);
        }
      }
      return true;
    }
    return FunctionCall::preOutputCPP(cg, ar, state);
  }
  // Short circuit out if inExpression() returns false
  if (!cg.inExpression()) return true;
  cg.wrapExpressionBegin();
  m_ciTemp = cg.createNewLocalId(shared_from_this());

  if (outputLineMap(cg, ar)) cg_printf("0);\n");

  bool isThis = m_object->isThis();
  if (!isThis) {
    int s = 0;
    if (m_name.empty() &&
        m_nameExp->hasEffect() && !m_object->isScalar()) {
      s = FixOrder;
    }
    m_object->preOutputCPP(cg, ar, s);
  }
  if (m_name.empty()) {
    m_nameExp->preOutputCPP(cg, ar, 0);
  }
  cg_printf("MethodCallPackage mcp%d;\n", m_ciTemp);

  if (!isThis) {
    TypePtr t = m_object->getType();
    if (t && t->is(Type::KindOfObject)) {
      cg_printf("CObjRef obj%d = ", m_ciTemp);
    } else {
      cg_printf("CVarRef obj%d = ", m_ciTemp);
    }
    outputCPPObject(cg, ar);
    cg_printf(";\n");
  }
  if (m_name.empty()) {
    cg_printf("CStrRef mth%d = ", m_ciTemp);
    m_nameExp->outputCPP(cg, ar);
    cg_printf(";\n");
  }

  cg_printf("mcp%d.methodCall((", m_ciTemp);
  if (isThis) {
    if (!getClassScope() || getClassScope()->derivedByDynamic() ||
        !static_pointer_cast<SimpleVariable>(m_object)->isGuarded()) {
      cg_printf("GET_THIS_VALID()");
    } else {
      cg_printf("this");
    }
  } else {
    cg_printf("obj%d", m_ciTemp);
    m_object->outputCPPGuardedObjectPtr(cg);
  }
  cg_printf("), ");
  if (!m_name.empty()) {
    uint64 hash = hash_string_i(m_name.c_str());
    cg_printString(m_origName, ar, shared_from_this());
    cg_printf(", 0x%016llXLL);\n", hash);
  } else {
    cg_printf("mth%d, -1);\n", m_ciTemp);
  }
  cg_printf("const CallInfo *cit%d ATTRIBUTE_UNUSED = mcp%d.ci;\n",
            m_ciTemp, m_ciTemp);

  if (m_params && m_params->getCount() > 0) {
    cg.pushCallInfo(m_ciTemp);
    m_params->preOutputCPP(cg, ar, 0);
    cg.popCallInfo();
  }

  if (state & FixOrder) {
    cg.pushCallInfo(m_ciTemp);
    preOutputStash(cg, ar, state);
    cg.popCallInfo();
  }

  return true;
}

void ObjectMethodExpression::outputCPPImpl(CodeGenerator &cg,
                                           AnalysisResultPtr ar) {
  if (!m_name.empty() && m_valid && m_object->getType()->isSpecificObject()) {
    // Direct method call
    ClassScopePtr cls(m_object->getType()->getClass(ar, getScope()));
    if (cls) getFileScope()->addUsedClassFullHeader(cls);

    outputCPPObjectCall(cg, ar);
    cg_printf("%s%s(", m_funcScope ?
              m_funcScope->getPrefix(ar, m_params) : Option::MethodPrefix,
              m_name.c_str());

    FunctionScope::OutputCPPArguments(m_params, m_funcScope, cg, ar,
                                      m_extraArg, m_variableArgument,
                                      m_argArrayId, m_argArrayHash,
                                      m_argArrayIndex);
    cg_printf(")");
  } else {
    bool maybeStatic = true;
    if (Option::WholeProgram && !ar->isSystem() && !m_name.empty()) {
      FunctionScope::FunctionInfoPtr info =
        FunctionScope::GetFunctionInfo(m_name);
      if (info && !info->getMaybeStatic()) maybeStatic = false;
    }
    cg_printf("(mcp%d.%s->",
              m_ciTemp, maybeStatic ? "bindClass(fi)" : "ci" );
    outputDynamicCall(cg, ar, true);
  }
}
