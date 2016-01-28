/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/compiler/statement/declare_statement.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

DeclareStatement::DeclareStatement(STATEMENT_CONSTRUCTOR_PARAMETERS)
  : Statement(STATEMENT_CONSTRUCTOR_PARAMETER_VALUES(DeclareStatement))
{}

StatementPtr DeclareStatement::clone() {
  return std::make_shared<DeclareStatement>(*this);
}

//////////////////////////////////////////////////////////////////////

ConstructPtr DeclareStatement::getNthKid(int n) const { always_assert(0); }
int DeclareStatement::getKidCount() const { return 0; }
void DeclareStatement::setNthKid(int n, ConstructPtr cp) { always_assert(0); }

//////////////////////////////////////////////////////////////////////

void DeclareStatement::analyzeProgram(AnalysisResultPtr) {}
void DeclareStatement::outputPHP(CodeGenerator& cg, AnalysisResultPtr ar) {
  if (m_block) m_block->outputPHP(cg, ar);
}

//////////////////////////////////////////////////////////////////////

}
