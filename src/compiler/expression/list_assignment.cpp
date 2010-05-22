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

#include <compiler/expression/list_assignment.h>
#include <compiler/expression/assignment_expression.h>
#include <compiler/expression/expression_list.h>
#include <compiler/analysis/file_scope.h>
#include <compiler/analysis/function_scope.h>
#include <compiler/expression/array_element_expression.h>

using namespace HPHP;
using namespace std;
using namespace boost;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

ListAssignment::ListAssignment
(EXPRESSION_CONSTRUCTOR_PARAMETERS,
 ExpressionListPtr variables, ExpressionPtr array)
  : Expression(EXPRESSION_CONSTRUCTOR_PARAMETER_VALUES),
    m_variables(variables), m_array(array) {
  setLValue();
}

ExpressionPtr ListAssignment::clone() {
  ListAssignmentPtr exp(new ListAssignment(*this));
  Expression::deepCopy(exp);
  exp->m_variables = Clone(m_variables);
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
  FunctionScopePtr func = ar->getFunctionScope();
  if (func) func->disableInline();
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

ExpressionPtr ListAssignment::preOptimize(AnalysisResultPtr ar) {
  ar->preOptimize(m_variables);
  ar->preOptimize(m_array);
  return ExpressionPtr();
}

ExpressionPtr ListAssignment::postOptimize(AnalysisResultPtr ar) {
  ar->postOptimize(m_variables);
  ar->postOptimize(m_array);
  return ExpressionPtr();
}

TypePtr ListAssignment::inferTypes(AnalysisResultPtr ar, TypePtr type,
                                   bool coerce) {
  if (m_variables) {
    for (int i = 0; i < m_variables->getCount(); i++) {
      ExpressionPtr exp = (*m_variables)[i];
      if (exp) {
        if (exp->is(Expression::KindOfListAssignment)) {
          exp->inferAndCheck(ar, NEW_TYPE(Any), false);
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
  cg.printf("list(");
  if (m_variables) m_variables->outputPHP(cg, ar);
  if (m_array) {
    cg.printf(") = ");
    m_array->outputPHP(cg, ar);
  } else {
    cg.printf(")");
  }
}

void ListAssignment::outputCPPAssignment(CodeGenerator &cg,
    AnalysisResultPtr ar, const string &arrTmp) {
  if (!m_variables) return;

  for (int i = m_variables->getCount() - 1; i >= 0; --i) {
    ExpressionPtr exp = (*m_variables)[i];
    if (exp) {
      if (exp->is(Expression::KindOfListAssignment)) {
        ListAssignmentPtr sublist = dynamic_pointer_cast<ListAssignment>(exp);
        string subTmp = genCPPTemp(cg, ar);
        cg.printf("Variant %s((ref(%s.rvalAt(%d))));\n", subTmp.c_str(),
                  arrTmp.c_str(), i);
        sublist->outputCPPAssignment(cg, ar, subTmp);
      } else {
        bool done = false;
        if (exp->is(Expression::KindOfArrayElementExpression)) {
          ArrayElementExpressionPtr arrExp =
            dynamic_pointer_cast<ArrayElementExpression>(exp);
          if (!arrExp->isSuperGlobal() && !arrExp->isDynamicGlobal()) {
            arrExp->getVariable()->outputCPP(cg, ar);
            if (arrExp->getOffset()) {
              cg.printf(".set(");
              arrExp->getOffset()->outputCPP(cg, ar);
              cg.printf(", ");
            } else {
              cg.printf(".append(");
            }
            cg.printf("%s.rvalAt(%d));\n", arrTmp.c_str(), i);
            done = true;
          }
        }
        if (!done) {
          exp->outputCPP(cg, ar);
          if (arrTmp == "null") {
            cg.printf(" = null;\n");
          } else {
            cg.printf(" = %s.rvalAt(%d);\n", arrTmp.c_str(), i);
          }
        }
      }
    }
  }
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

  if (!ar->inExpression()) return true;

  if (m_variables) {
    preOutputVariables(cg, ar, m_variables->hasEffect() ||
                       m_array->hasEffect() ? FixOrder : 0);
  }
  m_array->preOutputCPP(cg, ar, 0);

  bool isArray = false, notArray = false;
  if (TypePtr type = m_array->getActualType()) {
    isArray = type->is(Type::KindOfArray);
    notArray = type->isPrimitive() ||
      type->is(Type::KindOfString) ||
      type->is(Type::KindOfObject);
  }
  m_cppTemp = genCPPTemp(cg, ar);
  ar->wrapExpressionBegin(cg);
  cg.printf("CVarRef %s((", m_cppTemp.c_str());
  m_array->outputCPP(cg, ar);
  cg.printf("));\n");
  std::string tmp;
  if (notArray) {
    tmp = "null";
  } else {
    tmp = genCPPTemp(cg, ar);
    cg.printf("Variant %s((ref(%s)));\n", tmp.c_str(), m_cppTemp.c_str());
    if (!isArray) {
      cg.printf("if (!f_is_array(%s)) %s.unset();\n",tmp.c_str(),
                tmp.c_str());
    }
  }
  outputCPPAssignment(cg, ar, tmp);

  return true;
}

void ListAssignment::outputCPPImpl(CodeGenerator &cg, AnalysisResultPtr ar) {
  assert(false);
}
