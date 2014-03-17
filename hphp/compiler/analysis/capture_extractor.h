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

#ifndef incl_HPHP_CAPTURE_EXTRACTOR_H_
#define incl_HPHP_CAPTURE_EXTRACTOR_H_

#include "hphp/compiler/expression/binary_op_expression.h"
#include <vector>
#include "hphp/compiler/expression/expression.h"
#include "hphp/compiler/expression/object_property_expression.h"
#include "hphp/compiler/expression/query_expression.h"
#include "hphp/compiler/expression/simple_function_call.h"
#include "hphp/compiler/expression/simple_query_clause.h"
#include "hphp/compiler/expression/simple_variable.h"
#include "hphp/compiler/expression/unary_op_expression.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/** A rewriter for query expressions that capture variables that lie
 *  outside of the scope of the query expression. Subexpressions that
 *  contain such references are turned into references to query parameter
 *  variables. The original expressions are collected into the list
 *  returned by getCapturedEpressions. Their runtime values are obtained
 *  before the query is evaluated and are passed as arguments to the query
 *  processor.
 */
class CaptureExtractor {
public:

  ExpressionPtr rewrite(ExpressionPtr ep);

  std::vector<ExpressionPtr> getCapturedExpressions() {
    return m_capturedExpressions;
  }

private:
  bool dependsOnQueryOnlyState(ExpressionPtr e);
  SimpleVariablePtr newQueryParamRef(ExpressionPtr ae);
  ExpressionPtr rewriteBinary(BinaryOpExpressionPtr be);
  ExpressionPtr rewriteCall(SimpleFunctionCallPtr sfc);
  ExpressionListPtr rewriteExpressionList(ExpressionListPtr l);
  ExpressionPtr rewriteObjectProperty(ObjectPropertyExpressionPtr ope);
  QueryExpressionPtr rewriteQuery(QueryExpressionPtr qe);
  SimpleQueryClausePtr rewriteSimpleClause(SimpleQueryClausePtr sc);
  SimpleVariablePtr rewriteSimpleVariable(SimpleVariablePtr sv);
  ExpressionPtr rewriteUnary(UnaryOpExpressionPtr ue);

  std::vector<ExpressionPtr> m_capturedExpressions;
  std::vector<std::string> m_boundVars;
};

}
#endif // incl_HPHP_CAPTURE_EXTRACTOR_H_
