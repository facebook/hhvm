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

#include "hphp/compiler/statement/goto_statement.h"
#include "hphp/compiler/analysis/function_scope.h"

using namespace HPHP;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

GotoStatement::GotoStatement
(STATEMENT_CONSTRUCTOR_PARAMETERS, const std::string &label)
  : Statement(STATEMENT_CONSTRUCTOR_PARAMETER_VALUES(GotoStatement)),
    m_label(label), m_error((ParserBase::GotoError)0), m_id(0) {
}

StatementPtr GotoStatement::clone() {
  GotoStatementPtr stmt(new GotoStatement(*this));
  stmt->m_label = m_label;
  stmt->m_error = m_error;
  return stmt;
}

///////////////////////////////////////////////////////////////////////////////
// parser functions
void GotoStatement::invalidate(ParserBase::GotoError error) {
  m_error = error;
  switch (m_error) {
  case ParserBase::UndefLabel:
    Compiler::Error(Compiler::GotoUndefLabel, shared_from_this());
    break;
  case ParserBase::InvalidBlock:
    Compiler::Error(Compiler::GotoInvalidBlock, shared_from_this());
    break;
  default:
    assert(false);
    break;
  }
}

///////////////////////////////////////////////////////////////////////////////
// static analysis functions

void GotoStatement::analyzeProgram(AnalysisResultPtr ar) {
  if (ar->getPhase() == AnalysisResult::AnalyzeAll) {
    FunctionScopeRawPtr fs = getFunctionScope();
    if (fs) fs->setHasGoto();
  }
}

ConstructPtr GotoStatement::getNthKid(int n) const {
  switch (n) {
    default:
      assert(false);
      break;
  }
  return ConstructPtr();
}

int GotoStatement::getKidCount() const {
  return 0;
}

void GotoStatement::setNthKid(int n, ConstructPtr cp) {
  switch (n) {
    default:
      assert(false);
      break;
  }
}

void GotoStatement::inferTypes(AnalysisResultPtr ar) {
}

///////////////////////////////////////////////////////////////////////////////

void GotoStatement::outputCodeModel(CodeGenerator &cg) {
  cg.printObjectHeader("GotoStatement", 2);
  cg.printPropertyHeader("label");
  cg.printValue(m_label);
  cg.printPropertyHeader("sourceLocation");
  cg.printLocation(this->getLocation());
  cg.printObjectFooter();
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void GotoStatement::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  cg_printf("goto %s;\n", m_label.c_str());
}

