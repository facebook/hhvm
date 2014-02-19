<?hh //strict
// Copyright 2004-present Facebook. All Rights Reserved.

use HH\CodeModel as CM;

/**
 * Provides the ability to use Language INtegrated Query expressions
 * to query data kept in a SQLite3 database.
 */
class LINQToSQLite {
  private SQLite3 $sqliteConnection;

  /**
   * Given a connection to a SQLite3 database, create an object
   * that can be used construct and execute queries using PHP LINQ.
   */
  public function __construct(SQLite3 $sqlite_connection) {
    $this->sqliteConnection = $sqlite_connection;
  }

  /**
   * Returns a SQLiteTable instance that can be used as from/join
   * collections in a PHP LINQ expressions. The valid of the table
   * name is not checked by this call. If invalid, a runtime error
   * occurs when a query involving this table is executed.
   */
  public function getTable(string $table_name) : SQLiteTable {
    return new SQLiteTable($this->sqliteConnection, $table_name);
  }

  //todo: consider methods for creating and editing tables
  //i.e. can we make the SQLite3 class an implementation detail?
}

/**
 * Represents a table in a SQLite3 database. Instances of this class
 * typically are used as the from/join collections in PHP LINQ expressions.
 */
class SQLiteTable {
  // A connection to the database that contains this table.
  private SQLite3 $sqliteConnection;
  // The name of the table.
  private string $tableName;

  /**
   * Constructs an object representing the table named $table_name
   * in the SQLite3 database that is connected to via $sqlite_connection.
   */
  public function __construct(SQLite3 $sqlite_connection, string $table_name) {
    $this->sqliteConnection = $sqlite_connection;
    $this->tableName = $table_name;
  }

  /*
   * Returns a SQLiteQuery instance that represents the result of executing
   * a PHP LINQ query expression. The instance can be used to iterate
   * over the query results, or it can be composed with other instances
   * to form more complex queries, for example unions.
   *
   * The SQL query does not get sent to the SQLite3 query engine until the
   * query instance has been asked for an element of the result stream.
   */
  public function executeQuery(/*args*/) : SQLiteQuery {
    $args = func_get_args();
    $serialized_ast = array_shift($args);
    $callback = array_shift($args);
    array_unshift($args, $this);
    return new SQLiteQuery(
      $this->sqliteConnection, $serialized_ast, $callback, $args);
  }

  /**
   * Returns the name of this table. Do not assume that a table
   * with this name actually exists in the connected database
   * unless there was a prior call to $this->validate().
   */
  public function getTableName() : string {
    return $this->tableName;
  }

  /**
   * Throws an exception if the connected database does not have a table named
   * $this->getTableName(). Use this function before injecting
   * $this->getTableName() into a SQL command.
   */
  public function validate() {
    $name = str_replace("'", "''", $this->tableName);
    $name = "'$name'";

    $statement = $this->sqliteConnection->prepare(
      "select exists(SELECT * FROM sqlite_master WHERE type='table' ".
      "AND name = $name)"
    );
    $valid = $statement->execute()->fetcharray(SQLITE3_NUM)[0];
    if (!$valid) {
      throw new LINQToSQLiteException("bad table name: $name");
    }
  }

}

/**
 * This class encapsulates the result of executing a PHP LINQ query
 * epxression. The result is "lazy" in as much as the corresponding
 * SQLite query does not execute until data is demanded from the query
 * instance via, for example, a call to getIterator.
 *
 * It is perfectly possible for a query to be an intermediate object
 * that is never asked for its data. Such cases arise when the query
 * is composed with another query. For example, two queries may be
 * composed into a single query via a union operation.
 */
class SQLiteQuery implements IteratorAggregate{
  // when a query occurs inside a loop (perish the thought) we don't
  // want to compile it every time through the loop, so we cache queries here.
  // The cache is static since we want different instances of SQLiteQuery
  // to share the same compiled SQLite query.
  // We need different instances because each instance captures client
  // state that is specific to the particular call ocurrence.
  protected static $compiledQueries = Map {};

  // A connection to the database that this query gets its results from.
  protected SQLite3 $sqliteConnection;

  // A string that can be unserialized into an object model for this query.
  protected string $serializedAst;

  // A closure that is called for each tuple in the SQL query result
  // in order to obtain a PHP LINQ query expression result element.
  protected (function(mixed) : mixed) $callback;

  // The runtime values of expressions that appear in the original
  // PHP LINQ query, but which cannot be evaluated by the SQL query
  // engine. The query serialized in $serialized_ast will refer to
  // these expressions as @query_param_0, @query_param_1, and so on.
  protected array $arguments;

  // The number of result rows to discard.
  protected int $offset = -1;

  // The maximum number of result rows to return.
  protected int $limit = -1;

  /**
   * Constructs a SQLiteQuery instance to return as the result of
   * executing a PHP LINQ query expression.
   *
   * @param @sqlite_connection
   * A connection to the database that this query gets its results from.
   *
   * @param @serialized_ast
   * A string that can be unserialized into an object model for this query.
   *
   * @param @callback
   * A closure that is called for each tuple in the SQL query result
   * in order to obtain a PHP LINQ query expression result element.
   *
   * @param @arguments
   * The runtime values of expressions that appear in the original
   * PHP LINQ query, but which cannot be evaluated by the SQL query
   * engine. The query serialized in $serialized_ast will refer to
   * these expressions as @query_param_0, @query_param_1, and so on.
   */
  public function __construct(
    SQLite3 $sqlite_connection,
    string $serialized_ast,
    (function(mixed) : mixed) $callback,
    array $arguments
    ) {
    $this->sqliteConnection = $sqlite_connection;
    $this->serializedAst = $serialized_ast;
    $this->callback = $callback;
    $this->arguments = $arguments;
  }

  /**
   * Returns a query that computes the union of the results
   * of $this query and the $other query.
   */
  public function union(SQLiteQuery $other) : SQLiteCombinedQuery {
    return new SQLiteCombinedQuery('union', $this, $other);
  }

  /**
   * Returns a query that computes the concatenation of the results
   * of $this query and the $other query.
   */
  public function concat(SQLiteQuery $other) : SQLiteCombinedQuery {
    return new SQLiteCombinedQuery('union all', $this, $other);
  }

