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

#include <compiler/statement/statement.h>

using namespace HPHP;

///////////////////////////////////////////////////////////////////////////////

Statement::Statement(LocationPtr loc, KindOf kindOf)
  : Construct(loc), m_kindOf(kindOf), m_silencerCountMax(0),
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
    cg.indentBegin("{\n");
    cg.printf("Silencer ");
    for (int i = 0; i < m_silencerCountMax; ++i) {
      if (i != 0)
        cg.printf(", ");
      cg.printf("%s%d", Option::SilencerPrefix, i);
    }
    cg.printf(";\n");
  }
  outputCPPImpl(cg, ar);
  if (m_silencerCountMax > 0) {
    cg.indentEnd("}\n");
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


