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

#include <lib/expression/assignment_expression.h>
#include <lib/expression/array_element_expression.h>
#include <lib/analysis/code_error.h>
#include <lib/expression/constant_expression.h>
#include <lib/expression/simple_variable.h>
#include <lib/analysis/block_scope.h>
#include <lib/analysis/variable_table.h>
#include <lib/analysis/constant_table.h>
#include <lib/analysis/file_scope.h>
#include <lib/expression/unary_op_expression.h>
#include <lib/parser/hphp.tab.hpp>
#include <lib/option.h>
#include <lib/analysis/class_scope.h>
#include <lib/analysis/function_scope.h>
#include <lib/analysis/dependency_graph.h>
#include <lib/expression/scalar_expression.h>
#include <lib/expression/expression_list.h>
#include <lib/expression/simple_function_call.h>

using namespace HPHP;
using namespace std;
using namespace boost;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

AssignmentExpression::AssignmentExpression
(EXPRESSION_CONSTRUCTOR_PARAMETERS,
 ExpressionPtr variable, ExpressionPtr value, bool ref)
  : Expression(EXPRESSION_CONSTRUCTOR_PARAMETER_VALUES),
    m_variable(variable), m_value(value), m_ref(ref) {
  m_variable->setContext(Expression::DeepAssignmentLHS);
  m_variable->setContext(Expression::AssignmentLHS);
  m_variable->setContext(Expression::LValue);
  m_variable->setContext(Expression::NoLValueWrapper);
  if (ref) {
    m_value->setContext(Expression::RefValue);

    // we have &new special case that's handled in this class
    m_value->setContext(Expression::NoRefWrapper);
  }
  // The assignment expression itself must always be handled in
  // a by-value fashion even when it is passed to a function that
  // expects an argument by reference.
  setContext(Expression::NoRefWrapper);
}

ExpressionPtr AssignmentExpression::clone() {
  AssignmentExpressionPtr exp(new AssignmentExpression(*this));
  Expression::deepCopy(exp);
  exp->m_variable = Clone(m_variable);
  exp->m_value = Clone(m_value);
  return exp;
}

///////////////////////////////////////////////////////////////////////////////
// parser functions

void AssignmentExpression::onParse(AnalysisResultPtr ar) {
  BlockScopePtr scope = ar->getScope();

  // This is that much we can do during parse phase.
  TypePtr type;
  if (m_value->is(Expression::KindOfScalarExpression)) {
    type = m_value->inferAndCheck(ar, NEW_TYPE(Some), false);
  } else if (m_value->is(Expression::KindOfUnaryOpExpression)) {
    UnaryOpExpressionPtr uexp =
      dynamic_pointer_cast<UnaryOpExpression>(m_value);
    if (uexp->getOp() == T_ARRAY) {
      type = Type::Array;
    }
  }
  if (!type) type = NEW_TYPE(Some);

  if (m_variable->is(Expression::KindOfConstantExpression)) {
    // ...as in ClassConstant statement
    // We are handling this one here, not in ClassConstant, purely because
    // we need "value" to store in constant table.
    ConstantExpressionPtr exp =
      dynamic_pointer_cast<ConstantExpression>(m_variable);
    scope->getConstants()->add(exp->getName(), type, m_value, ar, m_variable);

    string name = ar->getClassScope()->getName() + "::" + exp->getName();
    ar->getDependencyGraph()->
      addParent(DependencyGraph::KindOfConstant, "", name, exp);
  } else if (m_variable->is(Expression::KindOfSimpleVariable)) {
    SimpleVariablePtr var = dynamic_pointer_cast<SimpleVariable>(m_variable);
    scope->getVariables()->add(var->getName(), type, true, ar,
                               shared_from_this(), scope->getModifiers());
    var->clearContext(Declaration); // to avoid wrong CodeError
  } else {
    ASSERT(false); // parse phase shouldn't handle anything else
  }
}

///////////////////////////////////////////////////////////////////////////////
// static analysis functions

void AssignmentExpression::analyzeProgram(AnalysisResultPtr ar) {
  m_variable->analyzeProgram(ar);
  m_value->analyzeProgram(ar);
  if (ar->getPhase() == AnalysisResult::AnalyzeAll) {
    if (m_ref && m_variable->is(Expression::KindOfSimpleVariable)) {
      SimpleVariablePtr var =
        dynamic_pointer_cast<SimpleVariable>(m_variable);
      const std::string &name = var->getName();
      VariableTablePtr variables = ar->getScope()->getVariables();
      variables->addReferenced(name);
    }
    if (m_variable->is(Expression::KindOfConstantExpression)) {
      ConstantExpressionPtr exp =
        dynamic_pointer_cast<ConstantExpression>(m_variable);
      if (!m_value->isScalar()) {
        ar->getScope()->getConstants()->setDynamic(ar, exp->getName());
      }
    }
  }
}