  /**
   * Returns a query that computes the intersection of the results
   * of $this query and the $other query.
   */
  public function intersect(SQLiteQuery $other) : SQLiteCombinedQuery {
    return new SQLiteCombinedQuery('intersect', $this, $other);
  }

  /**
   * Returns a query that returns only the rows from the result
   * of $this query if they are not present in the result of the $other query.
   */
  public function except(SQLiteQuery $other) : SQLiteCombinedQuery {
    return new SQLiteCombinedQuery('except', $this, $other);
  }

  /**
   * Returns a query that will return the results of $this query
   * but after excluding the first $count rows.
   */
  public function skip(int $count) : SQLiteQuery {
    if ($this->offset > 0) {
      $count += $this->offset;
    }
    return $this->skipAndTake($count, $this->limit);
  }

  /**
   * Returns a query that will return no more than $count of the rows
   * of the result of $this query.
   */
  public function take(int $count) : SQLiteQuery {
    if ($this->limit >= 0 && $this->limit < $count) {
      $count = $this->limit;
    }
    return $this->skipAndTake($this->offset, $count);
  }

  /**
   * Clones and modifies $this query with a limit clause that reflects
   * skipping $offset rows and returning no more than $limit rows.
   */
  private function skipAndTake(int $offset, int $limit) : SQLiteQuery {
    $result = clone $this;
    $result->offset = $offset;
    $result->limit = $limit;
    return $result;
  }

  /**
   * Obtains an iterator that produces the elements resulting from
   * this query. Use this for pull style queries.
   * This is the point when the query actually gets executed.
   */
  public function getIterator() : SQLiteQueryIterator {
    // Gets an object representing the SQL query.
    // The object is cached in a static cache and should
    // not be expected to hold on to state beyond this
    // method call.
    $sql_statement = $this->getStatement();

    // Initialize the cached statement for this particular query
    $sql_statement->clear();
    $sql_statement->reset();
    $num_args = count($this->arguments);
    $arg_count = 1;
    $arg_count =
      $this->bindValues($sql_statement, $this->arguments, $arg_count);
    $sql_statement->bindvalue($arg_count++, $this->limit, SQLITE3_INTEGER);
    $sql_statement->bindvalue($arg_count++, $this->offset, SQLITE3_INTEGER);

    // Now execute the query and encapsulate the query result in
    // the iterator object that represents the result of this query.
    $result = $sql_statement->execute();
    return new SQLiteQueryIterator($result, $this->callback);
  }

  //todo: do we need an async version of getIterator?

  //todo: we also need a way to obtain an event source that
  //pushes query result elements to a listener.

  /**
   * Binds the values in $arguments to the corresponding ? expressions
   * in $sql_statement. Skips over values in $arguments that correspond
   * to tables since those have been precompiled into the SQL string.
   * $arg_count specifies an offset for the arguments, so that more than
   * one array can be used to supply argument values.
   * Returns the original value of $arg_count plus the number of arguments
   * that were bound by this call.
   */
  protected function bindValues(
    SQLite3Stmt $sql_statement, array $arguments, int $arg_count) : int {
    $num_args = count($arguments);
    for ($i = 1; $i < $num_args; $i++) {
      $arg = $arguments[$i];
      if ($arg instanceof SQLiteTable) { continue; }
      $type = $this->getType($arg);
      if (!$sql_statement->bindvalue($arg_count++, $arg, $type)) {
        throw new LINQToSQLiteException("bad query arg");
      }
    }
    return $arg_count;
  }

  /**
   * Obtains a SQLite3 type tag for $arg based on its runtime PHP type.
   */
  protected function getType(mixed $arg) : int {
    switch (gettype($arg)) {
      case "boolean":
      case "integer": return SQLITE3_INTEGER;
      case "double": return SQLITE3_FLOAT;
      case "string": return SQLITE3_TEXT;
      case "array":
      case "object": return SQLITE3_BLOB;
      case "NULL": return SQLITE3_NULL;
      default: return -1;
    }
  }

  /**
   * Obtains a SQLite3Stmt instance corresponding to $this->serializedAst and
   * $this->arguments from a static cache, or creates and caches one if need be.
   *
   * The reason why the per query call argument values ($this->arguments) are
   * needed when we are obtaining a SQL statement to share between different
   * calls, is that we need to inject table names into the actual
   * SQL before calling prepare, so the statement we return needs
   * to know the table names, which are lurking in $arguments rather
   * than inside $serialized_ast.
   */
  protected function getStatement() : SQLite3Stmt {
    $ser = $this->serializedAst;
    $args = $this->arguments;

    $cache_root = self::$compiledQueries->get($ser);
    $cache_entry = $this->findOrCreateEntry($cache_root, $args);
    if ($cache_root === null) {
      self::$compiledQueries[$ser] = $cache_entry;
    }
    if ($cache_entry->statement === null) {
      $cache_entry->statement = $this->createStatement();
    }
    return $cache_entry->statement;
  }

  /**
   * Traverses the linked list of StatementCacheEntry instances until it finds
   * one with an arguments property that matches $this->arguments.
   * If no such entry can be found, a new entry is created and the end
   * of the list rooted at $root is made to point to it.
   * If $root is null, this just creates a new entry.
   */
  protected function findOrCreateEntry(
    ?StatementCacheEntry $root, array $args) : StatementCacheEntry {
    $curr = $root;
    while ($curr !== null) {
      if ($this->argumentsMatch($curr->arguments, $args)) {
        return $curr;
      }
      $root = $curr;
      $curr = $curr->next;
    }
    $curr = new StatementCacheEntry();
    $curr->arguments = $args;
    $curr->key = null;
    $curr->combinedEntry = null;
    $curr->next = null;
    if ($root !== null) {
      $root->next = $curr;
    }
    return $curr;
  }

  /**
   * Obtains the SQLite string that corresponds to the query,
   * either from the APC cache, or by creating and caching it,
   * and then compiles it into a SQLite3Stmt instance.
   */
  protected function createStatement() : SQLite3Stmt {
    $sql = $this->getSql();
    return $this->sqliteConnection->prepare($sql);
  }

