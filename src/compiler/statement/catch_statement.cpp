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

#include <compiler/statement/catch_statement.h>
#include <compiler/analysis/analysis_result.h>
#include <compiler/analysis/block_scope.h>
#include <compiler/analysis/function_scope.h>
#include <compiler/analysis/variable_table.h>
#include <compiler/expression/modifier_expression.h>
#include <compiler/analysis/code_error.h>
#include <util/util.h>
#include <compiler/analysis/class_scope.h>
#include <compiler/analysis/dependency_graph.h>
#include <compiler/option.h>

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
  m_originalClassName = className;
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

void CatchStatement::analyzeProgramImpl(AnalysisResultPtr ar) {
  addUserClass(ar, m_className);
  if (ar->isFirstPass()) {
    ar->getScope()->getVariables()->addUsed(m_variable);
  }
  if (m_stmt) m_stmt->analyzeProgram(ar);
}

ConstructPtr CatchStatement::getNthKid(int n) const {
  switch (n) {
    case 0:
      return m_stmt;
    default:
      ASSERT(false);
      break;
  }
  return ConstructPtr();
}

int CatchStatement::getKidCount() const {
  return 1;
}

void CatchStatement::setNthKid(int n, ConstructPtr cp) {
  switch (n) {
    case 0:
      m_stmt = boost::dynamic_pointer_cast<Statement>(cp);
      break;
    default:
      ASSERT(false);
      break;
  }
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
  ClassScopePtr cls = ar->findClass(m_className);
  TypePtr type;
  m_valid = cls;
  if (!m_valid && ar->isFirstPass()) {
    ConstructPtr self = shared_from_this();
    Compiler::Error(Compiler::UnknownClass, self);
  }

  // This can never be a specific exception type, because a future exception
  // class may be re-declaring, then generated code like this won't work with
  // DynamicObjectData: p_exception v_e = e;
  type = Type::Object;

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
  cg_printf(" catch (%s $%s) ", m_originalClassName.c_str(),
            m_variable.c_str());
  cg_indentBegin("{\n");
  if (m_stmt) m_stmt->outputPHP(cg, ar);
  cg_indentEnd("}");
}

void CatchStatement::outputCPPImpl(CodeGenerator &cg, AnalysisResultPtr ar) {
  if (m_valid) {
    cg_printf("if (e.instanceof(");
    cg_printString(m_className, ar);
    cg_indentBegin(")) {\n");
    VariableTablePtr variables = ar->getScope()->getVariables();
    string name = variables->getVariableName(cg, ar, m_variable);
    cg_printf("%s = e;\n", name.c_str());
  } else {
    cg_indentBegin("if (false) {\n");
  }
  if (m_stmt) m_stmt->outputCPP(cg, ar);
  cg_indentEnd("}");
}
