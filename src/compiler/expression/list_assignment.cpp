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

#include <compiler/expression/list_assignment.h>
#include <compiler/expression/assignment_expression.h>
#include <compiler/expression/expression_list.h>
#include <compiler/analysis/file_scope.h>
#include <compiler/analysis/function_scope.h>
#include <compiler/expression/array_element_expression.h>
#include <compiler/expression/object_property_expression.h>
#include <compiler/expression/unary_op_expression.h>
#include <compiler/expression/binary_op_expression.h>
#include <util/parser/hphp.tab.hpp>

using namespace HPHP;
using namespace std;
using namespace boost;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

/*
  Determine whether the rhs behaves normall, or abnormally.

  1) If the expression is the silence operator, recurse on the inner expression.
  2) If the expression is a list assignment expression, recurse on the
     RHS of the expression.
  3) If the expression is one of the following, then E behaves normally:
  Simple/Dynamic variable (including $this and superglobals)
  Array element expression
  Property expression
  Static variable expression
  Function call expression
  Preinc/predec expression (but not postinc/postdec)
  Assignment expression
  Assignment op expression
  Binding assignment expression
  Include/require expression
  Eval expression
  Array expression
  Array cast expression
  4) For all other expressions, E behaves abnormally. This includes:
  All binary operator expressions
  All unary operator expressions except silence and preinc/predec
  Scalar expression of type null, bool, int, double, or string
  Qop expression (?:)
  Constant expression
  Class constant expression
  Isset or empty expression
  Exit expression
  Instanceof expression
*/
static ListAssignment::RHSKind GetRHSKind(ExpressionPtr rhs) {
  switch (rhs->getKindOf()) {
    case Expression::KindOfSimpleVariable:
    case Expression::KindOfDynamicVariable:
    case Expression::KindOfArrayElementExpression:
    case Expression::KindOfObjectPropertyExpression:
    case Expression::KindOfSimpleFunctionCall:
    case Expression::KindOfDynamicFunctionCall:
    case Expression::KindOfObjectMethodExpression:
    case Expression::KindOfNewObjectExpression:
    case Expression::KindOfAssignmentExpression:
    case Expression::KindOfIncludeExpression:
      return ListAssignment::Regular;

    case Expression::KindOfListAssignment:
      return GetRHSKind(static_pointer_cast<ListAssignment>(rhs)->getArray());

    case Expression::KindOfUnaryOpExpression: {
      UnaryOpExpressionPtr u(static_pointer_cast<UnaryOpExpression>(rhs));
      switch (u->getOp()) {
        case '@':
          return GetRHSKind(u->getExpression());
        case T_INC:
        case T_DEC:
          return u->getFront() ?
            ListAssignment::Regular : ListAssignment::Checked;
        case T_EVAL:
        case T_ARRAY:
        case T_ARRAY_CAST:
          return ListAssignment::Regular;
        default:
          return ListAssignment::Null;
      }
      break;
    }

    case Expression::KindOfBinaryOpExpression: {
      BinaryOpExpressionPtr b(static_pointer_cast<BinaryOpExpression>(rhs));
      return b->isAssignmentOp() || b->getOp() == '+' ?
        ListAssignment::Regular : ListAssignment::Null;
    }
    case Expression::KindOfQOpExpression:
      return ListAssignment::Checked;

    default: break;
  }
  return ListAssignment::Null;
}

ListAssignment::ListAssignment
(EXPRESSION_CONSTRUCTOR_PARAMETERS,
 ExpressionListPtr variables, ExpressionPtr array)
  : Expression(EXPRESSION_CONSTRUCTOR_PARAMETER_VALUES),
    m_variables(variables), m_array(array), m_rhsKind(Regular) {
  setLValue();

  if (m_array) {
    m_rhsKind = GetRHSKind(m_array);
  }
}

ExpressionPtr ListAssignment::clone() {
  ListAssignmentPtr exp(new ListAssignment(*this));
  Expression::deepCopy(exp);
  exp->m_variables = Clone(m_variables);
  exp->m_array = Clone(m_array);
  return exp;
}