  /**
   * Returns true if both arrays have the same number of elemements
   * and any element of type SQLiteTable in $args0 is matched by
   * a corresponding element of type SQLiteTable in $args1 with
   * the same table name.
   */
  protected function argumentsMatch(?array $args0, array $args1) {
    if ($args0 === null) return false;
    $c = count($args0);
    if ($c !== count($args1)) return false;
    for ($i = 0; $i < $c; $i++) {
      $arg0 = $args0[$i];
      $arg1 = $args1[$i];
      if ($arg0 instanceof SQLiteTable) {
        if (!($arg1 instanceof SQLiteTable)) {
          return false;
        }
        if ($arg0->getTableName() != $arg1->getTableName()) {
          return false;
        }
      }
    }
    return true;
  }

  /**
   * Combine $this->serializedAst and the table names from $this->arguments
   * into an md5 hash to use for a table lookup key.
   */
  protected function getHashKey() {
    $serialized_ast = $this->serializedAst;
    foreach ($this->arguments as $arg) {
      if ($arg instanceof SQLiteTable) {
        $serialized_ast .= $arg->getTableName();
      }
    }
    return md5($serialized_ast);
  }

  /**
   * Obtains a string containing a SQLite query expression that corresponds
   * to the PHP LINQ expression in $this->serializedAst (with @query_param_{i}
   * expressions that correspond to table names, already replaced with the
   * actual table names obtained from $this->arguments, or creates and
   * caches one if need be.
   *
   * The $hash string is derived from both $serialized_ast and the table names.
   * We are willing to pay the cost of constructing this hash because it
   * happens once per query expression (involving the same tables)
   * and because we need a short key for the APC.
   */
  protected function getSql() : string {
    $hash = $this->getHashKey();
    $cached_sql = apc_fetch($hash, &$success);
    if ($success) {
      return $cached_sql;
    }
    $sql_str = $this->makeSql();
    apc_store($hash, $sql_str);
    return $sql_str;
  }

  /**
   * Makes a string containing a SQLite query expression that corresponds
   * to the PHP LINQ expression in $this->serializedAst (with @query_param_{i}
   * expressions that correspond to table names, already replaced with the
   * actual table names obtained from $this->arguments.
   */
  protected function makeSql() {
    $ast = unserialize($this->serializedAst);
    $removeIntoClauses = new RemoveIntoClauses();
    $ast = $ast->accept($removeIntoClauses);
    $query_to_sql = new QueryToSQL($this->arguments);
    $sql_str = $ast->accept($query_to_sql);
    $sql_str .= ' limit ? offset ?';
    return $sql_str;
  }

}

/**
 * A node in a linked list obtained from a hash table.
 * Used to search for SQLite3Stmt instances based on
 * the serialized AST and either an array of arguments
 * or a string key together with a second with a second
 * serialized AST.
 */
class StatementCacheEntry {
  public ?array $arguments;
  public ?SQLite3Stmt $statement;
  public ?StatementCacheEntry $next;
  public ?StatementCacheEntry $combinedEntry;
  public ?string $key;
}

/**
 * A query that is the combination of two other queries.
 * For example the union of two queries.
 */
class SQLiteCombinedQuery extends SQLiteQuery {

  //The left hand operand of the combined expression.
  private SQLiteQuery $query1;
  //The right hand operand of the combined expression.
  private SQLiteQuery $query2;
  // union, union all, intersect, or except
  private string $operator;

  /**
   * Constructs a query that is the combination of two other queries.
   * For example the union of two queries.
   *
   * @operator
   * a string containing the SQL combining operator, for example "union".
   *
   * @query1
   * The left hand operand of the combined expression.
   *
   * @query2
   * The right hand operand of the combined expression.
   */
  public function __construct(
    string $operator, SQLiteQuery $query1, SQLiteQuery $query2) {
    $this->operator = $operator;
    $this->query1 = $query1;
    $this->query2 = $query2;
    $this->sqliteConnection = $query1->sqliteConnection;
    $this->callback = $query1->callback;
  }

  /**
   * Obtains an iterator that produces the elements resulting from
   * this query. Use this for pull style queries.
   * This is the point when the query actually gets executed.
   */
  public function getIterator() : SQLiteQueryIterator {
    // Gets an object representing the SQL query.
    // The object is cached in a static cache and should
    // not be expected to hold on to state beyond this
    // method call.
    $sql_statement = $this->getStatement();

    // Set up the cached statement for this particular query
    $sql_statement->clear();
    $sql_statement->reset();
    $arg_count = 1;
    $arg_count = $this->bindValues(
      $sql_statement, $this->query1->arguments, $arg_count);
    $sql_statement->bindvalue(
      $arg_count++, $this->query1->limit, SQLITE3_INTEGER);
    $sql_statement->bindvalue(
      $arg_count++, $this->query1->offset, SQLITE3_INTEGER);
    $arg_count = $this->bindValues(
      $sql_statement, $this->query2->arguments, $arg_count);
    $sql_statement->bindvalue(
      $arg_count++, $this->query2->limit, SQLITE3_INTEGER);
    $sql_statement->bindvalue(
      $arg_count++, $this->query2->offset, SQLITE3_INTEGER);
    $sql_statement->bindvalue($arg_count++, $this->limit, SQLITE3_INTEGER);
    $sql_statement->bindvalue($arg_count++, $this->offset, SQLITE3_INTEGER);

    // Now execute the query and encapsulate the query result
    // in the iterator object that represents the result of this
    // query.
    $result = $sql_statement->execute();
    return new SQLiteQueryIterator($result, $this->callback);
  }

