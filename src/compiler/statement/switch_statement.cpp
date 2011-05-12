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

#include <runtime/base/comparisons.h>

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

  bool staticIntCases    = true;
  bool staticStringCases = true;

  uint64 numDistinctLabels   = 0;
  uint64 numNonDefaultLabels = 0;
  uint64 tableSize           = 0;
  if (m_cases) {
    bool seenDefault = false;
    set<int64> seenNums;
    set<string> seenStrs;
    for (int i = 0; 
         i < m_cases->getCount() && (staticIntCases || staticStringCases); 
         i++) {
      CaseStatementPtr stmt =
        dynamic_pointer_cast<CaseStatement>((*m_cases)[i]);
      if (stmt->getCondition()) {
        numNonDefaultLabels++;
        Variant v;
        bool hasValue = stmt->getScalarConditionValue(v);
        if (hasValue && v.isInteger()) {
          staticStringCases = false;
          numDistinctLabels++;
          int64 num = v.toInt64();
          if (num != 0 && firstNonZero == 0) {
            firstNonZero = num;
          }
          // detecting duplicate case value
          if (seenNums.find(num) != seenNums.end()) {
            staticIntCases = false;
            break;
          }
          seenNums.insert(num);
        } else if (hasValue && v.isString()) {
          staticIntCases = false;
          string str(v.toString());
          // static string optimization can handle duplicate
          // cases, since they will just hash to the same bucket
          // anyways
          if (seenStrs.find(str) == seenStrs.end()) {
            numDistinctLabels++;
            seenStrs.insert(str);
          }
        } else {
          staticIntCases = staticStringCases = false;
          break;
        }
      } else {
        if (seenDefault) {
          // don't want to optimize > 1 default cases for static int cases
          staticIntCases = false;
          break;
        }
        seenDefault = true;
      }
    }

    // now scan for an available sentinel value - we only have to do this
    // if we care (which is we will do static cases and our operand isn't
    // known to be an int)
    if (staticIntCases && !isStaticInt) {
      while (sentinel < ULLONG_MAX) {
        if (seenNums.find((int64)sentinel) == seenNums.end()) {
          break;
        }
        sentinel++;
      }
      if (sentinel == ULLONG_MAX && 
          seenNums.find((int64)sentinel) != seenNums.end()) {
        hasSentinel = false;
      }
    }
  }

  // if theres no non-zero match, then we use the sentinel to create a no-match 
  if (firstNonZero == 0) {
    ASSERT(!staticIntCases || isStaticInt || hasSentinel);
    firstNonZero = sentinel;
  }

  // if we somehow managed to enumerate all 2^64 int literals + 
  // we don't have statically known int switch operand, then sorry
  // we are out of luck
  if (staticIntCases && !isStaticInt && !hasSentinel) {
    staticIntCases = false;
  }

  // in the degenerate case of no labels, then treat it as
  // a static int case, or if there is only 1 (non-default) case, then also
  // no point to do static string case optimization
  if ((staticIntCases && staticStringCases) || 
      (staticStringCases && numNonDefaultLabels <= 1)) {
    staticStringCases = false;
  }

  if (staticStringCases) {
    ASSERT(numDistinctLabels > 0);
    tableSize = Util::roundUpToPowerOfTwo(numDistinctLabels * 2);
    if (tableSize == ULLONG_MAX) {
      // we cannot guarantee a no match value in this case
      staticStringCases = false;
    }
  }

  labelId |= CodeGenerator::InsideSwitch;
  if (staticIntCases) labelId |= CodeGenerator::StaticCases;
  cg.pushBreakScope(labelId, false);
  labelId &= ~CodeGenerator::BreakScopeBitMask;

  string var;
  int varId = -1;
  bool closeBrace = false; 

  bool needsPreOutput = m_exp->preOutputCPP(cg, ar, 0);
  TypePtr staticIntSwitchOpnd = m_exp->getType();
  if (staticStringCases || needsPreOutput) {
    varId = cg.createNewLocalId(shared_from_this());
    if (!needsPreOutput &&
        m_exp->hasContext(Expression::LValue) &&
        m_exp->is(Expression::KindOfSimpleVariable)) {
      // use existing variable
      var = getScope()->getVariables()->getVariableName(
        cg, ar, static_pointer_cast<SimpleVariable>(m_exp)->getName());
    } else {
      var = string(Option::SwitchPrefix) + lexical_cast<string>(varId);
      string var0; // holds the variable name to call outputCPP on

      bool needsDecl = true;
      if (staticIntCases) {
        ASSERT(hasSentinel);
        switch (m_exp->getType()->getKindOf()) {
        case Type::KindOfInt64:
        case Type::KindOfBoolean:
        case Type::KindOfDouble:
          m_exp->getType()->outputCPPDecl(cg, ar, getScope());
          cg_printf(" %s;\n", var.c_str());
          needsDecl = false;
          var0 = var;
          break;
        default:
          cg_printf("int64 %s;\n", var.c_str());
          // we must do extra work here
          var0 = string(Option::SwitchPrefix) +
            lexical_cast<string>(cg.createNewLocalId(shared_from_this()));
          staticIntSwitchOpnd = Type::Int64;
          break;
        }
      } else var0 = var;

      closeBrace = true;
      cg_indentBegin("{\n");

      if (needsDecl) {
        m_exp->getType()->outputCPPDecl(cg, ar, getScope());
        cg_printf(" %s;\n", var0.c_str());
      }

      m_exp->outputCPPBegin(cg, ar);
      cg_printf("%s = (", var0.c_str());
      m_exp->outputCPP(cg, ar);
      cg_printf(");\n");
      m_exp->outputCPPEnd(cg, ar);

      if (staticIntCases) {
        if (needsDecl) {
          // copy var0 over to var
          cg_printf("%s = %s.hashForIntSwitch(%lldLL, %lldLL);\n",
                    var.c_str(),
                    var0.c_str(),
                    firstNonZero,
                    (int64) sentinel);
        }
        cg_indentEnd("}\n");
        closeBrace = false;
      }
    }
  }

  if (staticIntCases) {
    ASSERT(!closeBrace);
    ASSERT(hasSentinel);
    ASSERT(!needsPreOutput || !var.empty());
    ASSERT(staticIntSwitchOpnd);

    cg_printf("switch (");
    if (staticIntSwitchOpnd->is(Type::KindOfDouble)) {
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

    if (staticIntSwitchOpnd->is(Type::KindOfBoolean)) {
      // boolean we must special case
      cg_printf(" ? %lldLL : 0LL", firstNonZero);
    } else if (staticIntSwitchOpnd->is(Type::KindOfDouble)) {
      cg_printf(", %lldLL)", (int64) sentinel);
    } else if (!staticIntSwitchOpnd->is(Type::KindOfInt64)) {
      // at this point we must be dealing with a variable
      // which implements hashForIntSwitch()
      cg_printf(".hashForIntSwitch(%lldLL, %lldLL)",
                firstNonZero,
                (int64) sentinel);
    }

    cg_printf(") {\n");
    if (m_cases) m_cases->outputCPP(cg, ar);
    cg_printf("}\n");
  } else if (staticStringCases) {
    ASSERT(!var.empty());
    ASSERT(tableSize > 0);

    // create cases
    MapIntToStatementPtrWithPosVec caseMap;
    int64 
      firstTrueCaseHash  = 0,
      firstNullCaseHash  = 0,
      firstFalseCaseHash = 0,
      firstZeroCaseHash  = 0,
      firstHash          = 0,
      noMatchHash        = 0;
    bool
      hasFirstTrue  = false,
      hasFirstNull  = false,
      hasFirstFalse = false,
      hasFirstZero  = false;

    CaseStatementPtr defaultCase;
    int defaultCaseNum = -1;
    int maxHashCase    = -1;
    set<int64> defaultCases;
    if (m_cases) {
      for (int i = 0; i < m_cases->getCount(); i++) {
        CaseStatementPtr stmt =
          static_pointer_cast<CaseStatement>((*m_cases)[i]);
        ASSERT(stmt);
        if (stmt->getCondition()) {
          Variant v;
          int64 condHash;
          bool hasValue = stmt->getScalarConditionValue(v);
          ASSERT(hasValue && !v.isNull());

          int64 lval; double dval;
          // allow errors, since we want "1abc" to match 1
          DataType type = v.getStringData()->isNumericWithVal(lval, dval, 1);

          switch (type) {
          case KindOfInt64:
            condHash = lval;
            break;
          case KindOfDouble:
            condHash = (int64) dval;
            break;
          case KindOfNull: 
            {
              string l(v.toString());
              condHash = hash_string(l.c_str());
            }
            break;
          default:
            ASSERT(false);
            break;
          }

          if (!hasFirstTrue && equal(v, true)) {
            hasFirstTrue = true;
            firstTrueCaseHash = condHash;
          }
          if (!hasFirstNull && equal(v, null)) {
            hasFirstNull = true;
            firstNullCaseHash = condHash;
          }
          if (!hasFirstFalse && equal(v, false)) {
            hasFirstFalse = true;
            firstFalseCaseHash = condHash;
          }
          if (!hasFirstZero && equal(v, 0)) {
            hasFirstZero = true;
            firstZeroCaseHash = condHash;
          }
          if (i == 0) firstHash = condHash;

          uint64 bucket = ((uint64)condHash) % tableSize;
          MapIntToStatementPtrWithPosVec::iterator it(caseMap.find(bucket));
          if (it == caseMap.end()) {
            shared_ptr<StatementPtrWithPosVec> 
              list(new StatementPtrWithPosVec());
            list->push_back(StatementPtrWithPos(i, stmt));
            caseMap[bucket] = list;
          } else {
            it->second->push_back(StatementPtrWithPos(i, stmt));
          }
          maxHashCase = max(maxHashCase, i);
        } else {
          // we only care about the *last* default case, so we let it be
          // overriden each time
          defaultCase = stmt;
          defaultCaseNum = i;
          defaultCases.insert(i);
        }
      }
      // compute the no match hash
      noMatchHash = tableSize;
    }

    cg_printf("bool needsOrder;\n");
    cg_printf("int64 hash;\n");
    switch (m_exp->getType()->getKindOf()) {
    case Type::KindOfBoolean:
      cg_printf("needsOrder = false;\n");
      cg_printf("hash = %s ? %lldLL : %lldLL;\n",
                var.c_str(), 
                firstTrueCaseHash,
                firstFalseCaseHash);
      break;
    case Type::KindOfInt64:
      cg_printf("needsOrder = false;\n");
      cg_printf("hash = %s == 0 ? %lldLL : %s;\n", 
                var.c_str(),
                firstZeroCaseHash,
                var.c_str());
      break;
    case Type::KindOfDouble:
      cg_printf("needsOrder = false;\n");
      cg_printf("hash = %s == 0 ? %lldLL : ((int64)%s);\n", 
                var.c_str(),
                firstZeroCaseHash,
                var.c_str());
      break;
    default:
      cg_printf("hash = %s.hashForStringSwitch("
                "%lldLL, %lldLL, %lldLL, %lldLL, %lldLL, %lldLL, needsOrder);\n",
                var.c_str(),
                firstTrueCaseHash,
                firstNullCaseHash,
                firstFalseCaseHash,
                firstZeroCaseHash,
                firstHash,
                noMatchHash);
      break;
    }

    cg_printf("switch (((uint64) hash) & %lluUL) {\n", tableSize - 1);

    // need to precompute which hash labels will be jumped to,
    // thanks to no __attribute__((unused)) decls allowed after labels:
    // http://gcc.gnu.org/bugzilla/show_bug.cgi?id=11613
    size_t bucketIdx = 0;
    MapIntToStatementPtrWithPosVec::iterator it(caseMap.begin());
    set<int> willJumpTo;
    for (; it != caseMap.end(); ++it, ++bucketIdx) {
      StatementPtrWithPosVecPtr cases = it->second;
      StatementPtrWithPosVec::iterator caseIt(cases->begin());
      for (; caseIt != cases->end(); ++caseIt) {
        StatementPtrWithPos p = *caseIt;
        if (p.first < maxHashCase) { 
          int c = p.first + 1;
          while (defaultCases.find(c) != defaultCases.end()) c++;
          ASSERT(c <= maxHashCase);
          if (caseIt + 1 == cases->end() || 
              (*(caseIt + 1)).first != c) {
            willJumpTo.insert(c);
          }
        }
      }
    }

    bucketIdx = 0;
    it = caseMap.begin();
    for (; it != caseMap.end(); ++it, ++bucketIdx) {
      uint64 bucket = it->first;
      StatementPtrWithPosVecPtr cases = it->second;
      
      size_t caseNum = 0;
      StatementPtrWithPosVec::iterator caseIt(cases->begin());
      for (; caseIt != cases->end(); ++caseIt, ++caseNum) {

        StatementPtrWithPos p = *caseIt;

        // emit case_h_s{i} label if necessary
        if (willJumpTo.find(p.first) != willJumpTo.end()) {
          cg_printf("case_%d_h_s%d:\n", varId, p.first);
        }
        if (caseNum == 0) {
          // emit bucket case label
          cg_printf("case %lluUL:\n", bucket);
        }

        ASSERT(p.second->getCondition());

        // emit equality check
        cg.indentBegin("");
        p.second->getCondition()->outputCPPBegin(cg, ar);
        cg_printf("if (equal(%s, (", var.c_str());
        p.second->getCondition()->outputCPP(cg, ar);
        cg_printf("))) goto case_%d_%d;\n", varId, p.first);
        p.second->getCondition()->outputCPPEnd(cg, ar);

        // emit jump check if necessary
        if (p.first < maxHashCase) { 
          int c = p.first + 1;
          while (defaultCases.find(c) != defaultCases.end()) c++;
          ASSERT(c <= maxHashCase);
          
          // see if the next hash case to jump to 
          // is the next in the bucket chain, if so, no need to jump
          if (caseIt + 1 == cases->end() || 
              (*(caseIt + 1)).first != c) {
            ASSERT(willJumpTo.find(c) != willJumpTo.end());
            cg_printf("if (UNLIKELY(needsOrder)) goto case_%d_h_s%d;\n",
                      varId, c);
          }
        }
        cg.indentEnd("");
      }
      // emit jump to default:
      cg.indentBegin("");
      if (defaultCaseNum >= 0) {
        cg_printf("goto case_%d_%d;\n", varId, defaultCaseNum);
      } else {
        cg_printf("goto break%d;\n", labelId);
      }
      cg.indentEnd("");
    }

    if (defaultCaseNum >= 0) {
      cg_printf("default: goto case_%d_%d;\n", varId, defaultCaseNum);
    } else {
      cg_printf("default: goto break%d;\n", labelId);
      cg.addLabelId("break", labelId);
    }

    cg_printf("}\n");

    if (closeBrace)
      cg_indentEnd("}\n");

    // now emit the cases
    if (m_cases) {
      for (int i = 0; i < m_cases->getCount(); i++) {
        CaseStatementPtr stmt =
          static_pointer_cast<CaseStatement>((*m_cases)[i]);
        stmt->outputCPPByNumber(cg, ar, varId,
                                !stmt->getCondition() && defaultCaseNum != i ?
                                -1 : i);
      }
    }
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
      if (closeBrace)
        cg_indentEnd("}\n");
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
  cg.popBreakScope();
}
