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

#include <compiler/statement/class_constant.h>
#include <compiler/analysis/analysis_result.h>
#include <compiler/expression/expression_list.h>
#include <compiler/expression/constant_expression.h>
#include <compiler/analysis/class_scope.h>
#include <compiler/analysis/constant_table.h>
#include <compiler/expression/assignment_expression.h>
#include <compiler/option.h>

using namespace HPHP;
using namespace std;
using namespace boost;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

ClassConstant::ClassConstant
(STATEMENT_CONSTRUCTOR_PARAMETERS, ExpressionListPtr exp)
  : Statement(STATEMENT_CONSTRUCTOR_PARAMETER_VALUES), m_exp(exp) {
}

StatementPtr ClassConstant::clone() {
  ClassConstantPtr stmt(new ClassConstant(*this));
  stmt->m_exp = Clone(m_exp);
  return stmt;
}

///////////////////////////////////////////////////////////////////////////////
// parser functions

void ClassConstant::onParse(AnalysisResultPtr ar) {
  for (int i = 0; i < m_exp->getCount(); i++) {
    IParseHandlerPtr ph = dynamic_pointer_cast<IParseHandler>((*m_exp)[i]);
    ph->onParse(ar);
  }
}

///////////////////////////////////////////////////////////////////////////////
// static analysis functions

void ClassConstant::analyzeProgramImpl(AnalysisResultPtr ar) {
  m_exp->analyzeProgram(ar);
}

ConstructPtr ClassConstant::getNthKid(int n) const {
  switch (n) {
    case 0:
      return m_exp;
    default:
      ASSERT(false);
      break;
  }
  return ConstructPtr();
}

int ClassConstant::getKidCount() const {
  return 1;
}

void ClassConstant::setNthKid(int n, ConstructPtr cp) {
  switch (n) {
    case 0:
      m_exp = boost::dynamic_pointer_cast<ExpressionList>(cp);
      break;
    default:
      ASSERT(false);
      break;
  }
}

StatementPtr ClassConstant::preOptimize(AnalysisResultPtr ar) {
  ar->preOptimize(m_exp);
  return StatementPtr();
}

StatementPtr ClassConstant::postOptimize(AnalysisResultPtr ar) {
  ar->postOptimize(m_exp);
  return StatementPtr();
}

void ClassConstant::inferTypes(AnalysisResultPtr ar) {
  m_exp->inferAndCheck(ar, NEW_TYPE(Some), false);
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void ClassConstant::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  cg.printf("const ");
  m_exp->outputPHP(cg, ar);
  cg.printf(";\n");
}

void ClassConstant::outputCPPImpl(CodeGenerator &cg, AnalysisResultPtr ar) {
  bool lazyInit = cg.getContext() == CodeGenerator::CppLazyStaticInitializer;
  if (cg.getContext() != CodeGenerator::CppClassConstantsDecl &&
      cg.getContext() != CodeGenerator::CppClassConstantsImpl &&
      !lazyInit) {
    return;
  }

  ClassScopePtr scope = ar->getClassScope();
  for (int i = 0; i < m_exp->getCount(); i++) {
    AssignmentExpressionPtr exp =
      dynamic_pointer_cast<AssignmentExpression>((*m_exp)[i]);
    ConstantExpressionPtr var =
      dynamic_pointer_cast<ConstantExpression>(exp->getVariable());
    TypePtr type = scope->getConstants()->getFinalType(var->getName());
    ExpressionPtr value = exp->getValue();
    if (!scope->getConstants()->isDynamic(var->getName()) == lazyInit) {
        continue;
    }
    switch (cg.getContext()) {
    case CodeGenerator::CppClassConstantsDecl:
      cg.printf("extern const ");
      if (type->is(Type::KindOfString)) {
        cg.printf("StaticString");
      } else {
        type->outputCPPDecl(cg, ar);
      }
      cg.printf(" %s%s_%s;\n", Option::ClassConstantPrefix,
                scope->getId().c_str(),
                var->getName().c_str());
      break;
    case CodeGenerator::CppClassConstantsImpl:
      cg.printf("const ");
      if (type->is(Type::KindOfString)) {
        cg.printf("StaticString");
      } else {
        type->outputCPPDecl(cg, ar);
      }
      value->outputCPPBegin(cg, ar);
      cg.printf(" %s%s_%s = ", Option::ClassConstantPrefix,
                scope->getId().c_str(),
                var->getName().c_str());
      value->outputCPP(cg, ar);
      cg.printf(";\n");
      value->outputCPPEnd(cg, ar);
      break;
    case CodeGenerator::CppLazyStaticInitializer:
      value->outputCPPBegin(cg, ar);
      cg.printf("g->%s%s_%s = ", Option::ClassConstantPrefix,
                scope->getId().c_str(),
                var->getName().c_str());
      value->outputCPP(cg, ar);
      cg.printf(";\n");
      value->outputCPPEnd(cg, ar);
      break;
    default:
      ASSERT(false);
    }
  }
}