  /**
   * Obtains a SQLite3Stmt instance corresponding to the combination of
   * $this->query1 and $this->query2 using the $this->operator.
   */
  protected function getStatement() : SQLite3Stmt {
    $ser1 = $this->query1->serializedAst;
    $args1 = $this->query1->arguments;
    $ser2 = $this->query2->serializedAst;
    $args2 = $this->query2->arguments;

    $cache_root = parent::$compiledQueries->get($ser1);
    $cache_entry1 = $this->findOrCreateEntry($cache_root, $args1);
    if ($cache_root === null) {
      parent::$compiledQueries[$ser1] = $cache_entry1;
    }
    $cache_entry2 = $this->findOrCreateKeyedEntry(
      $cache_entry1->combinedEntry, $ser2, $args2);
    if ($cache_entry1->combinedEntry === null) {
      $cache_entry1->combinedEntry = $cache_entry2;
    }
    $cache_entry3 = $this->findOrCreateKeyedEntry(
      $cache_entry2->combinedEntry, $this->operator, null);
    if ($cache_entry2->combinedEntry === null) {
      $cache_entry2->combinedEntry = $cache_entry2;
    }
    if ($cache_entry3->statement === null) {
      $cache_entry3->statement = $this->createStatement();
    }
    return $cache_entry3->statement;
  }

  /**
   * Traverses the linked list of StatementCacheEntry instances until it finds
   * one with the same $key and an arguments property that is null or
   * matches $this->arguments.
   * If no such entry can be found, a new entry is created and the end
   * of the list rooted at $root is made to point to it.
   * If $root is null, this just creates a new entry.
   */
  private function findOrCreateKeyedEntry(
    ?StatementCacheEntry $root, string $key, ?array $args
  ) : StatementCacheEntry {
    $curr = $root;
    while ($curr !== null) {
      if ($key === $curr->key) {
        if ($args === null || $this->argumentsMatch($curr->arguments, $args)) {
          return $curr;
        }
      }
      $root = $curr;
      $curr = $curr->next;
    }
    $curr = new StatementCacheEntry();
    $curr->arguments = $args;
    $curr->key = $key;
    $curr->combinedEntry = null;
    $curr->next = null;
    if ($root !== null) {
      $root->next = $curr;
    }
    return $curr;
  }

  /*
   * Combines hashes of the two subqueries with $this-operator to
   * form a hash key for caching the SQLite3Stmt that corresponds
   * to this (and other) isntances of SQLiteCombinedQuery.
   */
  protected function getHashKey() : string{
    $key1 = $this->query1->getHashKey();
    $key2 = $this->query2->getHashKey();
    return $key1.$this->operator.$key2;
  }

  /**
   * Makes a string containing the SQLite strings for the two
   * sub queries, combined with $this->operator.
   */
  protected function makeSql() : string {
    $sql1 = $this->query1->getSql();
    $sql2 = $this->query2->getSql();
    $sql_str = "select * from ($sql1) ".$this->operator.
      " select * from ($sql2) ";
    $sql_str .= ' limit ? offset ?';
    return $sql_str;
  }
}

/**
 * Provides the logic for a foreach statement to pull
 * PHP LINQ query expression result elements from a
 * SQLite3Result object.
 */
class SQLiteQueryIterator implements Iterator {
  // A way to get query result tuples from the connected database.
  private SQLite3Result $sqlResult;

  // A closure that is called for each tuple in the SQL query result
  // in order to obtain a PHP LINQ query expression result element.
  private (function(mixed) : mixed) $callback;

  // The number of elements already pulled from the iterator.
  // Used to provide the value of $this->key().
  private int $count;

  // The current element to be pulled via $this->current().
  // Updated by $this->next();
  private mixed $curr;

  // True if $this->curr contains a valid value.
  private bool $valid;

  /**
   * Constructs an iterator instance that allows foreach
   * statements to pull query results from SQL query result.
   *
   * @param @sqlResult
   * A way to get query result tuples from the connected database.
   *
   * @param @callback
   * A closure that is called for each tuple in the SQL query result
   * in order to obtain a PHP LINQ query expression result element.
   */
  public function __construct(
    SQLite3Result $sqlResult,
    (function(mixed) : mixed) $callback
  ) {
    $this->sqlResult = $sqlResult;
    $this->callback = $callback;

    $this->count = -1; // $this->next() will advance it to zero
    $this->valid = $this->next(); // get the first element if there is one
  }

  /**
   * Returns the current element of the iteration.
   * Only call this if $this->valid() return true.
   * To advance the iteration to the next element, call $this->next().
   */
  public function current() {
    return $this->curr;
  }

  /**
   * Returns the zero based sequence number of the current element.
   */
  public function key() {
    return $this->count-1;
  }

  /**
   * Advances the iterator by one element and returns
   * true if there is one, false if the iterator has run out elements.
   */
  public function next() {
    $this->count++;
    $row = $this->sqlResult->fetcharray(SQLITE3_NUM);
    if ($row === false) {
      $this->valid = false;
      return false;
    }
    $callback = $this->callback;
    $this->curr = $callback($row);
    return true;
  }

  /**
   * Resets the iterator back to the first element.
   */
  public function rewind() {
    if ($this->count === 0) { return; }
    $this->sqlResult->reset();
    $this->count = -1; // $this->next() will advance it to zero
    $this->valid = $this->next(); // get the first element if there is one
  }

  /**
   * Returns true if there is a current element.
   * Do not call $this->current(), $this->key() or $this->next()
   * if this returns false.
   */
  public function valid() {
    return $this->valid;
  }
}

/**
 * Transformer that rewrites:
 *   from .1. select .2. into id query_body
 * into
 *   from id in (from .1. select .2.) query_body
 *
 * and
 *   from .1. group id1 by .2. into id2 query_body
 * into
 *   from .1. group id1 by .2. let id2 = id1 query_body
 */
class RemoveIntoClauses {
  // Used to move or flatten query expression clauses during the rewrite
  private Vector $clauses = Vector {};

  /**
   * Returns a rewritten LINQ query expression that is easier
   * to translate into SQL.
   */
  public function visitQueryExpression($node) {
    foreach ($node->getClauses() as $clauses_elem) {
      $clauses_elem->accept($this);
    }
    $node->setClauses($this->clauses);
    return $node;
  }

  /**
   * Collect uninteresting clauses into $this->clauses.
   */
  public function visitNode($node) {
    $this->clauses[] = $node;
  }

