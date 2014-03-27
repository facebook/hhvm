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

#ifndef incl_HPHP_SELECT_REWRITERS_H_
#define incl_HPHP_SELECT_REWRITERS_H_

#include "hphp/compiler/expression/expression.h"
#include <vector>
#include "hphp/compiler/expression/expression_list.h"
#include "hphp/compiler/expression/query_expression.h"
#include "hphp/compiler/expression/simple_query_clause.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/**
 * A common base class for ClientSideSelectRewriter and ServerSideSelectRewriter
 */
class SelectRewriter {
public:
  void rewriteQuery(QueryExpressionPtr ep);

protected:
  /**
   * Records whether this rewriter is rewriting select clauses for
   * use by the query server or the query client.
   */
  bool m_serverSide = false;

  /**
   * For use by ClientSideSelectRewriter. See getClientSideSelectClause.
   */
  SelectClausePtr m_clientSideSelectClause;

private:
  void rewrite(ExpressionPtr ep);
  void rewriteSelect(SelectClausePtr sc);
  void rewriteExpressionList(ExpressionListPtr l);
  void rewriteOther(ExpressionPtr ep);
  void rewriteClientSide(ExpressionPtr sc);
  void collectSelectedColumns(ExpressionPtr ep);
  bool isTableName(ExpressionPtr ep);
  bool isColumnName(ExpressionPtr ep);

  /**
   * A list of simple variables and object property expressions
   * that collects all of the database column names (and let clause
   * identifiers) that are used in the select clause that is being
   * rewritten.
   */
  ExpressionListPtr m_selectedColumns;

  /**
   * A list of the names introduced by the entire query expression.
   * These names are bound to tables or query results, or have their values
   * specified by let clauses. They only have meaning to the query processor
   * of the database server. Expressions that involve these variables will
   * end up in the serialized query that is sent to the query provider.
   */
  std::vector<std::string> m_boundVars;
};

/**
 * A rewriter for query expressions that rewrites the select clause
 * so that any references to database columns are replaced with
 * references to synthetic object properties.
 * The clause will become part of lambda that transforms elements of the result
 * of the query execution. I.e. used to transform a stream of objects made up
 * of the selected database columns, into the stream of PHP values specified
 * by the original select clause.
 */
class ClientSideSelectRewriter : public SelectRewriter {
public:

  /**
   * The select clause that becomes part of the lambda function
   * that post processes the query result obtained from the
   * server, which may be out of process, into values (such as objects)
   * that are tied to the client side of the query.
   */
  SelectClausePtr getClientSideSelectClause() {
    return m_clientSideSelectClause;
  }

};

/**
 * A rewriter for query expressions that rewrites the select clause so that
 * it only contain references to database columns (and let clause variables).
 * The rewritten clause becomes part of the rewritten query and is sent
 * to the query provider, which then does not have to deal with any
 * expressions that need to be evaluated in the context of the query expression.
 */
class ServerSideSelectRewriter : public SelectRewriter {
public:
  ServerSideSelectRewriter() {
    m_serverSide = true;
  }
};

}
#endif // incl_HPHP_SELECT_REWRITERS_H_
