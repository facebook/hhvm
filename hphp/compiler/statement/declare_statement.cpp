/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

ConstructPtr DeclareStatement::getNthKid(int n) const {
  switch (n) {
    case 0:
      return m_block;
    default:
      assert(false);
      break;
  }
  return ConstructPtr();
}

int DeclareStatement::getKidCount() const { return 1; }

void DeclareStatement::setNthKid(int n, ConstructPtr cp) {
  switch (n) {
    case 0:
      m_block = dynamic_pointer_cast<BlockStatement>(cp);
      break;
    default:
      assert(false);
      break;
  }
}

//////////////////////////////////////////////////////////////////////

void DeclareStatement::outputPHP(CodeGenerator& cg, AnalysisResultPtr ar) {
  if (m_block) m_block->outputPHP(cg, ar);
}

//////////////////////////////////////////////////////////////////////

}
