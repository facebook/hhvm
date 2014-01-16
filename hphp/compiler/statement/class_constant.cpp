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

#include "hphp/compiler/statement/class_constant.h"
#include "hphp/compiler/analysis/analysis_result.h"
#include "hphp/compiler/expression/expression_list.h"
#include "hphp/compiler/expression/constant_expression.h"
#include "hphp/compiler/analysis/class_scope.h"
#include "hphp/compiler/analysis/constant_table.h"
#include "hphp/compiler/expression/assignment_expression.h"
#include "hphp/compiler/expression/scalar_expression.h"
#include "hphp/compiler/option.h"

using namespace HPHP;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

ClassConstant::ClassConstant
(STATEMENT_CONSTRUCTOR_PARAMETERS, std::string typeConstraint,
  ExpressionListPtr exp)
  : Statement(STATEMENT_CONSTRUCTOR_PARAMETER_VALUES(ClassConstant)),
    m_typeConstraint(typeConstraint), m_exp(exp) {
}

StatementPtr ClassConstant::clone() {
  ClassConstantPtr stmt(new ClassConstant(*this));
  stmt->m_exp = Clone(m_exp);
  return stmt;
}

///////////////////////////////////////////////////////////////////////////////
// parser functions

void ClassConstant::onParseRecur(AnalysisResultConstPtr ar,
                                 ClassScopePtr scope) {
  ConstantTablePtr constants = scope->getConstants();

  if (scope->isTrait()) {
    parseTimeFatal(Compiler::InvalidTraitStatement,
                   "Traits cannot have constants");
  }

  for (int i = 0; i < m_exp->getCount(); i++) {
    AssignmentExpressionPtr assignment =
      dynamic_pointer_cast<AssignmentExpression>((*m_exp)[i]);

    ExpressionPtr var = assignment->getVariable();
    const std::string &name =
      dynamic_pointer_cast<ConstantExpression>(var)->getName();
    if (constants->isPresent(name)) {
      assignment->parseTimeFatal(Compiler::DeclaredConstantTwice,
                                 "Cannot redeclare %s::%s",
                                 scope->getOriginalName().c_str(),
                                 name.c_str());
    } else {
      assignment->onParseRecur(ar, scope);
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// static analysis functions

void ClassConstant::analyzeProgram(AnalysisResultPtr ar) {
  m_exp->analyzeProgram(ar);
}

ConstructPtr ClassConstant::getNthKid(int n) const {
  switch (n) {
    case 0:
      return m_exp;
    default:
      assert(false);
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
      m_exp = dynamic_pointer_cast<ExpressionList>(cp);
      break;
    default:
      assert(false);
      break;
  }
}

StatementPtr ClassConstant::preOptimize(AnalysisResultConstPtr ar) {
  for (int i = 0; i < m_exp->getCount(); i++) {
    AssignmentExpressionPtr assignment =
      dynamic_pointer_cast<AssignmentExpression>((*m_exp)[i]);

    ExpressionPtr var = assignment->getVariable();
    ExpressionPtr val = assignment->getValue();

    const std::string &name =
      dynamic_pointer_cast<ConstantExpression>(var)->getName();

    Symbol *sym = getScope()->getConstants()->getSymbol(name);
    Lock lock(BlockScope::s_constMutex);
    if (sym->getValue() != val) {
      getScope()->addUpdates(BlockScope::UseKindConstRef);
      sym->setValue(val);
    }
  }
  return StatementPtr();
}

void ClassConstant::inferTypes(AnalysisResultPtr ar) {
  m_exp->inferAndCheck(ar, Type::Some, false);
}

///////////////////////////////////////////////////////////////////////////////

void ClassConstant::outputCodeModel(CodeGenerator &cg) {
  cg.printObjectHeader("ConstantStatement", 2);
  cg.printPropertyHeader("expressions");
  cg.printExpressionVector(m_exp);
  cg.printPropertyHeader("sourceLocation");
  cg.printLocation(this->getLocation());
  cg.printObjectFooter();
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void ClassConstant::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  cg_printf("const ");
  m_exp->outputPHP(cg, ar);
  cg_printf(";\n");
}

