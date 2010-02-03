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

#include <lib/statement/catch_statement.h>
#include <lib/analysis/analysis_result.h>
#include <lib/analysis/block_scope.h>
#include <lib/analysis/function_scope.h>
#include <lib/analysis/variable_table.h>
#include <lib/expression/modifier_expression.h>
#include <lib/analysis/code_error.h>
#include <util/util.h>
#include <lib/analysis/class_scope.h>
#include <lib/analysis/dependency_graph.h>
#include <lib/option.h>

using namespace HPHP;
using namespace std;
using namespace boost;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

CatchStatement::CatchStatement
(STATEMENT_CONSTRUCTOR_PARAMETERS,
 const std::string &className, const std::string &variable,
 StatementPtr stmt)
  : Statement(STATEMENT_CONSTRUCTOR_PARAMETER_VALUES),
    m_variable(variable), m_stmt(stmt), m_valid(true) {
  m_className = Util::toLower(className);
}

StatementPtr CatchStatement::clone() {
  CatchStatementPtr stmt(new CatchStatement(*this));
  stmt->m_stmt = Clone(m_stmt);
  return stmt;
}

///////////////////////////////////////////////////////////////////////////////
// parser functions

///////////////////////////////////////////////////////////////////////////////
// static analysis functions

void CatchStatement::analyzeProgram(AnalysisResultPtr ar) {
  addUserClass(ar, m_className);
  if (ar->isFirstPass()) {
    ar->getScope()->getVariables()->addReferenced(m_variable);
  }
  if (m_stmt) m_stmt->analyzeProgram(ar);
}

StatementPtr CatchStatement::preOptimize(AnalysisResultPtr ar) {
  ar->preOptimize(m_stmt);
  return StatementPtr();
}

StatementPtr CatchStatement::postOptimize(AnalysisResultPtr ar) {
  ar->postOptimize(m_stmt);
  return StatementPtr();
}

void CatchStatement::inferTypes(AnalysisResultPtr ar) {
  m_valid = true;

  ClassScopePtr cls = ar->findClass(m_className);
  TypePtr type;
  if (!cls || cls->isRedeclaring()) {
    if (!cls && ar->isFirstPass()) {
      ConstructPtr self = shared_from_this();
      ar->getCodeError()->record(self, CodeError::UnknownClass, self);
    }
    m_valid = false;
    type = NEW_TYPE(Object);
  } else {
    type = Type::CreateObjectType(m_className);
  }

  BlockScopePtr scope = ar->getScope();
  VariableTablePtr variables = scope->getVariables();
  variables->add(m_variable, type, false, ar, shared_from_this(),
                 ModifierExpressionPtr(), false);
  if (ar->isFirstPass()) {
    FunctionScopePtr func = dynamic_pointer_cast<FunctionScope>(scope);
    if (func && variables->isParameter(m_variable)) {
      variables->addLvalParam(m_variable);
    }
  }
  if (m_stmt) m_stmt->inferTypes(ar);
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void CatchStatement::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  cg.printf(" catch (%s $%s) ", m_className.c_str(), m_variable.c_str());
  cg.indentBegin("{\n");
  if (m_stmt) m_stmt->outputPHP(cg, ar);
  cg.indentEnd("}");
}

void CatchStatement::outputCPP(CodeGenerator &cg, AnalysisResultPtr ar) {
  if (m_valid) {
    cg.indentBegin("if (e.instanceof(\"%s\")) {\n", m_className.c_str());
    VariableTablePtr variables = ar->getScope()->getVariables();
    cg.printf("%s = e;\n", variables->getVariableName(ar, m_variable).c_str());
    if (m_stmt) m_stmt->outputCPP(cg, ar);
    cg.indentEnd("}");
  } else {
    cg.indentBegin("if (true) {\n");
    cg.printf("throw_fatal(\"unknown class %s\");\n", m_className.c_str());
    cg.indentEnd("}");
  }
}
