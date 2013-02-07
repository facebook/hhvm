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
#include "compiler/statement/typedef_statement.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

TypedefStatement::TypedefStatement(
    STATEMENT_CONSTRUCTOR_PARAMETERS,
    const std::string& name,
    const std::string& value)
  : Statement(STATEMENT_CONSTRUCTOR_PARAMETER_VALUES(TypedefStatement))
  , name(name)
  , value(value)
{}

TypedefStatement::~TypedefStatement() {}

StatementPtr TypedefStatement::clone() {
  return StatementPtr(new TypedefStatement(*this));
}

//////////////////////////////////////////////////////////////////////

ConstructPtr TypedefStatement::getNthKid(int n) const {
  always_assert(0);
}

int TypedefStatement::getKidCount() const {
  return 0;
}

void TypedefStatement::setNthKid(int n, ConstructPtr cp) {
  always_assert(0);
}

//////////////////////////////////////////////////////////////////////

void TypedefStatement::analyzeProgram(AnalysisResultPtr) {}

void TypedefStatement::inferTypes(AnalysisResultPtr) {}

void TypedefStatement::outputPHP(CodeGenerator& cg, AnalysisResultPtr ar) {
}

//////////////////////////////////////////////////////////////////////

}

