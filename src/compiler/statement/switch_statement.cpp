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

#include <compiler/statement/switch_statement.h>
#include <compiler/analysis/analysis_result.h>
#include <compiler/statement/statement_list.h>
#include <compiler/statement/case_statement.h>
#include <compiler/option.h>
#include <compiler/analysis/code_error.h>

using namespace HPHP;
using namespace std;
using namespace boost;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

SwitchStatement::SwitchStatement
(STATEMENT_CONSTRUCTOR_PARAMETERS, ExpressionPtr exp, StatementListPtr cases)
  : Statement(STATEMENT_CONSTRUCTOR_PARAMETER_VALUES),
    m_exp(exp), m_cases(cases) {
}

StatementPtr SwitchStatement::clone() {
  SwitchStatementPtr stmt(new SwitchStatement(*this));
  stmt->m_cases = Clone(m_cases);
  stmt->m_exp = Clone(m_exp);
  return stmt;
}

int SwitchStatement::getRecursiveCount() const {
  return 1 + (m_cases ? m_cases->getRecursiveCount() : 0);
}

///////////////////////////////////////////////////////////////////////////////
// parser functions

///////////////////////////////////////////////////////////////////////////////
// static analysis functions

void SwitchStatement::analyzeProgramImpl(AnalysisResultPtr ar) {
  m_exp->analyzeProgram(ar);
  if (m_cases) m_cases->analyzeProgram(ar);
}

bool SwitchStatement::hasDecl() const {
  return m_cases && m_cases->hasDecl();
}

bool SwitchStatement::hasRetExp() const {
  return m_cases && m_cases->hasRetExp();
}

ConstructPtr SwitchStatement::getNthKid(int n) const {
  switch (n) {
    case 0:
      return m_exp;
    case 1:
      return m_cases;
    default:
      ASSERT(false);
      break;
  }
  return ConstructPtr();
}

int SwitchStatement::getKidCount() const {
  return 2;
}

void SwitchStatement::setNthKid(int n, ConstructPtr cp) {
  switch (n) {
    case 0:
      m_exp = boost::dynamic_pointer_cast<Expression>(cp);
      break;
    case 1:
      m_cases = boost::dynamic_pointer_cast<StatementList>(cp);
      break;
    default:
      ASSERT(false);
      break;
  }
}

StatementPtr SwitchStatement::preOptimize(AnalysisResultPtr ar) {
  ar->preOptimize(m_exp);
  ar->preOptimize(m_cases);
  return StatementPtr();
}

StatementPtr SwitchStatement::postOptimize(AnalysisResultPtr ar) {
  ar->postOptimize(m_exp);
  ar->postOptimize(m_cases);
  return StatementPtr();
}

