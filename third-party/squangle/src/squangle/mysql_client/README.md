squangle/mysql_client
--------------------

facebook::common::mysql_client is a collection of classes for
performing fully asynchronous queries against MySQL databases.  It
supports connecting and querying in asynchronous modes (either
callbacks or explicit waiting).

This document is a basic overview/roadmap; please consult the headers
for more details, as well as the test cases and ParallelMysqlTool.cpp
for example usage.

#### Synchronous Examples
### Basic Example

    #include "squangle/mysql_client/AsyncMysqlClient.h"

    AsyncMysqlClient* client = AsyncMysqlClient::defaultClient();

    std::shared_ptr<ConnectOperation> connect_op =
        client->beginConnection(hostname, port, dbname, user, password);
    connect_op->run();
    connect_op->wait();
    if (!connect_op.ok()) {
      panic();
    }

### Connect and Query Example

    Query query("SELECT %C FROM %T WHERE %C = %s AND %C BETWEEN %d AND %d",
      "pusheen_macro",
      "macros",
      "type", "pusheen",
      "awesomeness", MAX, INF);

    DbLocator dblocator(XDB_MACROS, db::InstanceRequirement::Any);

    auto conn_op = FbAsyncMysqlClient::defaultClient()->beginConnection(dblocator);
    conn_op->run()->wait();
    if (!conn_op->ok()) {
      panicalittlebit();
    }

    auto query_op = Connection::beginQuery(conn_op->releaseConnection(), std::move(query));
    query_op->run()->wait();
    if (!query_op->ok()) {
      dontpanicthatmuch();
    }

    for (const auto& row : query_op->queryResult()) {
      // Play with rows :)
    }

#### Asynchronous Examples
### Single Query Example

    std::shared_ptr<QueryOperation> query_op =
        Connection::beginQuery(std::move(conn), query_string);
    query_op->setCallback(query_callback);
    query_op->setTimeout(500ms);
    query_op->run();

    void query_callback(QueryOperation& op,
                        QueryResult* query_result,
                        QueryCallbackReason reason) {
      if (reason == QueryCallbackReason::RowsFetched) {
        LOG(INFO) << "Saw " << query_result->numRows() << " more rows!";
        for (const auto& row : *query_result) {
          LOG(INFO) << "Row: " << folly::join(row, "\t");
        }
      } else if (reason == QueryCallbackReason::Success) {
        LOG(INFO) << "Query succeeded!";
      } else {
        LOG(ERROR) << "Query failed! " << op.mysql_error();
      }
    }

### Multi Query Example

    vector<Query> queries = vector<Query>{Query(query_string), select_query};
    std::shared_ptr<MultiQueryOperation> query_op =
        Connection::beginMultiQuery(std::move(conn), std::move(queries));
    query_op->setCallback(query_callback);
    query_op->setTimeout(500ms);
    query_op->run();

    void query_callback(MultiQueryOperation& op,
                        QueryResult* query_result,
                        QueryCallbackReason reason) {
      if (reason == QueryCallbackReason::RowsFetched) {
        LOG(INFO) << "Saw " << query_result->numRows() << " more rows!";
        for (const auto& row : *query_result) {
          LOG(INFO) << "Row: " << folly::join(row, "\t");
        }
      } else if (reason == QueryCallbackReason::QueryBoundary) {
        LOG(INFO) << "Finished statement " << query_result->queryNum();
      } else if (reason == QueryCallbackReason::Success) {
        LOG(INFO) << "Query succeeded!";
      } else {
        LOG(ERROR) << "Query failed! " << op.mysql_error();
      }
    }

### Concepts

* `AsyncMysqlClient` -- the main class that you use to get connections
* `Connection` -- a connection to a specific database; pass this around
  when you run queries
* `Operation`, `ConnectOperation`, `QueryOperation` `MultiQueryOperation`
  -- a virtual base class and its three concrete children that represent
  things you want to do (or have done) asynchronously
* `OperationBatch` -- allows waiting for multiple operations to finish in
  parallel

Asynchronous programming can be tricky.  The Async MySQL client tried
to manage this complexity via the concept of Operations.  Operation
objects represent some kind of async request, be it a connect or
query, and hold the result when it completes.  In the above example,
we create a connection operation (`connect_op`), run it (which is a
non-blocking call), and wait for it to complete (which is a blocking
call). Specifically, *no function on any method of any class blocks,
except for wait() and mustSucceed()*.

In addition to being able to start a MySQL query or connection "in the
background" like the connect above, you can specify a callback, as
seen in the query example.  The callback may be invoked multiple
times; for each callback it's passed the reason for it: RowsFetched,
QueryBoundary when you're running multiple queries, Success and Failure.
The data regarding the query is inside QueryResult (last insert id,
affected rows, etc).

### Ownership and Thread Safety

99.9% of the time, you don't need to own an actual client object;
simply use `AsyncMysqlClient::defaultClient()` to get a singleton.
This class is thread safe.  If you do need multiple clients, just
construct them yourself.

One difficulty of asynchronous operations is ownership of the
underlying requests and results.  For this async client,
`std::shared_ptr` and `std::unique_ptr` are used to manage the
relevant ownerships.  Connections are owned via a `unique_ptr` and are
handed over to the async client when a query is being run.  When the
query completes (either successfully or with any error except a
timeout), you can retrieve the connection to begin a new query via
`releaseConnection`.

Operations are shared ownership with the client itself.  Once you call
`run()` however, you should not invoke other methods besides `wait()`
as status checks, etc, will otherwise be racey.  If you're using a
callback, you can safely discard your reference to the operation;
cleanup will occur when the query completes (either with rows or
errors).

### Bonus feature: Query Formatting and Construction

The mysql_client also supports a new method of constructing queries,
similar to the www function `queryfx`.  See `Query.h` for details, but
in a nutshell, it is designed to prevent SQL injection while making
query construction itself easier.