  /**
   * An into clause following a select clause indicates that
   * all of the preceding clauses form the from collection for
   * an implicit from clause that precedes the remaining LINQ clauses.
   *
   * On the other hand, an into clause following a group by clause
   * simply introduces a name by which subequent clauses can refer
   * to the group, rather than introducing a nested query.
   */
  public function visitIntoClause($node) {
    $id = $node->getIdentifier();
    $previous_clause = $this->clauses[count($this->clauses)-1];
    if ($previous_clause instanceof CM\GroupClause) {
      $coll = $previous_clause->getCollection();
      if (!($coll instanceof CM\SimpleVariableExpression)) {
        throw new LINQToSQLiteException("groupby expression must be a table");
      }
      $let_clause =
        (new CM\LetClause())->setIdentifier($id)->setExpression($coll);
      $this->clauses[] = $let_clause;
      foreach ($node->getClauses() as $clauses_elem) {
        $clauses_elem->accept($this);
      }
      return;
    }

    $nested_query = (new CM\QueryExpression())->setClauses($this->clauses);
    $from =
      (new CM\FromClause())->setIdentifier($id)->setCollection($nested_query);
    $this->clauses = Vector{};
    $this->clauses[] = $from;
    foreach ($node->getClauses() as $clauses_elem) {
      $clauses_elem->accept($this);
    }
    $node->setClauses($this->clauses);
  }

}

/**
 * Transforms a PHP LINQ epxression into an equivalent SQLite expression.
 */
class QueryToSQL {
  // The SQL equivalent for the LINQ select clause.
  private string $select = '';

  // The SQL equivalent of the LINQ from and join clauses.
  private string $from = '';

  // The SQL equivalent of the conjunction of the LINQ where clause expressions
  // that do not follow on a group by clause.
  private string $whereExpr = '';

  // The SQL equivalent of the LINQ group by clause expressions.
  private string $groupbyExprs = '';

  // The SQL equivalent of the conjunction of the LINQ where clause expressions
  // that follow on a group by clause.
  private string $havingExpr = '';

  // The SQL equivalent for the the LINQ orderby clause.
  private string $orderby = '';

  // SQL does not have let expressions, but the subset targeted by this
  // translation is a pure functional language, so we can just replace
  // references to the let identifiers with the translated let expressions.
  // This map tracks the translations that correspond to let identifiers.
  private $letExpressionFor = Map {};

  // The argument values that correspond to the LINQ query parameters
  // for the query call that caused this translation. We only use it
  // to obtain the names of SQL tables referenced in the LINQ expression
  // via sub expressions whose runtime values are instances of SQLiteTable.
  private array $args;

  /**
   * Constructs a transformer from PHP LINQ expressions into a string
   * containing an equivalent SQLite expression.
   */
  public function __construct(array $args) {
    $this->args = $args;
  }

  /**
   * Combines a vector of strings into a single string with
   * parameterizable left delimiter, separator and right delimiter.
   * If the vector is empty the value that is returned is specified
   * by parameter rather than just being $left_delimiter.$right_delimiter.
   */
  private function visitVector(
    Vector<string> $elements,
    string $left_delimiter,
    string $separator,
    string $right_delimiter,
    string $empty_value
  ) : string {
    if ($elements->count() == 0) { return $empty_value; }
    $result = $left_delimiter;
    foreach ($elements as $elem) {
      if ($result !== $left_delimiter) { $result .= $separator; }
      $result .= $elem;
    }
    return $result.$right_delimiter;
  }

  /**
   * Translates the clauses into strings containing their SQL equivalents
   * (which may be the empty string). Then concatenates the strings
   * together again, but putting the select clause translation first
   * and the orderby clause last.
   *
   * Since this may be translating a nested query expression, it
   * saves and restores the translation state variables kept in
   * $this->select, $this->whereExpr, $this->groupbyExprs, $this->havingExpr
   * and $this->orderby.
   */
  public function visitQueryExpression($node) {
    $saved_select = $this->select;
    $saved_whereExpr = $this->whereExpr;
    $saved_groupbyExprs = $this->groupbyExprs;
    $saved_havingExpr = $this->havingExpr;
    $saved_orderby = $this->orderby;

    foreach ($node->getClauses() as $clauses_elem) {
      $clauses_elem->accept($this);
    }

    $result = $this->select;
    if ($this->from !== '') {
      $result .= ' '.$this->from;
    }
    if ($this->whereExpr !== '') {
      $result .= ' where '.$this->whereExpr;
    }
    if ($this->groupbyExprs !== '') {
      $result .= ' group by'.$this->groupbyExprs;
    }
    if ($this->havingExpr !== '') {
      $result .= ' having '.$this->havingExpr;
    }
    if ($this->orderby !== '') {
      $result .= ' '.$this->orderby;
    }

    $this->select = $saved_select;
    $this->whereExpr = $saved_whereExpr;
    $this->groupbyExprs = $saved_groupbyExprs;
    $this->havingExpr = $saved_havingExpr;
    $this->orderby = $saved_orderby;

    return $result;
  }

  /**
   * Translates a select clause, mainly by substituting let expressions
   * for let identifiers.
   */
  public function visitSelectClause($node) {
    $select = 'select ';
    $columns = array();
    $args = $node->getExpression()->getArguments();
    // If the select expression consists of single name that is neither
    // a column name nor a let identifier, it is presumed to be a table
    // name alias defined by a from clause, in which case we select *.
    if (count($args) == 1 && $args[0] instanceof CM\SimpleVariableExpression &&
    $this->letExpressionFor->get($args[0]->getVariableName()) === null) {
      $this->select = 'select *';
      $this->letExpressionFor->clear();
      return;
    }
    foreach ($args as $arguments_elem) {
      if ($select !== 'select ') { $select .= ', '; }
      $col = (string)$arguments_elem->accept($this);
      $select .= $col;
    }
    $this->select = $select;
    $this->letExpressionFor->clear();
  }

  /**
   * from table as id
   */
  public function visitFromClause($node) {
    $coll = $node->getCollection();
    $collection = (string)$coll->accept($this);
    if ($coll instanceof CM\QueryExpression) {
      $collection = "($collection)";
    }
    $result = 'from '.$collection;
    $alias = $node->getIdentifier();
    if ($alias !== null) {
      $result .= ' as '.$alias;
    }
    $this->from = $result;
  }

  /**
   * group by expr {, expr}
   */
  public function visitGroupClause($node) {
    $collection = (string)$node->getCollection()->accept($this);
    $key = (string)$node->getKey()->accept($this);
    if ($this->groupbyExprs !== '') { $this->groupbyExprs .= ','; }
    $this->groupbyExprs .= " $collection.$key";
  }

