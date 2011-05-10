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

#include <compiler/expression/array_element_expression.h>
#include <compiler/expression/simple_variable.h>
#include <compiler/expression/scalar_expression.h>
#include <compiler/analysis/variable_table.h>
#include <compiler/analysis/code_error.h>
#include <compiler/option.h>
#include <compiler/expression/static_member_expression.h>
#include <compiler/analysis/function_scope.h>
#include <util/parser/hphp.tab.hpp>
#include <runtime/base/complex_types.h>
#include <runtime/base/builtin_functions.h>

using namespace HPHP;
using namespace std;
using namespace boost;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

ArrayElementExpression::ArrayElementExpression
(EXPRESSION_CONSTRUCTOR_PARAMETERS,
 ExpressionPtr variable, ExpressionPtr offset)
  : Expression(EXPRESSION_CONSTRUCTOR_PARAMETER_VALUES),
    m_variable(variable), m_offset(offset), m_global(false),
    m_dynamicGlobal(false), m_localEffects(AccessorEffect) {
  m_variable->setContext(Expression::AccessContext);

  if (m_variable->is(Expression::KindOfSimpleVariable)) {
    SimpleVariablePtr var =
      dynamic_pointer_cast<SimpleVariable>(m_variable);
    if (var->getName() == "GLOBALS") {
      m_global = true;
      m_dynamicGlobal = true;
      if (m_offset && m_offset->is(Expression::KindOfScalarExpression)) {
        ScalarExpressionPtr offset =
          dynamic_pointer_cast<ScalarExpression>(m_offset);

        if (offset->isLiteralString()) {
          m_globalName = offset->getIdentifier();
          if (!m_globalName.empty()) {
            m_dynamicGlobal = false;
          }
        }
      }
    }
  }
}

ExpressionPtr ArrayElementExpression::clone() {
  ArrayElementExpressionPtr exp(new ArrayElementExpression(*this));
  Expression::deepCopy(exp);
  exp->m_variable = Clone(m_variable);
  exp->m_offset = Clone(m_offset);
  return exp;
}

void ArrayElementExpression::setContext(Context context) {
  m_context |= context;
  switch (context) {
    case Expression::LValue:
      if (!hasContext(Expression::UnsetContext)) {
        m_variable->setContext(Expression::LValue);
      }
      if (m_variable->is(Expression::KindOfObjectPropertyExpression)) {
        m_variable->clearContext(Expression::NoLValueWrapper);
      }
      // special case for $GLOBALS[], see the if (m_global) check in
      // ArrayElementExpression::outputCPPImpl, we do not need lvalue wrapper
      if (m_variable->is(Expression::KindOfSimpleVariable)) {
        SimpleVariablePtr var =
          dynamic_pointer_cast<SimpleVariable>(m_variable);
        if (var->getName() == "GLOBALS") {
          m_context |= Expression::NoLValueWrapper;
        }
      }
      break;
    case Expression::DeepAssignmentLHS:
    case Expression::DeepOprLValue:
    case Expression::ExistContext:
    case Expression::UnsetContext:
    case Expression::DeepReference:
      m_variable->setContext(context);
      break;
    case Expression::RefValue:
    case Expression::RefParameter:
      m_variable->setContext(DeepReference);
      break;
    case Expression::InvokeArgument:
      m_variable->setContext(context);
      setContext(NoLValueWrapper);
    default:
      break;
  }
}

void ArrayElementExpression::clearContext(Context context) {
  m_context &= ~context;
  switch (context) {
    case Expression::LValue:
    case Expression::DeepOprLValue:
    case Expression::DeepAssignmentLHS:
    case Expression::UnsetContext:
    case Expression::DeepReference:
      m_variable->clearContext(context);
      break;
    case Expression::InvokeArgument:
      m_variable->clearContext(context);
      clearContext(NoLValueWrapper);
      break;
    case Expression::RefValue:
    case Expression::RefParameter:
      m_variable->clearContext(DeepReference);
      break;
    default:
      break;
  }
}

///////////////////////////////////////////////////////////////////////////////
// parser functions