void SwitchStatement::inferTypes(AnalysisResultPtr ar) {
  // we optimize the most two common cases of switch statements
  bool allInteger = true;
  bool allString = true;
  if (m_cases && m_cases->getCount()) {
    for (int i = 0; i < m_cases->getCount(); i++) {
      CaseStatementPtr stmt =
        dynamic_pointer_cast<CaseStatement>((*m_cases)[i]);
      if (!stmt->getCondition()) {
        if (m_cases->getCount() == 1) allInteger = allString = false;
      } else {
        if (!stmt->isLiteralInteger()) allInteger = false;
        if (!stmt->isLiteralString()) allString = false;
      }
    }
  }
  if (allInteger && allString) {
    allInteger = allString = false;
  }

  TypePtr ret;
  if (allInteger) {
    ret = m_exp->inferAndCheck(ar, Type::Int64, false);
  } else if (allString) {
    // We're not able to do this, because switch($obj) may work on an object
    // that didn't implement __toString(), throwing an exception, which isn't
    // consistent with PHP.
    //ret = m_exp->inferAndCheck(ar, Type::String, false);
    ret = m_exp->inferAndCheck(ar, NEW_TYPE(Some), false);
  } else {
    ret = m_exp->inferAndCheck(ar, NEW_TYPE(Some), false);
  }
  if (ret->is(Type::KindOfObject) && ret->isSpecificObject()) {
    m_exp->setExpectedType(NEW_TYPE(Object));
  }
  ConstructPtr self = shared_from_this();
  if (m_cases && m_cases->getCount()) {
    bool checking = false;
    vector<int> defaults;
    int defaultCount = 0;
    for (int i = 0; i < m_cases->getCount(); i++) {
      CaseStatementPtr stmt =
        dynamic_pointer_cast<CaseStatement>((*m_cases)[i]);
      stmt->inferAndCheck(ar, NEW_TYPE(Some), false);
      ExpressionPtr cond = stmt->getCondition();
      if (!cond) {
        checking = true;
        defaultCount++;
      } else if (checking && cond && ar->isFirstPass()) {
        defaults.push_back(i);
        ar->getCodeError()->record(self, CodeError::CaseAfterDefault, stmt);
      }
    }
    if (defaultCount > 1 && ar->isFirstPass()) {
      ar->getCodeError()->record(self, CodeError::MoreThanOneDefault, m_cases);
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void SwitchStatement::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  cg.printf("switch (");
  m_exp->outputPHP(cg, ar);
  cg.printf(") {\n");
  if (m_cases) m_cases->outputPHP(cg, ar);
  cg.printf("}\n");
}

void SwitchStatement::outputCPPImpl(CodeGenerator &cg, AnalysisResultPtr ar) {
  int labelId = cg.createNewId(ar);

  bool staticCases = true;
  if (!m_exp->getType()->isInteger()) {
    staticCases = false;
  } else if (m_cases) {
    bool seenDefault = false;
    set<int64> seenNums;
    for (int i = 0; i < m_cases->getCount(); i++) {
      CaseStatementPtr stmt =
        dynamic_pointer_cast<CaseStatement>((*m_cases)[i]);
      if (stmt->getCondition()) {
        if (!stmt->isLiteralInteger() || seenDefault) {
          staticCases = false;
          break;
        }

        // detecting duplicate case value
        int64 num = stmt->getLiteralInteger();
        if (seenNums.find(num) != seenNums.end()) {
          staticCases = false;
          break;
        }
        seenNums.insert(num);
      } else {
        seenDefault = true;
      }
    }
  }

  labelId |= CodeGenerator::InsideSwitch;
  if (staticCases) labelId |= CodeGenerator::StaticCases;
  cg.pushBreakScope(labelId, false);
  labelId &= ~CodeGenerator::BreakScopeBitMask;
  cg.indentBegin("{\n");

  int varId = -1;

  if (m_exp->preOutputCPP(cg, ar, 0)) {
    varId = cg.createNewId(ar);
    m_exp->getType()->outputCPPDecl(cg, ar);
    cg.printf(" %s%d;\n", Option::TempPrefix, varId);

    m_exp->outputCPPBegin(cg, ar);
    cg.printf("%s%d = (", Option::TempPrefix, varId);
    m_exp->outputCPP(cg, ar);
    cg.printf(");\n");
    m_exp->outputCPPEnd(cg, ar);
  }

  if (staticCases) {
    cg.printf("switch (");
    if (varId >= 0) {
      cg.printf("%s%d", Option::TempPrefix, varId);
    } else {
      m_exp->outputCPP(cg, ar);
    }
    cg.printf(") {\n");
    if (m_cases) m_cases->outputCPP(cg, ar);
    cg.printf("}\n");
  } else {
    if (varId < 0) {
      varId = cg.createNewId(ar);
      m_exp->getType()->outputCPPDecl(cg, ar);
      cg.printf(" %s%d = (", Option::TempPrefix, varId);
      m_exp->outputCPP(cg, ar);
      cg.printf(");\n");
    }

    if (m_cases && m_cases->getCount()) {
      CaseStatementPtr defaultCase;
      int defaultCaseNum = -1;
      for (int i = 0; i < m_cases->getCount(); i++) {
        CaseStatementPtr stmt =
          dynamic_pointer_cast<CaseStatement>((*m_cases)[i]);
        if (stmt->getCondition()) {
          stmt->outputCPPAsIf(cg, ar, varId, i);
        } else {
          defaultCase = stmt;
          defaultCaseNum = i;
        }
      }
      if (defaultCaseNum != -1) {
        defaultCase->outputCPPAsIf(cg, ar, varId, defaultCaseNum);
      } else {
        cg.printf("goto break%d;\n", labelId);
        cg.addLabelId("break", labelId);
      }
      cg.printf("\n");
      for (int i = 0; i < m_cases->getCount(); i++) {
        CaseStatementPtr stmt =
          dynamic_pointer_cast<CaseStatement>((*m_cases)[i]);
        stmt->outputCPPByNumber(cg, ar, varId,
                                !stmt->getCondition() && defaultCaseNum != i ?
                                -1 : i);
      }
    }
  }

  // Even though switch's break/continue will never goto these labels, we need
  // them for "break/continue n" inside switches.
  if (cg.findLabelId("continue", labelId)) {
    cg.printf("continue%d:;\n", labelId);
  }
  if (cg.findLabelId("break", labelId)) {
    cg.printf("break%d:;\n", labelId);
  }
  cg.indentEnd("}\n");
  cg.popBreakScope();
}
