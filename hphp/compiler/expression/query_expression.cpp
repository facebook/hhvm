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

#include "hphp/compiler/expression/query_expression.h"
#include <vector>
#include "hphp/compiler/expression/modifier_expression.h"
#include "hphp/compiler/expression/parameter_expression.h"
#include "hphp/compiler/expression/simple_query_clause.h"
#include "hphp/compiler/statement/function_statement.h"
#include "hphp/compiler/statement/return_statement.h"
#include "hphp/compiler/statement/statement_list.h"
#include "hphp/compiler/analysis/capture_extractor.h"
#include "hphp/compiler/analysis/code_error.h"
#include "hphp/compiler/analysis/file_scope.h"
#include "hphp/compiler/analysis/select_rewriters.h"
#include "hphp/runtime/base/complex-types.h"

using namespace HPHP;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

QueryOrderby::QueryOrderby(EXPRESSION_CONSTRUCTOR_BASE_PARAMETERS)
  : Expression(EXPRESSION_CONSTRUCTOR_BASE_PARAMETER_VALUES) {
  m_expressions = nullptr;
}

ExpressionPtr QueryOrderby::clone() {
  assert(false);
  return nullptr;
}

QueryExpression::QueryExpression(EXPRESSION_CONSTRUCTOR_PARAMETERS,
                                 ExpressionPtr head, ExpressionPtr body)
  : QueryOrderby(EXPRESSION_CONSTRUCTOR_PARAMETER_VALUES(QueryExpression)) {
  m_expressions = ExpressionListPtr(
    new ExpressionList(getScope(), getLocation())
  );
  m_expressions->addElement(head);

  assert(body != nullptr && body->is(Expression::KindOfExpressionList));
  ExpressionListPtr el(static_pointer_cast<ExpressionList>(body));
  for (unsigned int i = 0; i < el->getCount(); i++) {
    if ((*el)[i]) m_expressions->addElement((*el)[i]);
  }
  m_originalExpressions = m_expressions;
}

QueryExpression::QueryExpression(EXPRESSION_CONSTRUCTOR_PARAMETERS,
                                 ExpressionListPtr clauses)
  : QueryOrderby(EXPRESSION_CONSTRUCTOR_PARAMETER_VALUES(QueryExpression)) {
  m_expressions = clauses;
  m_originalExpressions = m_expressions;
}

///////////////////////////////////////////////////////////////////////////////
// parser functions

void QueryExpression::doRewrites(AnalysisResultPtr ar,
  FileScopePtr fileScope) {
  // Clone the query expression for client rewriting later on.
  auto csqe = static_pointer_cast<QueryExpression>(this->clone());

  // Find expressions that capture client state and turn them into
  // query parameter expression. The original expressions end up in m_queryargs.
  CaptureExtractor ce;
  auto qe = static_pointer_cast<QueryExpression>(
    ce.rewrite(static_pointer_cast<Expression>(shared_from_this())));

  // Expose the argument (capture) expressions to static analysis rather than
  // the original clause list (it is still available in m_originalExpressions).
  m_expressions = static_pointer_cast<ExpressionList>(m_expressions->clone());
  m_expressions->clearElements();
  auto queryArgs = static_pointer_cast<ExpressionList>(m_expressions->clone());
  for (auto e : ce.getCapturedExpressions()) {
    queryArgs->addElement(e);
  }
  assert(queryArgs->getCount() > 0); //syntax requires an initial from clause
  m_expressions->addElement(queryArgs);

  // Rewrite select clauses so that they become tuples of columns when
  // sent to the query provider.
  ServerSideSelectRewriter se;
  se.rewriteQuery(qe);

  // Serialize the rewritten query expression so that it can be sent to
  // the query provider as a string which it can unserialize and turn
  // into a query (unless already cached).
  std::ostringstream serialized;
  CodeGenerator cg(&serialized, CodeGenerator::Output::CodeModel);
  cg.setAstClassPrefix("Code");
  qe->outputCodeModel(cg);
  std::string s(serialized.str().c_str(), serialized.str().length());
  m_querystr = makeStaticString(s);

  // start again with the clone of the original query (not the one
  // that was rewritten for use by the query provider) and rewrite
  // it into a closure expression that is used by the query provider
  // to execute code in the context to the client, not the server.
  auto selectClosure = csqe->clientSideRewrite(ar, fileScope);
  if (selectClosure != nullptr) {
    m_expressions->addElement(selectClosure);
  }
}