bool ArrayElementExpression::appendClass(ExpressionPtr cls) {
  if (m_variable->is(Expression::KindOfArrayElementExpression)) {
    return dynamic_pointer_cast<ArrayElementExpression>(m_variable)
      ->appendClass(cls);
  }
  if (m_variable->is(Expression::KindOfSimpleVariable) ||
      m_variable->is(Expression::KindOfDynamicVariable)) {
    m_variable = StaticMemberExpressionPtr
      (new StaticMemberExpression(
        m_variable->getScope(), m_variable->getLocation(),
        Expression::KindOfStaticMemberExpression, cls, m_variable));
    m_global = m_dynamicGlobal = false;
    m_globalName.clear();
    return true;
  }
  return false;
}

///////////////////////////////////////////////////////////////////////////////
// static analysis functions

bool ArrayElementExpression::isTemporary() const {
  return !m_global &&
    !(m_context & (AccessContext|LValue|RefValue|UnsetContext));
}

void ArrayElementExpression::analyzeProgram(AnalysisResultPtr ar) {
  m_variable->analyzeProgram(ar);
  if (m_offset) m_offset->analyzeProgram(ar);
  if (ar->getPhase() == AnalysisResult::AnalyzeFinal) {
    if (!m_global && (m_context & AccessContext) &&
        !(m_context & (LValue|RefValue|DeepReference|
                       UnsetContext|RefParameter|InvokeArgument))) {
      TypePtr type = m_variable->getActualType();
      if (!type ||
          (!type->is(Type::KindOfString) && !type->is(Type::KindOfArray))) {
        FunctionScopePtr scope = getFunctionScope();
        if (scope) scope->setNeedsRefTemp();
      }
    }
    if (m_global) {
      if (getContext() & (LValue|RefValue|DeepReference)) {
        setContext(NoLValueWrapper);
      } else if (!m_dynamicGlobal &&
          !(getContext() &
            (LValue|RefValue|RefParameter|DeepReference|
             UnsetContext|ExistContext))) {
        VariableTablePtr vars = ar->getVariables();
        Symbol *sym = vars->getSymbol(m_globalName);
        if (!sym || sym->getDeclaration().get() == this) {
          Compiler::Error(Compiler::UseUndeclaredGlobalVariable,
                          shared_from_this());
        }
      }
      FunctionScopePtr scope = getFunctionScope();
      if (scope) scope->setNeedsCheckMem();
    }
  }
}

ConstructPtr ArrayElementExpression::getNthKid(int n) const {
  switch (n) {
    case 0:
      return m_variable;
    case 1:
      return m_offset;
    default:
      ASSERT(false);
      break;
  }
  return ConstructPtr();
}

int ArrayElementExpression::getKidCount() const {
  return 2;
}

void ArrayElementExpression::setNthKid(int n, ConstructPtr cp) {
  switch (n) {
    case 0:
      m_variable = boost::dynamic_pointer_cast<Expression>(cp);
      break;
    case 1:
      m_offset = boost::dynamic_pointer_cast<Expression>(cp);
      break;
    default:
      ASSERT(false);
      break;
  }
}

bool ArrayElementExpression::canonCompare(ExpressionPtr e) const {
  return m_offset && Expression::canonCompare(e);
}

void ArrayElementExpression::setEffect(Effect effect) {
  if ((m_localEffects & effect) != effect) {
    recomputeEffects();
    m_localEffects |= effect;
  }
}

void ArrayElementExpression::clearEffect(Effect effect) {
  if (m_localEffects & effect) {
    recomputeEffects();
    m_localEffects &= ~effect;
  }
}