void ListAssignment::setLValue() {
  if (m_variables) {
    for (int i = 0; i < m_variables->getCount(); i++) {
      ExpressionPtr exp = (*m_variables)[i];
      if (exp) {
        if (exp->is(Expression::KindOfListAssignment)) {
          ListAssignmentPtr sublist =
            dynamic_pointer_cast<ListAssignment>(exp);
          sublist->setLValue();
        } else {
          // Magic contexts I took from assignment expression
          exp->setContext(Expression::DeepAssignmentLHS);
          exp->setContext(Expression::AssignmentLHS);
          exp->setContext(Expression::LValue);
          exp->setContext(Expression::NoLValueWrapper);
        }
      }
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// parser functions

///////////////////////////////////////////////////////////////////////////////
// static analysis functions

void ListAssignment::analyzeProgram(AnalysisResultPtr ar) {
  if (m_variables) m_variables->analyzeProgram(ar);
  if (m_array) m_array->analyzeProgram(ar);
  FunctionScopePtr func = getFunctionScope();
  if (func) func->disableInline();
  if (ar->getPhase() == AnalysisResult::AnalyzeFinal) {
    if (m_variables) {
      for (int i = 0; i < m_variables->getCount(); i++) {
        ExpressionPtr exp = (*m_variables)[i];
        if (exp) {
          if (!exp->is(Expression::KindOfListAssignment)) {
            CheckNeeded(exp, ExpressionPtr());
          }
        }
      }
    }
  }
}

ConstructPtr ListAssignment::getNthKid(int n) const {
  switch (n) {
    case 0:
      return m_variables;
    case 1:
      return m_array;
    default:
      ASSERT(false);
      break;
  }
  return ConstructPtr();
}

int ListAssignment::getKidCount() const {
  return 2;
}

void ListAssignment::setNthKid(int n, ConstructPtr cp) {
  switch (n) {
    case 0:
      m_variables = boost::dynamic_pointer_cast<ExpressionList>(cp);
      break;
    case 1:
      m_array = boost::dynamic_pointer_cast<Expression>(cp);
      break;
    default:
      ASSERT(false);
      break;
  }
}

TypePtr ListAssignment::inferTypes(AnalysisResultPtr ar, TypePtr type,
                                   bool coerce) {
  if (m_variables) {
    for (int i = 0; i < m_variables->getCount(); i++) {
      ExpressionPtr exp = (*m_variables)[i];
      if (exp) {
        if (exp->is(Expression::KindOfListAssignment)) {
          exp->inferAndCheck(ar, Type::Any, false);
        } else {
          inferAssignmentTypes(ar, Type::Variant, true, exp);
        }
      }
    }
  }

  if (!m_array) return TypePtr();
  return m_array->inferAndCheck(ar, Type::Variant, false);
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void ListAssignment::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  cg_printf("list(");
  if (m_variables) m_variables->outputPHP(cg, ar);
  if (m_array) {
    cg_printf(") = ");
    m_array->outputPHP(cg, ar);
  } else {
    cg_printf(")");
  }
}

static string getArrRef(const string &arrTmp, int index) {
  if (arrTmp == "null_variant") return arrTmp;
  return arrTmp + "[" + lexical_cast<string>(index) + "]";
}

bool ListAssignment::outputCPPAssignment(CodeGenerator &cg,
                                         AnalysisResultPtr ar,
                                         const string &arrTmp, bool subRef) {
  if (!m_variables) return false;

  bool ret = false;
  for (int i = m_variables->getCount() - 1; i >= 0; --i) {
    ExpressionPtr exp = (*m_variables)[i];
    if (exp) {
      if (exp->is(Expression::KindOfListAssignment)) {
        ListAssignmentPtr sublist = dynamic_pointer_cast<ListAssignment>(exp);
        string subTmp;
        if (arrTmp == "null_variant") {
          subTmp = arrTmp;
        } else {
          subTmp = genCPPTemp(cg, ar);
          cg_printf("Variant %s((%s(%s[%d])));\n",
                    subTmp.c_str(), subRef ? "ref" : "",
                    arrTmp.c_str(), i);
        }
        if (sublist->outputCPPAssignment(cg, ar, subTmp, subRef)) ret = true;
      } else {
        ret = true;
        bool done = false;
        if (exp->is(Expression::KindOfArrayElementExpression)) {
          ArrayElementExpressionPtr arrExp =
            dynamic_pointer_cast<ArrayElementExpression>(exp);
          if (!arrExp->isSuperGlobal() && !arrExp->isDynamicGlobal()) {
            arrExp->getVariable()->outputCPP(cg, ar);
            if (arrExp->getOffset()) {
              cg_printf(".set(");
              arrExp->getOffset()->outputCPP(cg, ar);
              cg_printf(", ");
            } else {
              cg_printf(".append(");
            }
            cg_printf("%s);\n", getArrRef(arrTmp, i).c_str());
            done = true;
          }
        } else if (exp->is(Expression::KindOfObjectPropertyExpression)) {
          ObjectPropertyExpressionPtr var(
            dynamic_pointer_cast<ObjectPropertyExpression>(exp));
          if (!var->isValid()) {
            var->outputCPPObject(cg, ar);
            cg_printf("o_set(");
            var->outputCPPProperty(cg, ar);
            cg_printf(", %s, %s);\n",
                      getArrRef(arrTmp, i).c_str(),
                      getClassScope() ? "s_class_name" : "empty_string");
            done = true;
          }
        }
        if (!done) {
          exp->outputCPP(cg, ar);
          cg_printf(" = %s;\n", getArrRef(arrTmp, i).c_str());
        }
      }
    }
  }
  return ret;
}

void ListAssignment::preOutputVariables(CodeGenerator &cg,
                                        AnalysisResultPtr ar,
                                        int state) {
  for (int i = 0, c = m_variables->getCount(); i < c; ) {
    if (ExpressionPtr exp = (*m_variables)[i++]) {
      if (i == c) {
        state = 0;
      }
      if (exp->is(Expression::KindOfListAssignment)) {
        static_pointer_cast<ListAssignment>(exp)->
          preOutputVariables(cg, ar, state);
      } else {
        exp->preOutputCPP(cg, ar, state);
      }
    }
  }
}

bool ListAssignment::preOutputCPP(CodeGenerator &cg, AnalysisResultPtr ar,
                                  int state) {
  ASSERT(m_array);

  if (!cg.inExpression()) return true;

  if (m_variables) {
    preOutputVariables(cg, ar, m_variables->hasEffect() ||
                       m_array->hasEffect() ? FixOrder : 0);
  }
  m_array->preOutputCPP(cg, ar, 0);

  bool isArray = false, notArray = false;
  bool simpleVar = m_array->is(KindOfSimpleVariable) &&
    !static_pointer_cast<SimpleVariable>(m_array)->getAlwaysStash();
  bool needsUse = false;
  if (TypePtr type = m_array->getActualType()) {
    isArray = type->is(Type::KindOfArray);
    notArray = !isArray &&
      (type->isPrimitive() ||
       m_rhsKind == Null ||
      (m_rhsKind == Checked && (type->is(Type::KindOfString) ||
                                type->is(Type::KindOfObject))));
  }
  cg.wrapExpressionBegin();
  if (outputLineMap(cg, ar)) cg_printf("0);\n");
  if (notArray && isUnused()) {
    if (m_array->outputCPPUnneeded(cg, ar)) cg_printf(";\n");
    m_cppTemp = "null";
  } else {
    m_cppTemp = genCPPTemp(cg, ar);
    needsUse = simpleVar || m_array->isTemporary();
    const char *decl;
    if (isArray && m_array->getCPPType()->is(Type::KindOfArray)) {
      decl = needsUse ? "CArrRef" : "Array";
    } else {
      decl = needsUse ? "CVarRef" : "Variant";
    }
    cg_printf("%s %s((", decl, m_cppTemp.c_str());
    m_array->outputCPP(cg, ar);
    cg_printf("));\n");
  }
  std::string tmp;
  if (notArray) {
    tmp = "null_variant";
    if (needsUse) cg_printf("id(%s);\n", m_cppTemp.c_str());
    needsUse = false;
  } else if (m_rhsKind != Checked || isArray) {
    tmp = m_cppTemp;
  } else {
    tmp = genCPPTemp(cg, ar);
    cg_printf("CVarRef %s(f_is_array(%s)?%s:null_variant);\n",
              tmp.c_str(), m_cppTemp.c_str(), m_cppTemp.c_str());
    needsUse = true;
  }
  if (!outputCPPAssignment(cg, ar, tmp, simpleVar) && needsUse) {
    cg_printf("id(%s);\n", tmp.c_str());
  }

  return true;
}

void ListAssignment::outputCPPImpl(CodeGenerator &cg, AnalysisResultPtr ar) {
  assert(false);
}
