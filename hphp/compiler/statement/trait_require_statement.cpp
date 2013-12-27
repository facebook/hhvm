/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/compiler/statement/trait_require_statement.h"
#include "hphp/compiler/statement/class_statement.h"
#include "hphp/compiler/analysis/class_scope.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

TraitRequireStatement::TraitRequireStatement
(STATEMENT_CONSTRUCTOR_PARAMETERS,
 const std::string &required,
 bool isExtends)
    : Statement(STATEMENT_CONSTRUCTOR_PARAMETER_VALUES(TraitRequireStatement)),
      m_extends(isExtends), m_required(required) {
}

StatementPtr TraitRequireStatement::clone() {
  TraitRequireStatementPtr new_stmt(new TraitRequireStatement(*this));
  return new_stmt;
}


///////////////////////////////////////////////////////////////////////////////
// parser functions

void TraitRequireStatement::onParseRecur(AnalysisResultConstPtr ar,
                                         ClassScopePtr scope) {
  if (!scope->isTrait()) {
    parseTimeFatal(Compiler::InvalidTraitStatement,
                   "Only traits can require in class scope");
  }

  ar->parseOnDemandByClass(Util::toLower(m_required));
  scope->addTraitRequirement(m_required, m_extends);
}


///////////////////////////////////////////////////////////////////////////////
// static analysis functions

void TraitRequireStatement::analyzeProgram(AnalysisResultPtr ar) {}

ConstructPtr TraitRequireStatement::getNthKid(int n) const {
  always_assert(false);
  return ConstructPtr();
}

int TraitRequireStatement::getKidCount() const {
  return 0;
}

void TraitRequireStatement::setNthKid(int n, ConstructPtr cp) {
  always_assert(false);
}

void TraitRequireStatement::inferTypes(AnalysisResultPtr ar) {
}

///////////////////////////////////////////////////////////////////////////////

void TraitRequireStatement::outputCodeModel(CodeGenerator &cg) {
  // TODO t3429526: add Code Model counterpart objects for this statement ;
  // should depend on whether any runtime behavior changes for this syntax
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void TraitRequireStatement::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  cg_printf("require %s %s;\n",
            m_extends ? "extends " : "implements ",
            m_required.c_str());
}

}