ExpressionPtr ArrayElementExpression::preOptimize(AnalysisResultConstPtr ar) {
  if (!(m_context & (RefValue|LValue|UnsetContext|OprLValue|
                     InvokeArgument|DeepReference|DeepOprLValue))) {
    if (m_offset && m_variable->isScalar()) {
      Variant v, o;
      if (m_variable->getScalarValue(v)) {
        if (m_context & ExistContext &&
            !v.isArray() &&
            !v.isString() &&
            !m_offset->hasEffect()) {
          return replaceValue(makeConstant(ar, "null"));
        }
        if (m_offset->isScalar() && m_offset->getScalarValue(o)) {
          try {
            g_context->setThrowAllErrors(true);
            Variant res = v.rvalAt(
              o, hasContext(ExistContext) ?
              AccessFlags::None : AccessFlags::Error);
            g_context->setThrowAllErrors(false);
            return replaceValue(makeScalarExpression(ar, res));
          } catch (...) {
            g_context->setThrowAllErrors(false);
          }
        }
      }
    }
  }
  return ExpressionPtr();
}

/**
 * ArrayElementExpression comes from:
 *
 * reference_variable[|expr]
 * ->object_dim_list[|expr]
 * encaps T_VARIABLE[expr]
 * encaps ${T_STRING[expr]}
 */
TypePtr ArrayElementExpression::inferTypes(AnalysisResultPtr ar,
                                           TypePtr type, bool coerce) {
  ConstructPtr self = shared_from_this();

  if (m_offset &&
      !(m_context & (UnsetContext | ExistContext |
                     InvokeArgument | LValue | RefValue))) {
    setEffect(DiagnosticEffect);
  }
  if (m_context & (AssignmentLHS|OprLValue)) {
    clearEffect(AccessorEffect);
  } else if (m_context & (LValue | RefValue)) {
    setEffect(CreateEffect);
  }

  // handling $GLOBALS[...]
  if (m_variable->is(Expression::KindOfSimpleVariable)) {
    SimpleVariablePtr var =
      dynamic_pointer_cast<SimpleVariable>(m_variable);
    if (var->getName() == "GLOBALS") {
      clearEffect(AccessorEffect);
      m_global = true;
      m_dynamicGlobal = true;
      getScope()->getVariables()->
        setAttribute(VariableTable::NeedGlobalPointer);
      VariableTablePtr vars = ar->getVariables();

      if (m_offset && m_offset->is(Expression::KindOfScalarExpression)) {
        ScalarExpressionPtr offset =
          dynamic_pointer_cast<ScalarExpression>(m_offset);

        if (offset->isLiteralString()) {
          m_globalName = offset->getIdentifier();
          if (!m_globalName.empty()) {
            m_dynamicGlobal = false;
            clearEffect(DiagnosticEffect);
            getScope()->getVariables()->
              setAttribute(VariableTable::NeedGlobalPointer);
            TypePtr ret;
            if (coerce) {
              ret = vars->add(m_globalName, type, true, ar, self,
                              ModifierExpressionPtr());
            } else {
              int p;
              ret =
                vars->checkVariable(m_globalName, type, coerce, ar, self, p);
            }
            getScope()->getVariables()->addSuperGlobal(m_globalName);
            return ret;
          }
        }
      } else {
        vars->setAttribute(VariableTable::ContainsDynamicVariable);
      }

      if (hasContext(LValue) || hasContext(RefValue)) {
        ar->getVariables()->forceVariants(ar, VariableTable::AnyVars);
        ar->getVariables()->
          setAttribute(VariableTable::ContainsLDynamicVariable);
      }
      if (m_offset) {
        m_offset->inferAndCheck(ar, Type::Primitive, false);
      }
      return m_implementedType = Type::Variant; // so not to lose values
    }
  }
  if ((hasContext(LValue) || hasContext(RefValue)) &&
      !hasContext(UnsetContext)) {
    m_variable->setContext(LValue);
  }

  TypePtr varType;
  if (m_offset) {
    varType = m_variable->inferAndCheck(ar, coerce ? Type::AutoSequence :
                                        Type::Sequence, coerce);
    m_offset->inferAndCheck(ar, Type::Some, false);
  } else {
    if (hasContext(ExistContext) || hasContext(UnsetContext)) {
      if (getScope()->isFirstPass()) {
        Compiler::Error(Compiler::InvalidArrayElement, self);
      }
    }
    m_variable->inferAndCheck(ar, Type::Array, true);
  }

  if (varType && Type::SameType(varType, Type::String)) {
    clearEffect(AccessorEffect);
    m_implementedType.reset();
    return Type::String;
  }

  if (varType && Type::SameType(varType, Type::Array)) {
    clearEffect(AccessorEffect);
  }

  TypePtr ret = propagateTypes(ar, Type::Variant);
  m_implementedType = Type::Variant;
  return ret; // so not to lose values
}

