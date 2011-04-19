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

#include <compiler/statement/return_statement.h>
#include <compiler/expression/unary_op_expression.h>
#include <compiler/expression/function_call.h>
#include <compiler/analysis/analysis_result.h>
#include <compiler/analysis/function_scope.h>
#include <compiler/analysis/code_error.h>
#include <compiler/analysis/class_scope.h>

using namespace HPHP;
using namespace std;
using namespace boost;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

ReturnStatement::ReturnStatement
(STATEMENT_CONSTRUCTOR_PARAMETERS, ExpressionPtr exp)
  : Statement(STATEMENT_CONSTRUCTOR_PARAMETER_VALUES), m_exp(exp) {
}

StatementPtr ReturnStatement::clone() {
  ReturnStatementPtr stmt(new ReturnStatement(*this));
  stmt->m_exp = Clone(m_exp);
  return stmt;
}

///////////////////////////////////////////////////////////////////////////////
// parser functions

///////////////////////////////////////////////////////////////////////////////
// static analysis functions

void ReturnStatement::analyzeProgramImpl(AnalysisResultPtr ar) {
  if (m_exp) {
    FunctionScopePtr funcScope = getFunctionScope();
    if (funcScope) {
      if (funcScope->isRefReturn()) {
        m_exp->setContext(Expression::RefValue);
      }
    }
    m_exp->analyzeProgram(ar);
  }
}

ConstructPtr ReturnStatement::getNthKid(int n) const {
  switch (n) {
    case 0:
      return m_exp;
    default:
      ASSERT(false);
      break;
  }
  return ConstructPtr();
}

int ReturnStatement::getKidCount() const {
  return 1;
}

void ReturnStatement::setNthKid(int n, ConstructPtr cp) {
  switch (n) {
    case 0:
      m_exp = boost::dynamic_pointer_cast<Expression>(cp);
      break;
    default:
      ASSERT(false);
      break;
  }
}

void ReturnStatement::inferTypes(AnalysisResultPtr ar) {
  FunctionScopePtr funcScope = getFunctionScope();
  if (m_exp) {
    if (funcScope) {
      TypePtr ret;
      if (funcScope->isOverriding()) {
        if (funcScope->getReturnType()) {
          ret = m_exp->inferAndCheck(ar, funcScope->getReturnType(), false);
        } else {
          ConstructPtr self = shared_from_this();
          ret = m_exp->inferAndCheck(ar, Type::Some, false);
        }
      } else {
        ret = m_exp->inferAndCheck(ar, Type::Some, false);
        funcScope->setReturnType(ar, ret);
        funcScope->addRetExprToFix(m_exp);
      }
    } else {
      m_exp->inferAndCheck(ar, Type::Int64, false);
    }
  } else {
    funcScope->setReturnType(ar, TypePtr());
  }
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void ReturnStatement::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  if (m_exp) {
    cg_printf("return ");
    m_exp->outputPHP(cg, ar);
    cg_printf(";\n");
  } else {
    cg_printf("return;\n");
  }
}

static bool checkCopyElision(FunctionScopePtr func, ExpressionPtr exp) {
  if (!exp->getType()->is(Type::KindOfVariant) || func->isRefReturn()) {
    return false;
  }

  TypePtr imp = exp->getImplementedType();
  if (!imp) imp = exp->getActualType();
  if (!imp || !imp->is(Type::KindOfVariant)) return false;

  if (func->getNRVOFix() && exp->is(Expression::KindOfSimpleVariable)) {
    return true;
  }

  if (FunctionCallPtr fc = dynamic_pointer_cast<FunctionCall>(exp)) {
    FunctionScopePtr fs = fc->getFuncScope();
    if (!fs || fs->isRefReturn()) {
      return true;
    }
  }

  return false;
}

void ReturnStatement::outputCPPImpl(CodeGenerator &cg, AnalysisResultPtr ar) {
  bool braced = false;
  FunctionScopePtr func = getFunctionScope();
  ClassScopePtr cls = getClassScope();
  if (func->isConstructor(cls)) {
    cg_indentBegin("{\n"); braced = true;
    cg_printf("gasInCtor(oldInCtor);\n");
  }
  if (m_exp) {
    m_exp->outputCPPBegin(cg, ar);
  }

  cg_printf("return");

  if (m_exp) {
    bool close = false;
    cg_printf(" ");
    if (checkCopyElision(func, m_exp)) {
      cg_printf("wrap_variant(");
      close = true;
    }
    m_exp->outputCPP(cg, ar);
    if (close) cg_printf(")");
    cg_printf(";\n");
    cg.setReferenceTempUsed(false);
    m_exp->outputCPPEnd(cg, ar);
  } else if (func &&
             !(func->inPseudoMain() && !Option::GenerateCPPMain &&
               cg.getOutput() != CodeGenerator::SystemCPP)) {
    TypePtr type = func->getReturnType();
    if (type) {
      const char *initializer = type->getCPPInitializer();
      cg_printf(" %s", initializer ? initializer : "null");
    }
    cg_printf(";\n");
  }

  if (braced) cg_indentEnd("}\n");
}
