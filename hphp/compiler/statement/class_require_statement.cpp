/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/compiler/statement/class_require_statement.h"
#include "hphp/compiler/statement/class_statement.h"
#include "hphp/compiler/analysis/class_scope.h"
#include "hphp/compiler/code_model_enums.h"
#include "hphp/util/text-util.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

ClassRequireStatement::ClassRequireStatement
(STATEMENT_CONSTRUCTOR_PARAMETERS,
 const std::string &required,
 bool isExtends)
    : Statement(STATEMENT_CONSTRUCTOR_PARAMETER_VALUES(ClassRequireStatement)),
      m_extends(isExtends), m_required(required) {
}

StatementPtr ClassRequireStatement::clone() {
  ClassRequireStatementPtr new_stmt(new ClassRequireStatement(*this));
  return new_stmt;
}


///////////////////////////////////////////////////////////////////////////////
// parser functions

void ClassRequireStatement::onParseRecur(AnalysisResultConstPtr ar,
                                         FileScopeRawPtr fs,
                                         ClassScopePtr scope) {
  if (!scope->isTrait() && !scope->isInterface()) {
    parseTimeFatal(fs,
                   Compiler::InvalidTraitStatement,
                   "Only traits and interfaces may use 'require' in class scope");
  }
  if (scope->isInterface() && !m_extends) {
    parseTimeFatal(
      fs,
      Compiler::InvalidTraitStatement,
      "'require implements' may not be used in interface scope"
      "; instead, use interface inheritance");
  }

  ar->parseOnDemandByClass(toLower(m_required));
  scope->addClassRequirement(m_required, m_extends);
}


///////////////////////////////////////////////////////////////////////////////
// static analysis functions

void ClassRequireStatement::analyzeProgram(AnalysisResultPtr ar) {}

ConstructPtr ClassRequireStatement::getNthKid(int n) const {
  always_assert(false);
  return ConstructPtr();
}

int ClassRequireStatement::getKidCount() const {
  return 0;
}

void ClassRequireStatement::setNthKid(int n, ConstructPtr cp) {
  always_assert(false);
}

///////////////////////////////////////////////////////////////////////////////

void ClassRequireStatement::outputCodeModel(CodeGenerator &cg) {
  cg.printObjectHeader("ClassRequiresStatement", 3);
  cg.printPropertyHeader("name");
  cg.printValue(m_required);
  cg.printPropertyHeader("kind");
  if (m_extends) {
    cg.printValue(PHP_EXTENDS);
  } else {
    cg.printValue(PHP_IMPLEMENTS);
  }
  cg.printPropertyHeader("sourceLocation");
  cg.printLocation(this);
  cg.printObjectFooter();
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void ClassRequireStatement::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  cg_printf("require %s %s;\n",
            m_extends ? "extends " : "implements ",
            m_required.c_str());
}

}
