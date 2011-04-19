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

#include <compiler/expression/object_property_expression.h>
#include <compiler/expression/scalar_expression.h>
#include <compiler/expression/expression_list.h>
#include <compiler/analysis/code_error.h>
#include <compiler/analysis/class_scope.h>
#include <compiler/analysis/function_scope.h>
#include <compiler/analysis/variable_table.h>
#include <compiler/option.h>
#include <compiler/expression/simple_variable.h>
#include <util/hash.h>
#include <util/parser/hphp.tab.hpp>

using namespace HPHP;
using namespace std;
using namespace boost;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

ObjectPropertyExpression::ObjectPropertyExpression
(EXPRESSION_CONSTRUCTOR_PARAMETERS,
 ExpressionPtr object, ExpressionPtr property)
  : Expression(EXPRESSION_CONSTRUCTOR_PARAMETER_VALUES),
    m_object(object), m_property(property),
    m_localEffects(AccessorEffect), m_propSym(NULL) {
  m_valid = false;
  m_propSymValid = false;
  m_object->setContext(Expression::ObjectContext);
  m_object->setContext(Expression::AccessContext);
}

ExpressionPtr ObjectPropertyExpression::clone() {
  ObjectPropertyExpressionPtr exp(new ObjectPropertyExpression(*this));
  Expression::deepCopy(exp);
  exp->m_object = Clone(m_object);
  exp->m_property = Clone(m_property);
  return exp;
}

///////////////////////////////////////////////////////////////////////////////
// parser functions

///////////////////////////////////////////////////////////////////////////////
// static analysis functions

bool ObjectPropertyExpression::isTemporary() const {
  return !m_valid && !(m_context & (LValue | RefValue | UnsetContext));
}

bool ObjectPropertyExpression::isNonPrivate(AnalysisResultPtr ar) {
  // To tell whether a property is declared as private in the context
  ClassScopePtr cls = getOriginalClass();
  if (!cls || !cls->getVariables()->hasNonStaticPrivate()) return true;
  if (m_property->getKindOf() != Expression::KindOfScalarExpression) {
    return false;
  }
  ScalarExpressionPtr name =
    dynamic_pointer_cast<ScalarExpression>(m_property);
  string propName = name->getString();
  Symbol *sym = cls->getVariables()->getSymbol(propName);
  if (!sym || sym->isStatic() || !sym->isPrivate()) return true;
  return false;
}

void ObjectPropertyExpression::setContext(Context context) {
  m_context |= context;
  switch (context) {
    case Expression::LValue:
      if (!hasContext(Expression::UnsetContext)) {
        m_object->setContext(Expression::LValue);
      }
      break;
    case Expression::DeepAssignmentLHS:
    case Expression::DeepOprLValue:
    case Expression::ExistContext:
    case Expression::UnsetContext:
    case Expression::DeepReference:
    case Expression::InvokeArgument:
      m_object->setContext(context);
      break;
    case Expression::RefValue:
    case Expression::RefParameter:
      m_object->setContext(DeepReference);
      break;
    default:
      break;
  }
  if (!m_valid &&
      (m_context & (LValue|RefValue)) &&
      !(m_context & AssignmentLHS)) {
    setEffect(CreateEffect);
  }
  if (context == InvokeArgument) {
    setContext(NoLValueWrapper);
  }
}
void ObjectPropertyExpression::clearContext(Context context) {
  m_context &= ~context;
  switch (context) {
    case Expression::LValue:
    case Expression::DeepOprLValue:
    case Expression::DeepAssignmentLHS:
    case Expression::UnsetContext:
    case Expression::DeepReference:
    case Expression::InvokeArgument:
      m_object->clearContext(context);
      break;
    case Expression::RefValue:
    case Expression::RefParameter:
      m_object->clearContext(DeepReference);
      break;
    default:
      break;
  }

  if (!(m_context & (LValue|RefValue))) {
    clearEffect(CreateEffect);
  }
  if (context == InvokeArgument) {
    clearContext(NoLValueWrapper);
  }
}

