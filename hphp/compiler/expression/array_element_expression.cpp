/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/compiler/expression/array_element_expression.h"
#include "hphp/compiler/expression/simple_variable.h"
#include "hphp/compiler/expression/scalar_expression.h"
#include "hphp/compiler/analysis/variable_table.h"
#include "hphp/compiler/analysis/code_error.h"
#include "hphp/compiler/code_model_enums.h"
#include "hphp/compiler/option.h"
#include "hphp/compiler/expression/static_member_expression.h"
#include "hphp/compiler/analysis/function_scope.h"
#include "hphp/parser/hphp.tab.hpp"
#include "hphp/runtime/base/complex-types.h"
#include "hphp/runtime/base/builtin-functions.h"

using namespace HPHP;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

ArrayElementExpression::ArrayElementExpression
(EXPRESSION_CONSTRUCTOR_PARAMETERS,
 ExpressionPtr variable, ExpressionPtr offset)
  : Expression(EXPRESSION_CONSTRUCTOR_PARAMETER_VALUES(ArrayElementExpression)),
    LocalEffectsContainer(AccessorEffect),
    m_variable(variable), m_offset(offset), m_global(false),
    m_dynamicGlobal(false) {
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
      // special case for $GLOBALS[], we do not need lvalue wrapper
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

bool ArrayElementExpression::appendClass(ExpressionPtr cls,
                                         AnalysisResultConstPtr ar,
                                         FileScopePtr file) {
  if (m_variable->is(Expression::KindOfArrayElementExpression)) {
    return dynamic_pointer_cast<ArrayElementExpression>(m_variable)
      ->appendClass(cls, ar, file);
  }
  if (m_variable->is(Expression::KindOfSimpleVariable) ||
      m_variable->is(Expression::KindOfDynamicVariable)) {
    StaticMemberExpressionPtr sme(
      new StaticMemberExpression(
        m_variable->getScope(), m_variable->getLocation(),
        cls, m_variable));
    sme->onParse(ar, file);
    m_variable = sme;
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
    } else {
      TypePtr at(m_variable->getActualType());
      TypePtr et(m_variable->getExpectedType());
      if (et &&
          (et->is(Type::KindOfSequence) ||
           et->is(Type::KindOfAutoSequence)) &&
          at && at->isExactType()) {
        // since Sequence maps to Variant in the runtime,
        // using Sequence for the expected type will
        // never allow the necessary casts to be generated.
        m_variable->setExpectedType(at);
      }
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
      assert(false);
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
      m_variable = dynamic_pointer_cast<Expression>(cp);
      break;
    case 1:
      m_offset = dynamic_pointer_cast<Expression>(cp);
      break;
    default:
      assert(false);
      break;
  }
}

bool ArrayElementExpression::canonCompare(ExpressionPtr e) const {
  return m_offset && Expression::canonCompare(e);
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
          if (v.isArray()) {
            try {
              g_context->setThrowAllErrors(true);
              Variant res = v.toArrRef().rvalAt(
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
  }
  return ExpressionPtr();
}

ExpressionPtr ArrayElementExpression::postOptimize(AnalysisResultConstPtr ar) {
  if (!hasLocalEffect(AccessorEffect)) return ExpressionPtr();
  TypePtr at(m_variable->getActualType());
  if (at && (at->is(Type::KindOfString) || at->is(Type::KindOfArray))) {
    clearLocalEffect(AccessorEffect);
    return dynamic_pointer_cast<Expression>(shared_from_this());
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
    setLocalEffect(DiagnosticEffect);
  }
  if (m_context & (AssignmentLHS|OprLValue)) {
    clearLocalEffect(AccessorEffect);
  } else if (m_context & (LValue | RefValue)) {
    setLocalEffect(CreateEffect);
  }

  // handling $GLOBALS[...]
  if (m_variable->is(Expression::KindOfSimpleVariable)) {
    SimpleVariablePtr var =
      dynamic_pointer_cast<SimpleVariable>(m_variable);
    if (var->getName() == "GLOBALS") {
      clearLocalEffect(AccessorEffect);
      m_global = true;
      m_dynamicGlobal = true;
      getScope()->getVariables()->
        setAttribute(VariableTable::NeedGlobalPointer);
      VariableTablePtr vars = ar->getVariables();

      Lock l(ar->getMutex());
      if (m_offset && m_offset->is(Expression::KindOfScalarExpression)) {
        ScalarExpressionPtr offset =
          dynamic_pointer_cast<ScalarExpression>(m_offset);

        if (offset->isLiteralString()) {
          m_globalName = offset->getIdentifier();
          if (!m_globalName.empty()) {
            m_dynamicGlobal = false;
            clearLocalEffect(DiagnosticEffect);
            getScope()->getVariables()->
              setAttribute(VariableTable::NeedGlobalPointer);
            TypePtr ret;
            if (coerce) {
              ret = vars->add(m_globalName, type, true, ar, self,
                              ModifierExpressionPtr());
            } else {
              ret = vars->checkVariable(m_globalName, type, coerce, ar, self);
            }
            getScope()->getVariables()->addSuperGlobal(m_globalName);
            return ret;
          }
        }
      } else {
        vars->setAttribute(VariableTable::ContainsDynamicVariable);
      }

      if (hasContext(LValue) || hasContext(RefValue)) {
        vars->forceVariants(ar, VariableTable::AnyVars);
        vars->setAttribute(VariableTable::ContainsLDynamicVariable);
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
    m_implementedType.reset();
    return Type::String;
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

void ArrayElementExpression::outputCodeModel(CodeGenerator &cg) {
  if (m_offset) {
    cg.printObjectHeader("BinaryOpExpression", 4);
    cg.printPropertyHeader("expression1");
    m_variable->outputCodeModel(cg);
    cg.printPropertyHeader("expression2");
    cg.printExpression(m_offset, false);
    cg.printPropertyHeader("operation");
    cg.printValue(PHP_ARRAY_ELEMENT);
    cg.printPropertyHeader("sourceLocation");
    cg.printLocation(this->getLocation());
    cg.printObjectFooter();
  } else {
    cg.printObjectHeader("UnaryOpExpression", 3);
    cg.printPropertyHeader("expression");
    m_variable->outputCodeModel(cg);
    cg.printPropertyHeader("operation");
    cg.printValue(PHP_ARRAY_APPEND_POINT_OP);
    cg.printPropertyHeader("sourceLocation");
    cg.printLocation(this->getLocation());
    cg.printObjectFooter();
  }
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