ExpressionPtr ArrayElementExpression::unneeded() {
  if (m_global) {
    if (m_offset) return m_offset->unneeded();
  }
  return Expression::unneeded();
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void ArrayElementExpression::outputPHP(CodeGenerator &cg,
                                       AnalysisResultPtr ar) {
  if (Option::ConvertSuperGlobals && m_global && !m_dynamicGlobal &&
      getScope() && (getScope()->is(BlockScope::ProgramScope) ||
                     getScope()-> getVariables()->
                     isConvertibleSuperGlobal(m_globalName))) {
    cg_printf("$%s", m_globalName.c_str());
  } else {
    m_variable->outputPHP(cg, ar);
    cg_printf("[");
    if (m_offset) m_offset->outputPHP(cg, ar);
    cg_printf("]");
  }
}

bool ArrayElementExpression::preOutputCPP(CodeGenerator &cg,
                                          AnalysisResultPtr ar, int state) {
  return preOutputOffsetLHS(cg, ar, state);
}

void ArrayElementExpression::outputCPPImpl(CodeGenerator &cg,
                                           AnalysisResultPtr ar) {
  if (m_global) {
    if (!m_globalName.empty()) {
      VariableTablePtr variables = getScope()->getVariables();
      string name = variables->getGlobalVariableName(cg, ar, m_globalName);
      cg_printf("g->%s", name.c_str());
    } else {
      cg_printf("((LVariableTable *)g)->get(");
      m_offset->outputCPP(cg, ar);
      cg_printf(")");
    }
  } else {
    TypePtr type = m_variable->getActualType();
    if (hasContext(UnsetContext)) {
      cg_printf("unsetLval(");
      m_variable->outputCPP(cg, ar);
      cg_printf(", ");
    } else {
      if (m_variable->is(Expression::KindOfScalarExpression) ||
          (type && (type->isInteger() ||
                    type->is(Type::KindOfDouble) ||
                    type->is(Type::KindOfObject) ||
                    type->is(Type::KindOfBoolean)))) {
        cg_printf(type && type->is(Type::KindOfString) ? "((String)" :
                  "((Variant)");
        m_variable->outputCPP(cg, ar);
        cg_printf(")");
      } else {
        TypePtr act;
        if (!m_variable->hasCPPTemp() && m_variable->getImplementedType() &&
            type->is(Type::KindOfArray) &&
            !Type::SameType(m_variable->getImplementedType(), type)) {
          act = type;
          type = m_variable->getImplementedType();
          m_variable->setActualType(m_variable->getImplementedType());
        }
        m_variable->outputCPP(cg, ar);
        if (act) {
          m_variable->setActualType(act);
        }
      }
    }
    if (m_offset) {
      bool lvalAt = false;
      bool rvalAt = false;
      bool byRef = false;
      bool arrRef = false;
      const char *sep = ", AccessFlags::";
      if (hasContext(UnsetContext)) {
        // do nothing
      } else if (hasContext(InvokeArgument) && cg.callInfoTop() != -1) {
        cg_printf(".argvalAt(cit%d->isRef(%d), ", cg.callInfoTop(), m_argNum);
      } else if (m_context & (LValue|RefValue|DeepReference)) {
        cg_printf(".lvalAt(");
        lvalAt = true;
      } else {
        byRef = (m_context & AccessContext) &&
          (!type || !type->is(Type::KindOfString));
        arrRef = byRef && type && type->is(Type::KindOfArray);
        cg_printf(".rval%s%s(",
                  arrRef || !byRef ? "At" : "", byRef ? "Ref" : "");
        rvalAt = true;
      }
      m_offset->outputCPP(cg, ar);
      if (!type || !type->is(Type::KindOfString)) {
        if (rvalAt) {
          if (byRef && !arrRef) {
            const string &tmp = cg.getReferenceTemp();
            cg_printf(", %s", tmp.empty() ? "Variant()" : tmp.c_str());
          }
          if (!hasContext(ExistContext)) {
            cg_printf(", AccessFlags::Error"); // raise undefined index error
            sep = "_";
          }
        } else if (lvalAt) {
          if (hasContext(AccessContext)) {
            // Dont copy the array if the element is an object, or
            // is referenced.
            // This is safe in AccessContext (the parent is an ArrayElement,
            // or an ObjectProperty) because applying [] to an object will
            // either invoke OffsetGet, or fatal, and modifications to a
            // referenced element would be reflected in all copies
            // of the array anyway.
            cg_printf(", AccessFlags::CheckExist");
            sep = "_";
          }
        }
        ScalarExpressionPtr sc =
          dynamic_pointer_cast<ScalarExpression>(m_offset);
        if (!hasContext(UnsetContext) && sc && sc->isLiteralString()) {
          String s(sc->getLiteralString());
          int64 n;
          if (!s.get()->isStrictlyInteger(n)) {
            if (lvalAt || rvalAt) {
              cg_printf("%sKey", sep);
            } else {
              cg_printf(", true"); // skip toKey() at run time
            }
          }
        }
      }
      cg_printf(")");
    } else {
      cg_printf(".lvalAt()");
    }
  }
}

void ArrayElementExpression::outputCPPExistTest(CodeGenerator &cg,
                                                AnalysisResultPtr ar, int op) {
  switch (op) {
  case T_ISSET:  cg_printf("isset("); break;
  case T_EMPTY:  cg_printf("empty("); break;
  default: ASSERT(false);
  }

  if (m_global) {
    if (!m_globalName.empty()) {
      VariableTablePtr variables = getScope()->getVariables();
      string name = variables->getGlobalVariableName(cg, ar, m_globalName);
      cg_printf("g->%s", name.c_str());
    } else {
      cg_printf("((LVariableTable *)g)->get(");
      m_offset->outputCPP(cg, ar);
      cg_printf(")");
    }
  } else {
    m_variable->outputCPP(cg, ar);
    cg_printf(", ");
    m_offset->outputCPP(cg, ar);
    ScalarExpressionPtr sc =
      dynamic_pointer_cast<ScalarExpression>(m_offset);
    if (sc) {
      if (sc->isLiteralString()) {
        String s(sc->getLiteralString());
        int64 n;
        if (!s.get()->isStrictlyInteger(n)) {
          cg_printf(", true"); // skip toKey() at run time
        }
      }
    }
  }
  cg_printf(")");
}
void ArrayElementExpression::outputCPPUnset(CodeGenerator &cg,
                                            AnalysisResultPtr ar) {
  if (isSuperGlobal()) {
    Expression::outputCPPUnset(cg, ar);
  } else {
    TypePtr expected = m_variable->getExpectedType();
    TypePtr implemented = m_variable->getImplementedType();
    bool wrap = false;
    if (TypePtr t = m_variable->getActualType()) {
      if (t->is(Type::KindOfObject)) {
        if (!m_variable->getImplementedType() ||
            !m_variable->getImplementedType()->is(Type::KindOfVariant)) {
          cg_printf("((Variant)(");
          wrap = true;
        }
        m_variable->setImplementedType(TypePtr());
        m_variable->setExpectedType(TypePtr());
      }
    }
    m_variable->outputCPP(cg, ar);
    if (wrap) cg_printf("))");
    m_variable->setExpectedType(expected);
    m_variable->setImplementedType(implemented);
    cg_printf(".weakRemove(");
    m_offset->outputCPP(cg, ar);
    ScalarExpressionPtr sc =
      dynamic_pointer_cast<ScalarExpression>(m_offset);
    if (sc && sc->isLiteralString()) {
      String s(sc->getLiteralString());
      int64 n;
      if (!s->isStrictlyInteger(n)) {
        cg_printf(", true");
      }
    }
    cg_printf(")");
  }
}