void ObjectPropertyExpression::analyzeProgram(AnalysisResultPtr ar) {
  m_object->analyzeProgram(ar);
  m_property->analyzeProgram(ar);
  if (ar->getPhase() == AnalysisResult::AnalyzeFinal) {
    if (!m_valid && m_context & (LValue|RefValue|DeepReference|UnsetContext)) {
      FunctionScopePtr func = getFunctionScope();
      if (func) func->setNeedsRefTemp();
    }
  }
}

ConstructPtr ObjectPropertyExpression::getNthKid(int n) const {
  switch (n) {
    case 0:
      return m_object;
    case 1:
      return m_property;
    default:
      ASSERT(false);
      break;
  }
  return ConstructPtr();
}

int ObjectPropertyExpression::getKidCount() const {
  return 2;
}

void ObjectPropertyExpression::setNthKid(int n, ConstructPtr cp) {
  switch (n) {
    case 0:
      m_object = boost::dynamic_pointer_cast<Expression>(cp);
      break;
    case 1:
      m_property = boost::dynamic_pointer_cast<Expression>(cp);
      break;
    default:
      ASSERT(false);
      break;
  }
}

void ObjectPropertyExpression::setEffect(Effect effect) {
  if ((m_localEffects & effect) != effect) {
    recomputeEffects();
    m_localEffects |= effect;
  }
}

void ObjectPropertyExpression::clearEffect(Effect effect) {
  if (m_localEffects & effect) {
    recomputeEffects();
    m_localEffects &= ~effect;
  }
}

