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

#include <compiler/statement/break_statement.h>
#include <runtime/base/complex_types.h>

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

void BreakStatement::analyzeProgramImpl(AnalysisResultPtr ar) {
  if (m_exp) m_exp->analyzeProgram(ar);
}

ConstructPtr BreakStatement::getNthKid(int n) const {
  switch (n) {
    case 0:
      return m_exp;
    default:
      ASSERT(false);
      break;
  }
  return ConstructPtr();
}

int BreakStatement::getKidCount() const {
  return 1;
}

void BreakStatement::setNthKid(int n, ConstructPtr cp) {
  switch (n) {
    case 0:
      m_exp = boost::dynamic_pointer_cast<Expression>(cp);
      break;
    default:
      ASSERT(false);
      break;
  }
}

StatementPtr BreakStatement::preOptimize(AnalysisResultConstPtr ar) {
  Variant v;
  // break/continue 1 => break/continue;
  if (m_exp && m_exp->getScalarValue(v) &&
      v.isInteger() && v.toInt64() == 1) {
    m_exp = ExpressionPtr();
  }
  return StatementPtr();
}

void BreakStatement::inferTypes(AnalysisResultPtr ar) {
  if (m_exp) m_exp->inferAndCheck(ar, Type::Int64, false);
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void BreakStatement::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  if (m_exp) {
    cg_printf("%s ", m_name);
    m_exp->outputPHP(cg, ar);
    cg_printf(";\n");
  } else {
    cg_printf("%s;\n", m_name);
  }
}

int64 BreakStatement::getDepth() {
  if (!m_exp) return 1;
  Variant v;
  if (m_exp->getScalarValue(v) &&
      v.isInteger()) {
    int64 depth = v.toInt64();
    return depth >= 1 ? depth : 1;
  }
  return 0;
}

void BreakStatement::outputCPPImpl(CodeGenerator &cg, AnalysisResultPtr ar) {
  const std::vector<int> &labelIds = cg.getBreakScopes();
  if (labelIds.empty()) {
    cg_printf("throw_fatal(\"bad %s\");\n", m_name);
    return;
  }

  int64 depth = getDepth();

  if (!depth) {
    unsigned size = labelIds.size();
    int labelId;

    int varId = cg.createNewLocalId(shared_from_this());
    cg_printf("int64 %s%d;\n", Option::TempPrefix, varId);

    m_exp->outputCPPBegin(cg, ar);
    cg_printf("%s%d = (", Option::TempPrefix, varId);
    m_exp->outputCPP(cg, ar);
    cg_printf(");\n");
    m_exp->outputCPPEnd(cg, ar);

    if (size > 1) {
      cg_printf("switch (");
      cg_printf("%s%d", Option::TempPrefix, varId);
      cg_printf(") {\n");
      for (unsigned int i = 0; i < size; i++) {
        labelId = labelIds[i];
        labelId &= ~CodeGenerator::BreakScopeBitMask;
        cg_printf("case %d: goto %s%d;\n",
                  labelIds.size() - i, m_name, labelId);
        cg.addLabelId(m_name, labelId);
      }
      cg_printf("default:\n");
    } else {
      labelId = labelIds.back();
      labelId &= ~CodeGenerator::BreakScopeBitMask;
      cg.addLabelId(m_name, labelId);
    }
    cg_printf("if (");
    cg_printf("%s%d", Option::TempPrefix, varId);
    cg_printf("<2) {\n");
    cg_printf("goto %s%d;\n", m_name, labelId);
    cg_printf("} else {\n");
    cg_printf("throw_fatal(\"bad %s\");\n", m_name);
    cg_printf("}\n");
    if (size > 1) {
      cg_printf("}\n");
    }
    return;
  }

  ASSERT(depth >= 1);

  if (depth > (int64)labelIds.size()) {
    cg_printf("throw_fatal(\"bad %s\");\n", m_name);
    return;
  }

  int labelId = labelIds[labelIds.size() - depth];
  if (depth > 1 || labelId & CodeGenerator::InsideSwitch) {
    if (depth == 1 && labelId & CodeGenerator::StaticCases) {
      cg_printf("break;\n"); // continue will turn into break as well
    } else {
      labelId &= ~CodeGenerator::BreakScopeBitMask;
      cg_printf("goto %s%d;\n", m_name, labelId);
      cg.addLabelId(m_name, labelId);
    }
  } else {
    cg_printf("%s;\n", m_name);
  }
}