  /**
   * natural join table as id
   * join table as id on t1.c1 = t2.c2
   */
  public function visitJoinClause($node) {
    if ($this->groupbyExprs !== '') {
      throw new LINQToSQLiteException("join after group by is not supported");
    }
    $alias = $node->getIdentifier();
    $collection = (string)$node->getCollection()->accept($this);
    $left = $node->getLeft();
    $right = $node->getRight();
    $group = $node->getGroup();

    $result = 'join '.$collection;
    if ($alias !== null) {
      $result .= " as $alias";
    }
    if ($left !== null && $right !== null) {
      $result .= ' on ';
      $result .= (string)$left->accept($this);
      $result .= ' = ';
      $result .= (string)$right->accept($this);
    } else {
      $result = 'natural '.$result;
    }
    if ($group !== null) {
      throw new LINQToSQLiteException("join into is not supported");
    }
    $this->from .= ' '.$result;
  }

  /**
   * Updates $this->letExpressionFor.
   */
  public function visitLetClause($node) {
    $expr = $node->getExpression();
    $expr_str = (string)$expr->accept($this);
    if (!($expr instanceof CM\SimpleVariableExpression ||
          $expr instanceof CM\SimpleFunctionCallExpression)) {
      $expr_str = "($expr_str)";
    }
    $this->letExpressionFor[$node->getIdentifier()] = $expr_str;
  }

  /**
   * order by ordering {, ordering}
   */
  public function visitOrderbyClause($node) {
    $orders = Vector {};
    foreach ($node->getOrders() as $orders_elem) {
      $orders[] = (string)$orders_elem->accept($this);
    }

    $this->orderby = 'order by '.$this->visitVector($orders, '', ', ', '', '');
  }

  /**
   * asc or desc
   */
  public function visitOrdering($node) {
    $r = (string)$node->getExpression()->accept($this);
    switch ($node->getOrder()) {
      case CM\Orders::PHP_ASCENDING: $r .= ' asc'; break;
      case CM\Orders::PHP_DESCENDING: $r .= ' desc'; break;
    }
    return $r;
  }

  /**
   * having|where condition
   */
  public function visitWhereClause($node) {
    $condition = $node->getCondition()->accept($this);
    if ($this->groupbyExprs !== '') {
      if ($this->havingExpr !== '') {
        $this->havingExpr = '('.$this->havingExpr.') and ';
      }
      $this->havingExpr .= $condition;
    } else {
      if ($this->whereExpr !== '') {
        $this->whereExpr = '('.$this->whereExpr.') and ';
      }
      $this->whereExpr .= $condition;
    }
  }

  /**
   * (expression1) operation (expression2)
   * special handling for xor.
   */
  public function visitBinaryOpExpression($node) {
    $expression1 = (string)$node->getExpression1()->accept($this);
    $expression2 = (string)$node->getExpression2()->accept($this);
    $op = $node->getOperation();
    $r = "($expression1)";
    switch ($op) {
      case CM\BinaryOperators::PHP_AND: $r .= ' & '; break;
      case CM\BinaryOperators::PHP_CONCAT: $r .= '||'; break;
      case CM\BinaryOperators::PHP_DIVIDE: $r .= ' / '; break;
      case CM\BinaryOperators::PHP_IS_EQUAL: $r .= ' = '; break;
      case CM\BinaryOperators::PHP_IS_GREATER: $r .= ' > '; break;
      case CM\BinaryOperators::PHP_IS_GREATER_OR_EQUAL: $r .= ' >= '; break;
      case CM\BinaryOperators::PHP_IS_IDENTICAL: $r .= ' is '; break;
      case CM\BinaryOperators::PHP_IS_NOT_IDENTICAL: $r .= ' is not '; break;
      case CM\BinaryOperators::PHP_IS_NOT_EQUAL: $r .= ' != '; break;
      case CM\BinaryOperators::PHP_IS_SMALLER: $r .= ' < '; break;
      case CM\BinaryOperators::PHP_IS_SMALLER_OR_EQUAL: $r .= ' <= '; break;
      case CM\BinaryOperators::PHP_LOGICAL_AND: $r .= ' and '; break;
      case CM\BinaryOperators::PHP_LOGICAL_OR: $r .= ' or '; break;
      case CM\BinaryOperators::PHP_LOGICAL_XOR: $r .= ' or '; break;
      case CM\BinaryOperators::PHP_MINUS: $r .= ' - '; break;
      case CM\BinaryOperators::PHP_MODULUS: $r .= ' % '; break;
      case CM\BinaryOperators::PHP_MULTIPLY: $r .= ' * '; break;
      case CM\BinaryOperators::PHP_OR: $r .= ' | '; break;
      case CM\BinaryOperators::PHP_PLUS: $r .= ' + '; break;
      case CM\BinaryOperators::PHP_SHIFT_LEFT: $r .= ' << '; break;
      case CM\BinaryOperators::PHP_SHIFT_RIGHT: $r .= ' >> '; break;
      case CM\BinaryOperators::PHP_XOR: $r .= ' | '; break;
    }
    if ($op === CM\BinaryOperators::PHP_ARRAY_ELEMENT) {
      $expression2 .= ']';
    } else if ($op === CM\BinaryOperators::PHP_LOGICAL_XOR) {
      return '('.$r.
        "($expression2)) and not(($expression1) and ($expression2))";
    } else if ($op === CM\BinaryOperators::PHP_XOR) {
      return '('.$r.
        "($expression2)) & ~(($expression1) & ($expression2))";
    } else {
      $expression2 = "($expression2)";
    }
    return $r.$expression2;
  }

  /**
   *   case when condition then valueIfTrue else valueIfFalse
   */
  public function visitConditionalExpression($node) {
    $condition = (string)$node->getCondition()->accept($this);
    $value_if_true = (string)$node->getValueIfTrue()->accept($this);
    $value_if_false = (string)$node->getValueIfFalse()->accept($this);
    return "case when $condition then $value_if_true else $value_if_false end";
  }