TypePtr ObjectPropertyExpression::inferTypes(AnalysisResultPtr ar,
                                             TypePtr type, bool coerce) {
  m_valid = false;

  ConstructPtr self = shared_from_this();
  TypePtr objectType = m_object->inferAndCheck(ar, Type::Some, false);

  if (!m_property->is(Expression::KindOfScalarExpression)) {
    m_property->inferAndCheck(ar, Type::String, false);

    // we also lost track of which class variable an expression is about, hence
    // any type inference could be wrong. Instead, we just force variants on
    // all class variables.
    if (m_context & (LValue | RefValue)) {
      ar->forceClassVariants(getOriginalClass(), false);
    }

    return Type::Variant; // we have to use a variant to hold dynamic value
  }

  ScalarExpressionPtr exp = dynamic_pointer_cast<ScalarExpression>(m_property);
  string name = exp->getString();
  ASSERT(!name.empty());

  m_property->inferAndCheck(ar, Type::String, false);

  ClassScopePtr cls;
  if (objectType && !objectType->getName().empty()) {
    // what object-> has told us
    cls = ar->findExactClass(shared_from_this(), objectType->getName());
  } else {
    if ((m_context & LValue) && objectType &&
        !objectType->is(Type::KindOfObject) &&
        !objectType->is(Type::KindOfVariant) &&
        !objectType->is(Type::KindOfSome) &&
        !objectType->is(Type::KindOfAny)) {
      m_object->inferAndCheck(ar, Type::Object, true);
    }
  }

  if (!cls) {
    if (m_context & (LValue | RefValue | DeepReference | UnsetContext)) {
      ar->forceClassVariants(name, getOriginalClass(), false);
    }
    return Type::Variant;
  }

  int prop = hasContext(AssignmentLHS) ? ClassScope::MayHaveUnknownPropSetter :
    hasContext(ExistContext) ? ClassScope::MayHaveUnknownPropTester :
    hasContext(UnsetContext) && hasContext(LValue) ?
    ClassScope::MayHavePropUnsetter : ClassScope::MayHaveUnknownPropGetter;
  if ((m_context & (AssignmentLHS|OprLValue)) ||
      !cls->implementsAccessor(prop)) {
    clearEffect(AccessorEffect);
  }

  // resolved to this class
  if (m_context & RefValue) {
    type = Type::Variant;
    coerce = true;
  }

  // use $this inside a static function
  if (m_object->isThis()) {
    FunctionScopePtr func = m_object->getOriginalFunction();
    if (!func || func->isStatic()) {
      if (getScope()->isFirstPass()) {
        Compiler::Error(Compiler::MissingObjectContext, self);
      }
      m_actualType = Type::Variant;
      return m_actualType;
    }
  }

  if (!m_propSym || cls != m_objectClass.lock()) {
    m_objectClass = cls;
    ClassScopePtr parent;
    m_propSym = cls->findProperty(parent, name, ar, self);
    assert(m_propSym);
    if (!parent) {
      parent = cls;
    }
    m_propSymValid = m_propSym->isPresent() &&
      (!m_propSym->isPrivate() ||
       getOriginalClass() == parent) &&
      !m_propSym->isStatic();

    if (m_propSymValid) {
      parent->addUse(getScope(), BlockScope::UseKindNonStaticRef);
    }
  }

  TypePtr ret;
  if (m_propSymValid && (!cls->derivesFromRedeclaring() ||
                         m_propSym->isPrivate())) {
    ret = cls->checkProperty(m_propSym, type, coerce, ar);
    assert(m_object->getType()->isSpecificObject());
    m_valid = true;

    clearEffect(AccessorEffect);
    clearEffect(CreateEffect);
    return ret;
  } else {
    m_actualType = Type::Variant;
    return m_actualType;
  }
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void ObjectPropertyExpression::outputPHP(CodeGenerator &cg,
                                         AnalysisResultPtr ar) {
  m_object->outputPHP(cg, ar);
  cg_printf("->");
  if (m_property->getKindOf() == Expression::KindOfScalarExpression) {
    m_property->outputPHP(cg, ar);
  } else {
    cg_printf("{");
    m_property->outputPHP(cg, ar);
    cg_printf("}");
  }
}

void ObjectPropertyExpression::preOutputStash(CodeGenerator &cg,
                                              AnalysisResultPtr ar,
                                              int state) {
  Expression::preOutputStash(cg, ar, state);
}

bool ObjectPropertyExpression::preOutputCPP(CodeGenerator &cg,
                                            AnalysisResultPtr ar, int state) {
  return preOutputOffsetLHS(cg, ar, state);
}

void ObjectPropertyExpression::outputCPPImpl(CodeGenerator &cg,
                                             AnalysisResultPtr ar) {
  outputCPPObjProperty(cg, ar, false);
}

void ObjectPropertyExpression::outputCPPObjProperty(CodeGenerator &cg,
                                                    AnalysisResultPtr ar,
                                                    int doExist) {
  string func = Option::ObjectPrefix;
  const char *error = ", true";
  std::string context = "";
  bool doUnset = m_context & LValue && m_context & UnsetContext;
  bool needTemp = false;

  if (cg.getOutput() != CodeGenerator::SystemCPP) {
    context = originalClassName(cg, true);
  }
  if (doUnset) {
    func = "o_unset";
    error = "";
  } else if (doExist) {
    func = doExist > 0 ? "o_isset" : "o_empty";
    error = "";
  } else {
    if (m_context & ExistContext) {
      error = ", false";
    }
    if (m_context & InvokeArgument) {
      ASSERT(cg.callInfoTop() != -1);
      func += "argval";
    } else if (m_context & (LValue | RefValue | DeepReference | UnsetContext)) {
      if (m_context & UnsetContext) {
        assert(!(m_context & LValue)); // handled by doUnset
        func += "unsetLval";
      } else {
        func += "lval";
      }
      error = "";
      needTemp = true;
    } else {
      func += "get";
      if (isNonPrivate(ar)) {
        func += "Public";
        context = "";
      }
    }
  }

  if (m_valid && doExist) cg_printf(doExist > 0 ? "isset(" : "empty(");
  bool flag = outputCPPObject(cg, ar, doUnset || (!m_valid && doExist));
  if (flag) {
    cg_printf("id(");
    outputCPPProperty(cg, ar);
    cg_printf(")");
    if (doExist) cg_printf(", %s", doExist > 0 ? "false" : "true");
    cg_printf(")");
  } else if (!doUnset && m_valid) {
    assert(m_object->getType()->isSpecificObject());
    ScalarExpressionPtr name =
      dynamic_pointer_cast<ScalarExpression>(m_property);
    cg_printf("%s%s", Option::PropertyPrefix, name->getString().c_str());
    if (doExist) cg_printf(")");
  } else {
    cg_printf("%s(", func.c_str());
    if (hasContext(InvokeArgument)) {
      cg_printf("cit%d->isRef(%d), ", cg.callInfoTop(), m_argNum);
    }
    outputCPPProperty(cg, ar);
    if (needTemp) {
      const string &tmp = cg.getReferenceTemp();
      context = ", " + (tmp.empty() ? "Variant()" : tmp) + context;
    }
    cg_printf("%s%s)", error, context.c_str());
  }
}

bool ObjectPropertyExpression::outputCPPObject(CodeGenerator &cg,
                                               AnalysisResultPtr ar,
                                               bool noEvalOnError) {
  if (m_object->isThis()) {
    if (m_valid) {
      if (!m_object->getOriginalClass()) {
        m_valid = false;
      } else {
        FunctionScopeRawPtr fs = m_object->getOriginalFunction();
        if (!fs || fs->isStatic()) {
          m_valid = false;
        } else if (m_object->getOriginalClass() != getClassScope()) {
          if (m_object->getOriginalClass()->isRedeclaring()) {
            m_valid = false;
          } else {
            m_objectClass = getClassScope();
          }
        }
      }
    }
    if (!m_valid) {
      if (!getClassScope() || getClassScope()->derivedByDynamic() ||
          !static_pointer_cast<SimpleVariable>(m_object)->isGuardedThis()) {
        cg_printf("GET_THIS_ARROW()");
      }
    }
  } else if (m_valid) {
    TypePtr act;
    if (!m_object->hasCPPTemp() && m_object->getImplementedType() &&
        !Type::SameType(m_object->getImplementedType(),
                         m_object->getActualType())) {
      act = m_object->getActualType();
      m_object->setActualType(m_object->getImplementedType());
      ClassScopePtr cls = ar->findExactClass(shared_from_this(),
                                             act->getName());
      cg_printf("((%s%s*)", Option::ClassPrefix, cls->getId(cg).c_str());
    }
    m_object->outputCPP(cg, ar);
    if (act) {
      if (m_object->getImplementedType()->is(Type::KindOfObject)) {
        cg_printf(".get())");
      } else {
        cg_printf(".getObjectData())");
      }
      m_object->setActualType(act);
    }
    cg_printf("->");
  } else {
    TypePtr t = m_object->getType();
    bool ok = t && (t->is(Type::KindOfObject) || t->is(Type::KindOfVariant));
    if (noEvalOnError && !ok) {
      if (!t || !t->is(Type::KindOfArray)) {
        cg_printf("(");
        if (m_object->outputCPPUnneeded(cg, ar)) cg_printf(", ");
        return true;
      }
    }
    ok = ok || !t;
    if (!ok) cg_printf("Variant(");
    m_object->outputCPP(cg, ar);
    if (!ok) cg_printf(")");
    cg_printf(".");
  }

  if (m_valid && m_propSym->isPrivate() &&
      m_objectClass != getOriginalClass()) {
    cg_printf("%s%s::",
              Option::ClassPrefix, getOriginalClass()->getId(cg).c_str());
  }
  return false;
}

void ObjectPropertyExpression::outputCPPProperty(CodeGenerator &cg,
    AnalysisResultPtr ar) {
  if (m_property->getKindOf() == Expression::KindOfScalarExpression) {
    ScalarExpressionPtr name =
      dynamic_pointer_cast<ScalarExpression>(m_property);
    cg_printString(name->getString(), ar, shared_from_this());
  } else {
    m_property->outputCPP(cg, ar);
  }
}

void ObjectPropertyExpression::outputCPPExistTest(CodeGenerator &cg,
                                                  AnalysisResultPtr ar,
                                                  int op) {
  outputCPPObjProperty(cg, ar, op == T_ISSET ? 1 : -1);
}

void ObjectPropertyExpression::outputCPPUnset(CodeGenerator &cg,
                                              AnalysisResultPtr ar) {
  outputCPPObjProperty(cg, ar, 0);
}
