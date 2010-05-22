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

#include <compiler/statement/return_statement.h>
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
  if (m_exp) m_exp->analyzeProgram(ar);
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

StatementPtr ReturnStatement::preOptimize(AnalysisResultPtr ar) {
  ar->preOptimize(m_exp);
  return StatementPtr();
}

StatementPtr ReturnStatement::postOptimize(AnalysisResultPtr ar) {
  ar->postOptimize(m_exp);
  return StatementPtr();
}

void ReturnStatement::inferTypes(AnalysisResultPtr ar) {
  if (m_exp) {
    FunctionScopePtr funcScope =
      dynamic_pointer_cast<FunctionScope>(ar->getScope());
    if (funcScope) {
      if (funcScope->isRefReturn()) {
        m_exp->setContext(Expression::RefValue);
      }
      TypePtr ret;
      if (funcScope->isOverriding()) {
        if (funcScope->getReturnType()) {
          ret = m_exp->inferAndCheck(ar, funcScope->getReturnType(), false);
        } else {
          ConstructPtr self = shared_from_this();
          ret = m_exp->inferAndCheck(ar, NEW_TYPE(Some), false);
          ar->getCodeError()->record(self, CodeError::BadReturnStatement,
                                     self);
        }
      } else {
        TypePtr expected = NEW_TYPE(Some);
        if (ar->getPhase() == AnalysisResult::LastInference &&
            funcScope->getReturnType()) {
          expected = funcScope->getReturnType();
        }
        ret = m_exp->inferAndCheck(ar, expected, false);
        funcScope->setReturnType(ar, ret);
      }
    } else {
      m_exp->inferAndCheck(ar, Type::Int64, false);
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void ReturnStatement::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  if (m_exp) {
    cg.printf("return ");
    m_exp->outputPHP(cg, ar);
    cg.printf(";\n");
  } else {
    cg.printf("return;\n");
  }
}

void ReturnStatement::outputCPPImpl(CodeGenerator &cg, AnalysisResultPtr ar) {
  if (hasHphpNote("C++")) {
    cg.printf("%s", getEmbedded().c_str());
  }

  bool braced = false;
  FunctionScopePtr func = dynamic_pointer_cast<FunctionScope>(ar->getScope());
  ClassScopePtr cls = ar->getClassScope();
  if (func->isConstructor(cls)) {
    cg.indentBegin("{\n"); braced = true;
    cg.printf("gasInCtor(oldInCtor);\n");
  }
  if (m_exp) {
    m_exp->outputCPPBegin(cg, ar);
  }

  cg.printf("return");

  if (m_exp) {
    cg.printf(" ");
    m_exp->outputCPP(cg, ar);
    cg.printf(";\n");
    m_exp->outputCPPEnd(cg, ar);
  } else if (func &&
             !(func->inPseudoMain() && !Option::GenerateCPPMain &&
               cg.getOutput() != CodeGenerator::SystemCPP)) {
    TypePtr type = func->getReturnType();
    if (type) {
      const char *initializer = type->getCPPInitializer();
      cg.printf(" %s", initializer ? initializer : "null");
    }
    cg.printf(";\n");
  }

  if (braced) cg.indentEnd("}\n");
}