  /**
   * The q_ prefix is used to distinguish function calls meant for
   * SQL from functions calls that should execute on the client side
   * and come to the query provider as query parameter values.
   *
   * Some function calls are translated to specialized SQL syntax.
   * Often that syntax allows for negation. In such cases the
   * LINQ expression will use the not_ prefix communicate the
   * negation in the form of a function name.
   */
  public function visitSimpleFunctionCallExpression($node) {
    $func = $node->getFunctionName();
    $func = substr($func, 2); //strip off universal q_ prefix
    $not = stripos($func, 'not_') === 0;
    if ($not) {
      $func = substr($func, 4); //strip off not_
    }

    $arguments = Vector{};
    foreach ($node->getArguments() as $arguments_elem) {
      $arguments[] = (string)$arguments_elem->accept($this);
    }
    $c = count($arguments);

    $concat = function($x,$y) { return $x.', '.$y; };
    if (strcasecmp($func, 'distinct') === 0) {
      return 'distinct '.$this->visitVector($arguments, '', ', ', '', '');
    } else if (strcasecmp($func, 'collate') === 0) {
      if ($c != 2) {
        throw new LINQToSQLiteException('q_collate needs two arguments');
      }
      $cfn = substr($arguments[1], 1, -1);
      $cfn = $this->validateCollatingFunctionName($cfn);
      return $arguments[0]." collate $cfn";
    } else if (
      strcasecmp($func, 'like') === 0 ||
      strcasecmp($func, 'glob') === 0 ||
      strcasecmp($func, 'regexp') === 0 ||
      strcasecmp($func, 'match') === 0) {
      if ($c < 2 || $c > 3) {
        throw new LINQToSQLiteException('argument count mismatch');
      }
      $result = $arguments[0].' '.($not ? 'not ' : '').$func;
      $result .= ' '.$arguments[1];
      if ($c === 3) {
        $result .= ' escape '.$arguments[2];
      }
      return $result;
    } else if (
      strcasecmp($func, 'isnull') === 0 ||
      strcasecmp($func, 'null') === 0 && $not) {
      return $arguments[0].' '.($not ? 'not ' : '').$func;
    } else if (strcasecmp($func, 'between') === 0) {
      if ($c != 3) {
        throw new LINQToSQLiteException('q_between needs two arguments');
      }
      return $arguments[0].' '.($not ? 'not ' : '').'between '.
        $arguments[1].' and '.$arguments[2];
    } else if (strcasecmp($func, 'in') === 0) {
      if ($c < 1) {
        throw new LINQToSQLiteException('at least one argument expected');
      }
      $arg0 = array_shift($arguments);
      return $arg0.' '.($not ? 'not ' : '').'in '.
        $this->visitVector($arguments, '(', ', ', ')', '()');
    } else if (strcasecmp($func, 'exists') === 0) {
      return ($not ? 'not ' : '').'exists'.
        $this->visitVector($arguments, '(', ', ', ')', '()');
    } else if (strcasecmp($func, 'raise') === 0) {
      if ($c < 1) {
        throw new LINQToSQLiteException('at least one argument expected');
      }
      $action = substr($arguments[1], 1, -1);
      $action = $this->validateRaiseAction($action);
      if ($action === 'ignore') {
        if ($c !== 1) {
          throw new LINQToSQLiteException('only one argument expected');
        }
      } else {
        if ($c !== 2) {
          throw new LINQToSQLiteException('two arguments expected');
        }
        $action .= ', '.$arguments[2];
      }
      return "$func($action)";
    } else {
      $func = $this->validateFunctionName($func);
      return "$func".$this->visitVector($arguments, '(', ', ', ')', '()');
    }
  }

  private function validateCollatingFunctionName(string $cfn) : string {
    if (strcasecmp($cfn, "binary") === 0) return "binary";
    if (strcasecmp($cfn, "nocase") === 0) return "nocase";
    if (strcasecmp($cfn, "rtrim") === 0) return "rtrim";
    throw new LINQToSQLiteException("$cfn is not a valid collating sequence");
  }

  private function validateRaiseAction(string $action) : string {
    if (strcasecmp($cfn, "ignore") === 0) return "ignore";
    if (strcasecmp($cfn, "rollback") === 0) return "rollback";
    if (strcasecmp($cfn, "abort") === 0) return "abort";
    if (strcasecmp($cfn, "fail") === 0) return "fail";
    throw new LINQToSQLiteException("$action is not a valid raise action");
  }

  private function validateFunctionName(string $func_name) : string {
    if (preg_match('/[^[:digit:][:alpha:]_]+/', $func_name)) {
      throw new LINQToSQLiteException("$func_name is not a name");
    }
    return $func_name;
  }

  /**
   * object . propertyName
   */
  public function visitObjectPropertyExpression($node) {
    $object = $node->getObject()->accept($this);
    $prop_name = $node->getPropertyName();
    return $object.'.'.$prop_name;
  }

  /**
   * ' becomes '' and the resulting string value is placed inside single quotes.
   */
  private function escapeString($str) {
    $str = str_replace("'", "''", $str);
    return "'$str'";
  }

  /**
   * Escapes strings, formats numbers.
   */
  public function visitScalarExpression($node)  {
    $val = $node->getValue();
    if (is_string($val)) { return $this->escapeString($val); }
    $result = (string)$val;
    if (is_float($val) && !strpos($result, '.')) {
      $result .= '.0';
    }
    return $result;
  }

  /**
   * Translates query parameters into '?' or table names.
   * Substitutes expressions for variables defined by let
   * expressions. Leaves aliases as is.
   */
  public function visitSimpleVariableExpression($node) {
    $name = $node->getVariableName();
    if (strpos($name, "@query_param_") === 0) {
      $i = (int)substr($name, 13);
      $arg = $this->args[$i];
      if ($arg instanceof SQLiteTable) {
        $arg->validate();
        return $arg->getTableName();
      }
      return "?";
    }
    $expr = $this->letExpressionFor->get($name);
    if ($expr !== null) { return $expr; }
    return $name;
  }

