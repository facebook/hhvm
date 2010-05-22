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

#include <compiler/expression/encaps_list_expression.h>
#include <compiler/expression/expression_list.h>
#include <compiler/expression/binary_op_expression.h>
#include <compiler/analysis/code_error.h>

using namespace HPHP;
using namespace std;
using namespace boost;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

EncapsListExpression::EncapsListExpression
(EXPRESSION_CONSTRUCTOR_PARAMETERS, int type, ExpressionListPtr expList)
  : Expression(EXPRESSION_CONSTRUCTOR_PARAMETER_VALUES),
    m_type(type), m_exps(expList) {
}

ExpressionPtr EncapsListExpression::clone() {
  EncapsListExpressionPtr exp(new EncapsListExpression(*this));
  Expression::deepCopy(exp);
  exp->m_exps = Clone(m_exps);
  return exp;
}

///////////////////////////////////////////////////////////////////////////////
// parser functions

///////////////////////////////////////////////////////////////////////////////
// static analysis functions

void EncapsListExpression::analyzeProgram(AnalysisResultPtr ar) {
  if (m_exps) {
    for (int i = 0; i < m_exps->getCount(); i++) {
      (*m_exps)[i]->analyzeProgram(ar);
    }
  }
}

ConstructPtr EncapsListExpression::getNthKid(int n) const {
  switch (n) {
    case 0:
      return m_exps;
    default:
      ASSERT(false);
      break;
  }
  return ConstructPtr();
}

int EncapsListExpression::getKidCount() const {
  return 1;
}

void EncapsListExpression::setNthKid(int n, ConstructPtr cp) {
  switch (n) {
    case 0:
      m_exps = boost::dynamic_pointer_cast<ExpressionList>(cp);
      break;
    default:
      ASSERT(false);
      break;
  }
}

ExpressionPtr EncapsListExpression::preOptimize(AnalysisResultPtr ar) {
  ar->preOptimize(m_exps);
  if (m_type != '`' && m_type != '\'' && m_exps) {
    int count = m_exps->getCount();
    // turn into cascaded concat
    if (count > 2) {
      ExpressionPtr exp =
        BinaryOpExpressionPtr(new BinaryOpExpression(getLocation(),
          Expression::KindOfBinaryOpExpression,
          (*m_exps)[0], (*m_exps)[1], '.'));
      for (int i = 2; i < count; i++) {
        exp =
          BinaryOpExpressionPtr(new BinaryOpExpression(getLocation(),
            Expression::KindOfBinaryOpExpression,
            exp, (*m_exps)[i], '.'));
      }
      return exp;
    }
  }
  return ExpressionPtr();
}

ExpressionPtr EncapsListExpression::postOptimize(AnalysisResultPtr ar) {
  ar->postOptimize(m_exps);
  return ExpressionPtr();
}

TypePtr EncapsListExpression::inferTypes(AnalysisResultPtr ar, TypePtr type,
                                         bool coerce) {
  if (m_exps) {
    for (int i = 0; i < m_exps->getCount(); i++) {
      (*m_exps)[i]->inferAndCheck(ar, Type::String, false);
    }
  }

  if (ar->isFirstPass() && m_type == '`') {
    ConstructPtr self = shared_from_this();
    ar->getCodeError()->record(self, CodeError::UseShellExec, self);
  }

  return Type::String;
}

bool EncapsListExpression::canonCompare(ExpressionPtr e) const {
  if (!Expression::canonCompare(e)) return false;
  EncapsListExpressionPtr el = 
    static_pointer_cast<EncapsListExpression>(e);

  return m_type == el->m_type;
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void EncapsListExpression::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  if (m_type == '`') cg.printf("shell_exec(");

  if (m_exps) {
    for (int i = 0; i < m_exps->getCount(); i++) {
      if (i > 0) cg.printf(" . ");
      (*m_exps)[i]->outputPHP(cg, ar);
    }
  } else {
    cg.printf("''");
  }

  if (m_type == '`') cg.printf(")");
}

void EncapsListExpression::outputCPPImpl(CodeGenerator &cg,
                                         AnalysisResultPtr ar) {
  if (m_type == '`') cg.printf("f_shell_exec(");

  if (m_exps) {
    for (int i = 0; i < m_exps->getCount(); i++) {
      ExpressionPtr exp = (*m_exps)[i];
      if (i > 0) cg.printf(" + ");
      if (exp->is(Expression::KindOfScalarExpression)) {
        cg.printf("toString(");
        exp->outputCPP(cg, ar);
        cg.printf(")");
      } else {
        exp->outputCPP(cg, ar);
      }
    }
  } else {
    cg.printf("\"\"");
  }

  if (m_type == '`') cg.printf(")");
}