// Rewrite the outermost select clause so that it references properties of
// a result tuple constructed by the query provider. Then wrap this expression
// in a lambda so that the query provider can invoke it as a call back every
// time it produces a result tuple (row).
ClosureExpressionPtr QueryExpression::clientSideRewrite(AnalysisResultPtr ar,
  FileScopePtr fileScope) {
  // Rewrite the select expression into an expression that refers to
  // table columns (including computed columns) via properties of an
  // object produced by the query provider at runtime.
  ClientSideSelectRewriter cs;
  cs.rewriteQuery(static_pointer_cast<QueryExpression>(shared_from_this()));
  auto csSelect = cs.getClientSideSelectClause();
  // null if there is no select clause.
  if (csSelect == nullptr) return nullptr;
  ExpressionPtr selectExpr = csSelect->getExpression();

  // Now wrap up the rewritten expression into a lambda expression that
  // is passed to the query provider. When the query result is iterated,
  // the closure is called for each row in the query result in order to
  // produce the value specified by this select expression.

  // Create a return statement for the lambda body
  auto location = this->getLocation();
  LabelScopePtr labelScope(new LabelScope());
  ReturnStatementPtr returnStatement(
    new ReturnStatement(BlockScopePtr(), labelScope, location, selectExpr)
  );

  // Wrap up the return statement in a list for the lambda body
  StatementListPtr stmt(
    new StatementList(BlockScopePtr(), labelScope, location)
  );
  stmt->addElement(returnStatement);

  // Create a function statement for the lambda:

  // First create a formal parameter list, consisting of a single
  // parameter that will receive an object from the query provider
  // with a property for each table column that is referenced in the
  // expression of this select clause.
  TypeAnnotationPtr type;
  bool hhType = true;
  std::string paramName = "__query_result_row__";
  bool byRefParam = false;
  TokenID modifier = 0;
  ExpressionPtr defaultValue;
  ExpressionPtr attributeList;
  ParameterExpressionPtr parameter (
    new ParameterExpression(BlockScopePtr(), location, type, hhType,
        paramName, byRefParam, modifier, defaultValue, attributeList)
  );
  ExpressionListPtr params(new ExpressionList(BlockScopePtr(), location));
  params->addElement(parameter);

  // Now create a function statement object
  ModifierExpressionPtr modifiers(
    new ModifierExpression(BlockScopePtr(), location)
  );
  bool ref = false;
  static int counter = 0;
  std::string name = "__select__#" + std::to_string(counter++);
  TypeAnnotationPtr retTypeAnnotation;
  int attr = 0;
  std::string docComment;
  ExpressionListPtr attrList;
  FunctionStatementPtr func(
    new FunctionStatement(BlockScopePtr(), labelScope, location, modifiers, ref,
        name, params, retTypeAnnotation, stmt, attr, docComment, attrList)
  );

  // The function statement needs a scope
  std::vector<UserAttributePtr> uattrs;
  FunctionScopePtr funcScope
    (new FunctionScope(ar, false, name, func, false, 1, 1,
                       nullptr, attr, docComment, fileScope, uattrs));
  fileScope->addFunction(ar, funcScope);
  func->resetScope(funcScope, true);
  funcScope->setOuterScope(fileScope);

  // Now construct a closure expression to create the closure value to
  // pass to the query provider.
  ExpressionListPtr captures;
  ClosureExpressionPtr closure(
    new ClosureExpression(BlockScopePtr(), location, ClosureType::Short,
        func, captures)
  );
  closure->getClosureFunction()->setContainingClosure(closure);
  return closure;
}

///////////////////////////////////////////////////////////////////////////////
// static analysis functions

void QueryOrderby::analyzeProgram(AnalysisResultPtr ar) {
  for (unsigned int i = 0; i < m_expressions->getCount(); i++) {
    (*m_expressions)[i]->analyzeProgram(ar);
  }
}

ConstructPtr QueryOrderby::getNthKid(int n) const {
  if (n < (int)m_expressions->getCount()) {
    return (*m_expressions)[n];
  }
  return ConstructPtr();
}

int QueryOrderby::getKidCount() const {
  return m_expressions->getCount();
}

void QueryOrderby::setNthKid(int n, ConstructPtr cp) {
  int m = m_expressions->getCount();
  if (n >= m) {
    assert(false);
  } else {
    (*m_expressions)[n] = dynamic_pointer_cast<Expression>(cp);
  }
}

TypePtr QueryOrderby::inferTypes(AnalysisResultPtr ar, TypePtr type,
                                  bool coerce) {
  for (unsigned int i = 0; i < m_expressions->getCount(); i++) {
    if (ExpressionPtr e = (*m_expressions)[i]) {
      e->inferAndCheck(ar, Type::Some, false);
    }
  }
  return Type::Object;
}

///////////////////////////////////////////////////////////////////////////////

void QueryOrderby::outputCodeModel(CodeGenerator &cg) {
  if (this->getKindOf() == Expression::KindOfOrderbyClause) {
    cg.printObjectHeader("OrderbyClause", 2);
    cg.printPropertyHeader("orders");
  } else {
    cg.printObjectHeader("QueryExpression", 2);
    cg.printPropertyHeader("clauses");
  }
  cg.printExpressionVector(m_originalExpressions);
  cg.printPropertyHeader("sourceLocation");
  cg.printLocation(this->getLocation());
  cg.printObjectFooter();
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void QueryOrderby::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  if (this->getKindOf() == Expression::KindOfOrderbyClause) {
    cg_printf("orderby ");
  }
  for (unsigned int i = 0; i < m_originalExpressions->getCount(); i++) {
    if (ExpressionPtr e = (*m_originalExpressions)[i]) {
      e->outputPHP(cg, ar);
      if (i > 0) cg_printf(" ");
    }
  }

}

