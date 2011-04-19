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

#include <compiler/statement/statement.h>

using namespace HPHP;

///////////////////////////////////////////////////////////////////////////////

#define DEC_STMT_NAMES(x) #x
const char *Statement::Names[] = {
  DECLARE_STATEMENT_TYPES(DEC_STMT_NAMES)
};

Statement::Statement(STATEMENT_CONSTRUCTOR_PARAMETERS)
    : Construct(scope, loc), m_kindOf(kindOf), m_silencerCountMax(0),
      m_silencerCountCurrent(0) {
}

///////////////////////////////////////////////////////////////////////////////
// parser functions

void Statement::addElement(StatementPtr stmt) {
  ASSERT(false);
}

void Statement::insertElement(StatementPtr stmt, int index /* = 0 */) {
  ASSERT(false);
}

void Statement::analyzeProgram(AnalysisResultPtr ar) {
  ar->pushStatement(boost::dynamic_pointer_cast<Statement>(shared_from_this()));
  analyzeProgramImpl(ar);
  ar->popStatement();
}

void Statement::outputCPP(CodeGenerator &cg, AnalysisResultPtr ar) {
  if (m_silencerCountMax > 0) {
    cg_indentBegin("{\n");
    cg_printf("Silencer ");
    for (int i = 0; i < m_silencerCountMax; ++i) {
      if (i != 0)
        cg_printf(", ");
      cg_printf("%s%d", Option::SilencerPrefix, i);
    }
    cg_printf(";\n");
  }
  outputCPPImpl(cg, ar);
  if (m_silencerCountMax > 0) {
    cg_indentEnd("}\n");
  }
}

int Statement::requireSilencers(int count) {
  int ret = m_silencerCountCurrent;
  m_silencerCountCurrent += count;
  if (m_silencerCountMax < m_silencerCountCurrent) {
    m_silencerCountMax = m_silencerCountCurrent;
  }
  return ret;
}
void Statement::endRequireSilencers(int old) {
  m_silencerCountCurrent = old;
}
int Statement::getSilencerCount() {
  return m_silencerCountMax;
}

bool Statement::hasReachableLabel() const {
  switch (getKindOf()) {
    case KindOfMethodStatement:
    case KindOfFunctionStatement:
    case KindOfClassStatement:
    case KindOfInterfaceStatement:
      // dont recur into declarations
      return false;
    case KindOfForStatement:
    case KindOfForEachStatement:
    case KindOfWhileStatement:
    case KindOfDoStatement:
      // a label inside a loop cant be reached from outside the loop
      return false;
    case KindOfLabelStatement:
      return true;
    default:
      break;
  }
  for (int i = getKidCount(); i--; ) {
    StatementPtr child(getNthStmt(i));
    if (child && child->hasReachableLabel()) return true;
  }
  return false;
}