  /**
   * operation(expression) or cast(expression as operation)
   */
  public function visitUnaryOpExpression($node) {
    $op = $node->getOperation();
    $expr = $node->getExpression();
    $expression = (string)$expr->accept($this);

    switch ($op) {
      case CM\UnaryOperators::PHP_BITWISE_NOT_OP:
        return "~($expression)";
      case CM\UnaryOperators::PHP_FLOAT_CAST_OP:
        return "cast($expression as real)";
      case CM\UnaryOperators::PHP_INT_CAST_OP:
        return "cast($expression as integer)";
      case CM\UnaryOperators::PHP_MINUS_OP:
        return "-($expression)";
      case CM\UnaryOperators::PHP_NOT_OP:
        return "not($expression)";
      case CM\UnaryOperators::PHP_PLUS_OP:
        return "+($expression)";
      case CM\UnaryOperators::PHP_STRING_CAST_OP:
        return "cast($expression as text)";
    }
    throw new LINQToSQLiteException('invalid '.$expression);
  }
}

/**
 * Thrown when the PHP LINQ expression cannot be translated
 * into a SQL expression.
 */
class LINQToSQLiteException extends Exception {
  /**
   * Constructs an exception with an error message
   * that is thrown to terminate an invalid query.
   */
  public function __construct(string $message) {
    parent::__construct($message);
  }
}

function test() {
  $db = new SQLite3(':memory:');
  $db->exec("CREATE TABLE People (name STRING, surname STRING, ".
            "pid INTEGER primary key)");
  $db->exec("INSERT INTO People VALUES ('Joe', 'Aloe', 1)");
  $db->exec("INSERT INTO People VALUES ('Joe', 'Blow', 2)");
  $db->exec("INSERT INTO People VALUES ('Jill', 'Callow', 3)");
  $db->exec("INSERT INTO People VALUES ('Jill', 'Doe', 4)");
  $db->exec("INSERT INTO People VALUES ('Jill', 'Elbow', 5)");
  $db->exec("INSERT INTO People VALUES ('Jane', 'Doe', 6)");

  $db->exec("CREATE TABLE Follows (pid INTEGER, ".
  " leader INTEGER references People(pid))");
  $db->exec("INSERT INTO Follows VALUES (1, 3)");
  $db->exec("INSERT INTO Follows VALUES (1, 6)");
  $db->exec("INSERT INTO Follows VALUES (2, 1)");
  $db->exec("INSERT INTO Follows VALUES (3, 6)");
  $db->exec("INSERT INTO Follows VALUES (4, 2)");
  $db->exec("INSERT INTO Follows VALUES (5, 6)");
  $db->exec("INSERT INTO Follows VALUES (6, 1)");

  $phinq = new LINQToSQLite($db);
  $people = $phinq->getTable("People");
  $people->validate();
  $follows = $phinq->getTable("Follows");
  $follows->validate();

  $bogus = $phinq->getTable("drop table injection");
  try {
    $bogus->validate();
  } catch (LINQToSQLiteException $e) {
    echo $e->getMessage()."\n";
  }

  $j = "J";
  $oe = "oe";
  $joes =
    from $p in $people
    where $p->name == $j.$oe
    orderby q_collate($p->surname, 'nocase') descending
    let $d = q_length($p->name)
    select array($p->name, $p->surname, $d);
  foreach ($joes as $joe) {
    var_dump($joe);
  }
  echo "\n";

  $follies =
    from $p in $people join $f in $follows on $p->pid equals $f->leader
    select "{$p->name} {$p->surname} is a leader.";
  foreach ($follies as $folly) {
    echo $folly;
    echo "\n";
  }
  echo "\n";

  $folliesDuo =
    from $p in $people join $follows
    select "{$p->name} {$p->surname} is a follower.";
  foreach ($folliesDuo as $folly) {
    echo $folly;
    echo "\n";
  }
  echo "\n";

  $into =
    from $p in $people
    where $p->name == 'Joe'
    select $p
    into $joe
      join $follows
      select "{$joe->name} {$joe->surname} is a follower.";
  foreach ($into as $folly) {
    echo $folly;
    echo "\n";
  }
  echo "\n";

  $group =
    from $p in $people
    group $p by $p->surname
    into $families
      orderby $p->surname
      where $families->surname >= "Callow"
      let $c = q_group_concat($families->name)
      where $families->surname < "Elbow"
      select array($families->surname, $c);
  foreach ($group as $g) {
    var_dump($g);
  }
  echo "\n";

  $expr =
    from $p in $people
    let $e = q_isnull($p->surname) ? q_upper($p->name) : $p->name
    select $e;
  foreach ($expr as $e) {
    var_dump($e);
  }
  echo "\n";

  $expr =
    from $p in $people
    select q_not_like($p->surname, '$%', '$') ? q_upper($p->name) : $p->name;
  foreach ($expr as $e) {
    var_dump($e);
  }
  echo "\n";

  $expr =
    from $p in $people
    select $e = q_between(q_length($p->surname), 3, 4) ?
      q_upper($p->name) : $p->name;
  foreach ($expr as $e) {
    var_dump($e);
  }
  echo "\n";

  for ($i = 0; $i < 2; $i++) {
    $expr =
      from $p in $people
      select q_not_in($p->surname, 'Callow', 'Doe') ?
        q_upper($p->name) : $p->name;
    foreach ($expr as $e) {
      var_dump($e);
    }
    echo "\n";
  }

  //test limit
  $expr = from $p in $people select $p->name.' '.$p->surname;
  $expr1 = $expr->take(3);
  foreach ($expr1 as $e) {
    echo $e."\n";
  }
  echo "\n";

  //test offset
  $expr2 = $expr1->skip(1);
  foreach ($expr2 as $e) {
    echo $e."\n";
  }
  echo "\n";

  //test union
  $expr3 = $expr1->union($expr2);
  foreach ($expr3 as $e) {
    echo $e."\n";
  }
  echo "\n";

  //test union all
  $expr3 = $expr1->concat($expr2);
  foreach ($expr3 as $e) {
    echo $e."\n";
  }
  echo "\n";

  //test intersection
  $expr3 = $expr1->intersect($expr2);
  foreach ($expr3 as $e) {
    echo $e."\n";
  }
  echo "\n";

  //test except
  $expr3 = $expr1->except($expr2);
  foreach ($expr3 as $e) {
    echo $e."\n";
  }
  echo "\n";

}

test();
