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

#include <compiler/statement/class_constant.h>
#include <compiler/analysis/analysis_result.h>
#include <compiler/expression/expression_list.h>
#include <compiler/expression/constant_expression.h>
#include <compiler/analysis/class_scope.h>
#include <compiler/analysis/constant_table.h>
#include <compiler/expression/assignment_expression.h>
#include <compiler/expression/scalar_expression.h>
#include <compiler/option.h>

using namespace HPHP;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

ClassConstant::ClassConstant
(STATEMENT_CONSTRUCTOR_PARAMETERS, ExpressionListPtr exp)
  : Statement(STATEMENT_CONSTRUCTOR_PARAMETER_VALUES(ClassConstant)),
    m_exp(exp) {
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
      m_exp = boost::dynamic_pointer_cast<ExpressionList>(cp);
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
// code generation functions

void ClassConstant::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  cg_printf("const ");
  m_exp->outputPHP(cg, ar);
  cg_printf(";\n");
}

void ClassConstant::outputCPPImpl(CodeGenerator &cg, AnalysisResultPtr ar) {
  if (cg.getContext() != CodeGenerator::CppClassConstantsDecl &&
      cg.getContext() != CodeGenerator::CppClassConstantsImpl) {
    return;
  }

  ClassScopePtr scope = getClassScope();
  for (int i = 0; i < m_exp->getCount(); i++) {
    AssignmentExpressionPtr exp =
      dynamic_pointer_cast<AssignmentExpression>((*m_exp)[i]);
    ConstantExpressionPtr var =
      dynamic_pointer_cast<ConstantExpression>(exp->getVariable());
    TypePtr type = scope->getConstants()->getFinalType(var->getName());
    ExpressionPtr value = exp->getValue();
    if (scope->getConstants()->isDynamic(var->getName())) {
        continue;
    }
    switch (cg.getContext()) {
    case CodeGenerator::CppClassConstantsDecl:
      cg_printf("extern const ");
      if (type->is(Type::KindOfString)) {
        cg_printf("StaticString");
      } else {
        type->outputCPPDecl(cg, ar, getScope());
      }
      cg_printf(" %s%s%s%s;\n",
                Option::ClassConstantPrefix, scope->getId().c_str(),
                Option::IdPrefix.c_str(), var->getName().c_str());
      break;
    case CodeGenerator::CppClassConstantsImpl: {
      bool isString = type->is(Type::KindOfString);
      bool isVariant = Type::IsMappedToVariant(type);
      ScalarExpressionPtr scalarExp =
        dynamic_pointer_cast<ScalarExpression>(value);
      bool stringForVariant = false;
      if (isVariant && scalarExp &&
          scalarExp->getActualType() &&
          scalarExp->getActualType()->is(Type::KindOfString)) {
        cg_printf("static const StaticString %s%s%s%s%sv(LITSTR_INIT(%s));\n",
                  Option::ClassConstantPrefix, scope->getId().c_str(),
                  Option::IdPrefix.c_str(), var->getName().c_str(),
                  Option::IdPrefix.c_str(),
                  scalarExp->getCPPLiteralString().c_str());
        stringForVariant = true;
      }
      cg_printf("const ");
      if (isString) {
        cg_printf("StaticString");
      } else {
        type->outputCPPDecl(cg, ar, getScope());
      }
      value->outputCPPBegin(cg, ar);
      cg_printf(" %s%s%s%s",
                Option::ClassConstantPrefix, scope->getId().c_str(),
                Option::IdPrefix.c_str(), var->getName().c_str());
      cg_printf(isString ? "(" : " = ");
      if (stringForVariant) {
        cg_printf("%s%s%s%s%sv",
                  Option::ClassConstantPrefix, scope->getId().c_str(),
                  Option::IdPrefix.c_str(), var->getName().c_str(),
                  Option::IdPrefix.c_str());
      } else if (isString && scalarExp) {
        cg_printf("LITSTR_INIT(%s)",
                  scalarExp->getCPPLiteralString().c_str());
      } else {
        value->outputCPP(cg, ar);
      }
      cg_printf(isString ? ");\n" : ";\n");
      value->outputCPPEnd(cg, ar);
      break;
    }
    default:
      assert(false);
    }
  }
}