ConstructPtr AssignmentExpression::getNthKid(int n) const {
  switch (n) {
    case 0:
      return m_variable;
    case 1:
      return m_value;
    default:
      return ConstructPtr();
  }
  ASSERT(0);
}

int AssignmentExpression::getKidCount() const {
  return 2;
}

int AssignmentExpression::setNthKid(int n, ConstructPtr cp) {
  switch (n) {
    case 0:
      m_variable = boost::dynamic_pointer_cast<Expression>(cp);
      return 1;
    case 1:
      m_value = boost::dynamic_pointer_cast<Expression>(cp);
      return 1;
    default:
      return 0;
  }
  ASSERT(0);
}

ExpressionPtr AssignmentExpression::preOptimize(AnalysisResultPtr ar) {
  ar->preOptimize(m_variable);
  ar->preOptimize(m_value);
  return ExpressionPtr();
}

ExpressionPtr AssignmentExpression::postOptimize(AnalysisResultPtr ar) {
  ar->postOptimize(m_variable);
  ar->postOptimize(m_value);
  if (m_variable->is(Expression::KindOfSimpleVariable)) {
    SimpleVariablePtr var =
      dynamic_pointer_cast<SimpleVariable>(m_variable);
    const std::string &name = var->getName();
    VariableTablePtr variables = ar->getScope()->getVariables();
    if (!variables->isPseudoMainTable() &&
        !variables->getAttribute(VariableTable::ContainsDynamicVariable) &&
        !variables->isReferenced(name) &&
        variables->isLocal(name)) {
      variables->addUnused(name);
      if (m_value->isScalar()) {
        m_value->setExpectedType(m_expectedType);
        return m_value;
      } else {
        return makeIdCall(ar);
      }
    }
  }
  return ExpressionPtr();
}

ExpressionPtr AssignmentExpression::makeIdCall(AnalysisResultPtr ar) {
  ExpressionListPtr arg =
    ExpressionListPtr(new ExpressionList(getLocation(),
                                         Expression::KindOfExpressionList));
  arg->insertElement(m_value);
  SimpleFunctionCallPtr result =
    SimpleFunctionCallPtr(
      new SimpleFunctionCall(getLocation(),
                             Expression::KindOfSimpleFunctionCall,
                             "id", arg, NULL));
  result->setFunctionAndClassScope(ar->findHelperFunction("id"),
                                   ClassScopePtr());
  result->setValid();
  result->setNoPrefix();
  result->setActualType(m_value->getActualType());
  result->setExpectedType(m_expectedType);
  return result;
}

TypePtr AssignmentExpression::
inferTypesImpl(AnalysisResultPtr ar, TypePtr type, bool coerce,
               ExpressionPtr variable,
               ExpressionPtr value /* = ExpressionPtr() */) {
  TypePtr ret = type;
  if (value) {
    if (coerce) {
      ret = value->inferAndCheck(ar, type, coerce);
    } else {
      ret = value->inferAndCheck(ar, NEW_TYPE(Some), coerce);
    }
  }

  BlockScopePtr scope = ar->getScope();
  if (variable->is(Expression::KindOfConstantExpression)) {
    // ...as in ClassConstant statement
    ConstantExpressionPtr exp =
      dynamic_pointer_cast<ConstantExpression>(variable);
    bool p;
    scope->getConstants()->check(exp->getName(), ret, true, ar, variable, p);
  } else if (variable->is(Expression::KindOfDynamicVariable)) {
    // simptodo: not too sure about this
    ar->getFileScope()->setAttribute(FileScope::ContainsLDynamicVariable);
  } else if (variable->is(Expression::KindOfSimpleVariable)) {
    SimpleVariablePtr var = dynamic_pointer_cast<SimpleVariable>(variable);
    if (var->getName() == "this" && ar->getClassScope()) {
      if (ar->isFirstPass()) {
        ar->getCodeError()->record(variable, CodeError::ReassignThis,
                                   variable);
      }
    }
    if (ar->getPhase() == AnalysisResult::LastInference && value) {
      if (!value->getExpectedType()) {
        value->setExpectedType(variable->getActualType());
      }
    }
  }
  // if the value may involve object, consider the variable as "referenced"
  // so that objects are not destructed prematurely.
  bool referenced = true;
  if (value && value->isScalar()) referenced = false;
  if (ret && ret->isNoObjectInvolved()) referenced = false;
  if (referenced && variable->is(Expression::KindOfSimpleVariable)) {
    SimpleVariablePtr var =
      dynamic_pointer_cast<SimpleVariable>(variable);
    const std::string &name = var->getName();
    VariableTablePtr variables = ar->getScope()->getVariables();
    variables->addReferenced(name);
  }

  TypePtr vt = variable->inferAndCheck(ar, ret, true);
  if (!coerce && type->is(Type::KindOfAny)) {
    ret = vt;
  }

  return ret;
}

