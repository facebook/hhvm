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

#include <lib/statement/break_statement.h>
#include <cpp/base/type_variant.h>

using namespace HPHP;
using namespace std;
using namespace boost;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

BreakStatement::BreakStatement
(STATEMENT_CONSTRUCTOR_PARAMETERS, ExpressionPtr exp)
  : Statement(STATEMENT_CONSTRUCTOR_PARAMETER_VALUES), m_exp(exp) {
  m_name = "break";
}

StatementPtr BreakStatement::clone() {
  BreakStatementPtr stmt(new BreakStatement(*this));
  stmt->m_exp = Clone(m_exp);
  return stmt;
}

///////////////////////////////////////////////////////////////////////////////
// parser functions

///////////////////////////////////////////////////////////////////////////////
// static analysis functions

void BreakStatement::analyzeProgram(AnalysisResultPtr ar) {
  if (m_exp) m_exp->analyzeProgram(ar);
}

StatementPtr BreakStatement::preOptimize(AnalysisResultPtr ar) {
  ar->preOptimize(m_exp);
  Variant v;
  // break/continue 1 => break/continue;
  if (m_exp && m_exp->getScalarValue(v) &&
      v.isInteger() && v.toInt64() == 1) {
    m_exp = ExpressionPtr();
  }
  return StatementPtr();
}

StatementPtr BreakStatement::postOptimize(AnalysisResultPtr ar) {
  ar->postOptimize(m_exp);
  return StatementPtr();
}

void BreakStatement::inferTypes(AnalysisResultPtr ar) {
  if (m_exp) m_exp->inferAndCheck(ar, Type::Int64, false);
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void BreakStatement::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  if (m_exp) {
    cg.printf("%s ", m_name);
    m_exp->outputPHP(cg, ar);
    cg.printf(";\n");
  } else {
    cg.printf("%s;\n", m_name);
  }
}

void BreakStatement::outputCPP(CodeGenerator &cg, AnalysisResultPtr ar) {
  const std::vector<int> &labelIds = cg.getBreakScopes();
  if (labelIds.empty()) {
    cg.printf("throw_fatal(\"bad %s\");\n", m_name);
    return;
  }

  if (m_exp) {
    cg.printf("switch (");
    m_exp->outputCPP(cg, ar);
    cg.printf(") {\n");
    for (unsigned int i = 0; i < labelIds.size(); i++) {
      int labelId = labelIds[i];
      labelId &= ~CodeGenerator::BreakScopeBitMask;
      cg.printf("case %d: goto %s%d;\n", labelIds.size() - i, m_name, labelId);
      cg.addLabelId(m_name, labelId);
    }
    cg.printf("default: throw_fatal(\"bad %s\");\n", m_name);
    cg.printf("}\n");
  } else {
    int labelId = labelIds.back();
    if (labelId & CodeGenerator::InsideSwitch) {
      if (labelId & CodeGenerator::StaticCases) {
        cg.printf("break;\n"); // continue will turn into break as well
      } else {
        labelId &= ~CodeGenerator::BreakScopeBitMask;
        cg.printf("goto %s%d;\n", m_name, labelId);
        cg.addLabelId(m_name, labelId);
      }
    } else {
      cg.printf("%s;\n", m_name);
    }
  }
}
