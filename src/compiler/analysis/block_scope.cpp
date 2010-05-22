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

#include <compiler/analysis/block_scope.h>
#include <compiler/analysis/analysis_result.h>
#include <compiler/statement/statement_list.h>
#include <compiler/analysis/variable_table.h>
#include <compiler/analysis/constant_table.h>

using namespace HPHP;

///////////////////////////////////////////////////////////////////////////////

BlockScope::BlockScope(const std::string &name, const std::string &docComment,
                       StatementPtr stmt, KindOf kind)
  : m_docComment(docComment), m_stmt(stmt),
    m_kind(kind), m_loopNestedLevel(0), m_incLevel(0) {
  m_name = Util::toLower(name);
  m_variables = VariableTablePtr(new VariableTable(*this));
  m_constants = ConstantTablePtr(new ConstantTable(*this));
  SymbolTable::AllSymbolTables.push_back(m_variables);
  SymbolTable::AllSymbolTables.push_back(m_constants);
}

void BlockScope::incLoopNestedLevel() {
  m_loopNestedLevel++;
}

void BlockScope::decLoopNestedLevel() {
  ASSERT(m_loopNestedLevel > 0);
  m_loopNestedLevel--;
}

ModifierExpressionPtr
BlockScope::setModifiers(ModifierExpressionPtr modifiers) {
  ModifierExpressionPtr oldModifiers = m_modifiers;
  m_modifiers = modifiers;
  return oldModifiers;
}

void BlockScope::addMovableInclude(StatementPtr include) {
  if (!m_includes) {
    m_includes = StatementListPtr
      (new StatementList(LocationPtr(), Statement::KindOfStatementList));
  }
  m_includes->addElement(include);
}

void BlockScope::inferTypes(AnalysisResultPtr ar) {
  if (m_stmt) {
    ar->pushScope(shared_from_this());
    m_stmt->inferTypes(ar);
    ar->popScope();
  }
}

void BlockScope::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  m_constants->outputPHP(cg, ar);
  m_variables->outputPHP(cg, ar);
}

void BlockScope::outputCPP(CodeGenerator &cg, AnalysisResultPtr ar) {
  m_constants->outputCPP(cg, ar);
  m_variables->outputCPP(cg, ar);
}