TypePtr AssignmentExpression::inferTypes(AnalysisResultPtr ar, TypePtr type,
                                         bool coerce) {

  if (VariableTable::m_hookHandler) {
    VariableTable::m_hookHandler(ar, ar->getScope()->getVariables().get(),
                                 m_variable,
                                 beforeAssignmentExpressionInferTypes);
  }

  TypePtr ret = inferTypesImpl(ar, type, coerce, m_variable, m_value);

  if (VariableTable::m_hookHandler) {
    VariableTable::m_hookHandler(ar, ar->getScope()->getVariables().get(),
                                 m_variable,
                                 afterAssignmentExpressionInferTypes);
  }

  return ret;
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void AssignmentExpression::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  m_variable->outputPHP(cg, ar);
  cg.printf(" = ");
  if (m_ref) cg.printf("&");
  m_value->outputPHP(cg, ar);
}

void AssignmentExpression::outputCPPImpl(CodeGenerator &cg,
                                         AnalysisResultPtr ar) {
  BlockScopePtr scope = ar->getScope();
  bool ref = (m_ref && !m_value->is(Expression::KindOfNewObjectExpression));

  bool setElement = false; // turning $a['elem'] = $b into $a.set('elem', $b);
  bool type_cast = false;
  bool setNull = false;
  TypePtr m_actualType;

  if (m_variable->is(Expression::KindOfArrayElementExpression)) {
    ArrayElementExpressionPtr exp =
      dynamic_pointer_cast<ArrayElementExpression>(m_variable);
    m_actualType = m_variable->getActualType();
    if (m_actualType && m_actualType->getKindOf() == Type::KindOfVariant
        && !ref) {
      //type_cast = true;
    }
    if (!exp->isSuperGlobal() && !exp->isDynamicGlobal()) {
      exp->getVariable()->outputCPP(cg, ar);
      if (exp->getOffset()) {
        cg.printf(".set(");
        exp->getOffset()->outputCPP(cg, ar);
        cg.printf(", (");
      } else {
        cg.printf(".append((");
      }
      if (type_cast) {
        m_actualType->outputCPPCast(cg, ar);
        cg.printf("(");
      }
      if (ref && m_value->isRefable()) cg.printf("ref(");
      m_value->outputCPP(cg, ar);
      if (ref && m_value->isRefable()) cg.printf(")");
      if (type_cast) cg.printf(")");
      cg.printf(")");
      ExpressionPtr off = exp->getOffset();
      if (off) {
        ScalarExpressionPtr sc =
          dynamic_pointer_cast<ScalarExpression>(off);
        if (sc) {
          int64 hash = sc->getHash();
          if (hash >= 0) {
            cg.printf(", 0x%016llXLL", hash);
          }
        }
      }
      cg.printf(")");
      setElement = true;
    }
  }
  if (m_variable->is(Expression::KindOfSimpleVariable) &&
      m_value->is(Expression::KindOfConstantExpression)) {
    ConstantExpressionPtr exp =
      dynamic_pointer_cast<ConstantExpression>(m_value);
    if (exp->isNull()) setNull = true;
  }

  if (!setElement) {
    if (setNull) {
      cg.printf("setNull(");
      m_variable->outputCPP(cg, ar);
    } else {
      cg.printf("(");
      m_variable->outputCPP(cg, ar);
      cg.printf(" = ");

      if (type_cast) {
        m_actualType->outputCPPCast(cg, ar);
        cg.printf("(");
      }
      if (ref && m_value->isRefable()) cg.printf("ref(");
      m_value->outputCPP(cg, ar);
      if (ref && m_value->isRefable()) cg.printf(")");
      if (type_cast) cg.printf(")");
    }
    cg.printf(")");
  }
}
