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

#include <lib/expression/list_assignment.h>
#include <lib/expression/assignment_expression.h>
#include <lib/expression/expression_list.h>
#include <lib/analysis/file_scope.h>
#include <lib/analysis/function_scope.h>

using namespace HPHP;
using namespace std;
using namespace boost;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

ListAssignment::ListAssignment
(EXPRESSION_CONSTRUCTOR_PARAMETERS,
 ExpressionListPtr variables, ExpressionPtr array)
  : Expression(EXPRESSION_CONSTRUCTOR_PARAMETER_VALUES),
    m_variables(variables), m_array(array), m_sublist(false) {
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
          sublist->setSublist();
        } else {
          exp->setContext(Expression::LValue);
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
  if (!m_sublist && ar->isFirstPass()) {
    createLambda(ar);
  }
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
      return ConstructPtr();
  }
  ASSERT(0);
}

int ListAssignment::getKidCount() const {
  return 2;
}

int ListAssignment::setNthKid(int n, ConstructPtr cp) {
  switch (n) {
    case 0:
      m_variables = boost::dynamic_pointer_cast<ExpressionList>(cp);
      return 1;
    case 1:
      m_array = boost::dynamic_pointer_cast<Expression>(cp);
      return 1;
    default:
      return 0;
  }
  ASSERT(0);
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
          AssignmentExpression::inferTypesImpl(ar, Type::Variant, true, exp);
        }
      }
    }
  }

  if (!m_array) return TypePtr();
  return m_array->inferAndCheck(ar, NEW_TYPE(Some), true);
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

void ListAssignment::outputCPPLambdaBody(CodeGenerator &cg,
                                         AnalysisResultPtr ar,
                                         int &tempCount, int &argCount) {
  if (!m_variables) return;
  int entryTemp = tempCount;
  for (int i = 0; i < m_variables->getCount(); i++) {
    ExpressionPtr exp = (*m_variables)[i];
    if (exp) {
      if (exp->is(Expression::KindOfListAssignment)) {
        int tmp = ++tempCount;
        cg.printf("Variant a%d = a%d.rvalAt(%d);\n", tmp, entryTemp, i);
        ListAssignmentPtr sublist = dynamic_pointer_cast<ListAssignment>(exp);
        sublist->outputCPPLambdaBody(cg, ar, tempCount, argCount);
      } else {
        cg.printf("p%d = a%d.rvalAt(%d);\n", argCount++, entryTemp, i);
      }
    }
  }
}

void ListAssignment::createLambda(AnalysisResultPtr ar) {
  ostringstream args, body;
  CodeGenerator cg(&body, CodeGenerator::ClusterCPP);
  int tc = 0;
  int ac = 0;
  cg.printf("Variant a0; if (aa.is(KindOfArray)) a0 = aa;\n");
  outputCPPLambdaBody(cg, ar, tc, ac);
  cg.printf("return aa;\n");
  args << "CVarRef aa";
  for (int i = 0; i < ac; i++) {
    args << ", Variant &p" << i;
  }
  m_lambda = ar->getFileScope()->addLambda("Variant", args.str(), body.str());
}

void ListAssignment::outputCPPAssignment(CodeGenerator &cg,
                                         AnalysisResultPtr ar) {
  if (!m_variables) return;

  for (int i = 0; i < m_variables->getCount(); i++) {
    ExpressionPtr exp = (*m_variables)[i];
    if (exp) {
      if (exp->is(Expression::KindOfListAssignment)) {
        ListAssignmentPtr sublist = dynamic_pointer_cast<ListAssignment>(exp);
        sublist->outputCPPAssignment(cg, ar);
      } else {
        cg.printf(", ");
        exp->outputCPP(cg, ar);
      }
    }
  }
}

void ListAssignment::outputCPPImpl(CodeGenerator &cg, AnalysisResultPtr ar) {
  ASSERT(m_array);

  cg.printf("%s(", m_lambda.c_str());
  m_array->outputCPP(cg, ar);
  outputCPPAssignment(cg, ar);
  cg.printf(")");
}
