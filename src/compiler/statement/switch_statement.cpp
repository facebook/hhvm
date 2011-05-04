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

#include <compiler/statement/switch_statement.h>
#include <compiler/analysis/analysis_result.h>
#include <compiler/statement/statement_list.h>
#include <compiler/statement/case_statement.h>
#include <compiler/option.h>
#include <compiler/analysis/code_error.h>
#include <compiler/analysis/variable_table.h>
#include <compiler/expression/simple_variable.h>
#include <compiler/expression/scalar_expression.h>

using namespace HPHP;
using namespace std;
using namespace boost;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

SwitchStatement::SwitchStatement
(STATEMENT_CONSTRUCTOR_PARAMETERS, ExpressionPtr exp, StatementListPtr cases)
  : Statement(STATEMENT_CONSTRUCTOR_PARAMETER_VALUES),
    m_exp(exp), m_cases(cases) {
  if (m_cases && m_exp->is(Expression::KindOfSimpleVariable)) {
    for (int i = m_cases->getCount(); i--; ) {
      CaseStatementPtr c(dynamic_pointer_cast<CaseStatement>((*m_cases)[i]));
      if (c->getCondition() && c->getCondition()->hasEffect()) {
        m_exp->setContext(Expression::LValue);
        m_exp->setContext(Expression::NoLValueWrapper);
        break;
      }
    }
  }
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

  if (ar->getPhase() == AnalysisResult::AnalyzeAll &&
      m_exp->is(Expression::KindOfSimpleVariable)) {
    SimpleVariablePtr exp = dynamic_pointer_cast<SimpleVariable>(m_exp);
    if (exp && exp->getSymbol() && exp->getSymbol()->isClassName()) {
      // Mark some classes as volitle since the name is used in switch
      for (int i = 0; i < m_cases->getCount(); i++) {
        CaseStatementPtr stmt =
          dynamic_pointer_cast<CaseStatement>((*m_cases)[i]);
        ASSERT(stmt);
        ExpressionPtr caseCond = stmt->getCondition();
        if (caseCond && caseCond->isScalar()) {
          ScalarExpressionPtr name =
            dynamic_pointer_cast<ScalarExpression>(caseCond);
          if (name && name->isLiteralString()) {
            string className = name->getLiteralString();
            ClassScopePtr cls = ar->findClass(Util::toLower(className));
            if (cls && cls->isUserClass()) {
              cls->setVolatile();
            }
          }
        }
      }
      // Also note this down as code error
      ConstructPtr self = shared_from_this();
      Compiler::Error(Compiler::ConditionalClassLoading, self);
    }
  }
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

  TypePtr ret = m_exp->inferAndCheck(ar, Type::Some, false);
  // these are the cases where toInt64(x) is OK for the switch statement
  if (allInteger && (ret->is(Type::KindOfInt32) || ret->is(Type::KindOfInt64))) {
    m_exp->setExpectedType(Type::Int64);
  }
  if (ret->is(Type::KindOfObject) && ret->isSpecificObject()) {
    m_exp->setExpectedType(Type::Object);
  }
  ConstructPtr self = shared_from_this();
  if (m_cases && m_cases->getCount()) {
    int defaultCount = 0;
    for (int i = 0; i < m_cases->getCount(); i++) {
      CaseStatementPtr stmt =
        dynamic_pointer_cast<CaseStatement>((*m_cases)[i]);
      stmt->inferAndCheck(ar, Type::Some, false);
      ExpressionPtr cond = stmt->getCondition();
      if (!cond) {
        defaultCount++;
      }
    }
    if (defaultCount > 1 && getScope()->isFirstPass()) {
      Compiler::Error(Compiler::MoreThanOneDefault, m_cases);
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void SwitchStatement::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  cg_printf("switch (");
  m_exp->outputPHP(cg, ar);
  cg_printf(") {\n");
  if (m_cases) m_cases->outputPHP(cg, ar);
  cg_printf("}\n");
}

void SwitchStatement::outputCPPImpl(CodeGenerator &cg, AnalysisResultPtr ar) {
  int labelId = cg.createNewLocalId(shared_from_this());

  // if isStaticInt, then we can avoid calling hashForIntSwitch() in static case
  bool isStaticInt = m_exp->getType()->isInteger();

  bool hasSentinel   = true;
  uint64 sentinel    = 0;
  int64 firstNonZero = 0;

  bool staticCases = true;
  if (m_cases) {
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

        int64 num = stmt->getLiteralInteger();
        if (num != 0 && firstNonZero == 0)
          firstNonZero = num;

        // detecting duplicate case value
        if (seenNums.find(num) != seenNums.end()) {
          staticCases = false;
          break;
        }

        seenNums.insert(num);
      } else {
        seenDefault = true;
      }
    }

    // now scan for an available sentinel value - we only have to do this
    // if we care (which is we will do static cases and our operand isn't
    // known to be an int)
    if (staticCases && !isStaticInt) {
      while (sentinel < ULLONG_MAX) {
        if (seenNums.find((int64)sentinel) == seenNums.end()) {
          break;
        }
        sentinel++;
      }
      if (sentinel == ULLONG_MAX && 
          seenNums.find((int64)sentinel) != seenNums.end())
        hasSentinel = false;
    }
  }

  // if theres no non-zero match, then we use the sentinel to create a no-match 
  if (firstNonZero == 0) {
    ASSERT(!staticCases || isStaticInt || hasSentinel);
    firstNonZero = sentinel;
  }

  // if we somehow managed to enumerate all 2^64 int literals + 
  // we don't have statically known int switch operand, then sorry
  // we are out of luck
  if (staticCases && !isStaticInt && !hasSentinel)
    staticCases = false;

  labelId |= CodeGenerator::InsideSwitch;
  if (staticCases) labelId |= CodeGenerator::StaticCases;
  cg.pushBreakScope(labelId, false);
  labelId &= ~CodeGenerator::BreakScopeBitMask;
  cg_indentBegin("{\n");

  string var;
  int varId = -1;

  if (m_exp->preOutputCPP(cg, ar, 0)) {
    varId = cg.createNewLocalId(shared_from_this());
    var = string(Option::SwitchPrefix) + lexical_cast<string>(varId);
    m_exp->getType()->outputCPPDecl(cg, ar, getScope());
    cg_printf(" %s;\n", var.c_str());

    m_exp->outputCPPBegin(cg, ar);
    cg_printf("%s = (", var.c_str());
    m_exp->outputCPP(cg, ar);
    cg_printf(");\n");
    m_exp->outputCPPEnd(cg, ar);
  }

  if (staticCases) {
    cg_printf("switch (");
    if (isStaticInt) {
      if (!var.empty()) {
        cg_printf("%s", var.c_str());
      } else {
        m_exp->outputCPP(cg, ar);
      }
    } else {
      assert(hasSentinel);

      if (m_exp->getType()->is(Type::KindOfDouble)) {
        // double we must special case
        cg_printf("Variant::DoubleHashForIntSwitch(");
      }

      if (!var.empty()) {
        cg_printf("%s", var.c_str());
      } else {
        cg_printf("(");
        m_exp->outputCPP(cg, ar);
        cg_printf(")");
      }

      if (m_exp->getType()->is(Type::KindOfBoolean)) {
        // boolean we must special case
        cg_printf(" ? %ld : 0", firstNonZero);
      } else if (m_exp->getType()->is(Type::KindOfDouble)) {
        cg_printf(", %ld)", (int64) sentinel);
      } else {
        // at this point we must be dealing with a variable
        // which implements hashForIntSwitch()
        cg_printf(".hashForIntSwitch(%ld, %ld)",
                  firstNonZero,
                  (int64) sentinel);
      }
    }
    cg_printf(") {\n");
    if (m_cases) m_cases->outputCPP(cg, ar);
    cg_printf("}\n");
  } else {
    if (var.empty()) {
      varId = cg.createNewLocalId(shared_from_this());
      if (m_exp->hasContext(Expression::LValue) &&
          m_exp->is(Expression::KindOfSimpleVariable)) {
        var = getScope()->getVariables()->getVariableName(
          cg, ar, static_pointer_cast<SimpleVariable>(m_exp)->getName());
      } else {
        var = string(Option::SwitchPrefix) + lexical_cast<string>(varId);
        m_exp->getType()->outputCPPDecl(cg, ar, getScope());
        cg_printf(" %s = (", var.c_str());
        m_exp->outputCPP(cg, ar);
        cg_printf(");\n");
      }
    }

    if (m_cases && m_cases->getCount()) {
      CaseStatementPtr defaultCase;
      int defaultCaseNum = -1;
      for (int i = 0; i < m_cases->getCount(); i++) {
        CaseStatementPtr stmt =
          dynamic_pointer_cast<CaseStatement>((*m_cases)[i]);
        if (stmt->getCondition()) {
          stmt->outputCPPAsIf(cg, ar, varId, var.c_str(), i);
        } else {
          defaultCase = stmt;
          defaultCaseNum = i;
        }
      }
      if (defaultCaseNum != -1) {
        defaultCase->outputCPPAsIf(cg, ar, varId, var.c_str(), defaultCaseNum);
      } else {
        cg_printf("goto break%d;\n", labelId);
        cg.addLabelId("break", labelId);
      }
      cg_printf("\n");
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
    cg_printf("continue%d:;\n", labelId);
  }
  if (cg.findLabelId("break", labelId)) {
    cg_printf("break%d:;\n", labelId);
  }
  cg_indentEnd("}\n");
  cg.popBreakScope();
}
