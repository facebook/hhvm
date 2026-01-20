
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Begin running a query with multiple statements




``` Hack
public function multiQuery(
  Traversable<string, arraykey, mixed> $queries,
  int $timeout_micros = -1,
  dict<string> $query_attributes = dict [
],
): Awaitable<Vector<AsyncMysqlQueryResult>>;
```




[` AsyncMysqlConnection::multiQuery() `](/apis/Classes/AsyncMysqlConnection/multiQuery/) is similar to
[` AsyncMysqlConnection::query() `](/apis/Classes/AsyncMysqlConnection/query/), except that you can pass an array of
`` string `` queries to run one after the other. Then when you ``` await ``` or
```` join ```` on the returned [` Awaitable `](/apis/Classes/HH/Awaitable/), you will get a [` Vector `](/apis/Classes/HH/Vector/) of
[` AsyncMysqlQueryResult `](/apis/Classes/AsyncMysqlQueryResult/), one result for each query.




We strongly recommend using multiple calls to [` queryf() `](/apis/Classes/AsyncMysqlConnection/queryf/) instead as it
escapes parameters; multiple queries can be executed simultaneously by
combining [` queryf() `](/apis/Classes/AsyncMysqlConnection/queryf/) with `` HH\Asio\v() ``.




## Parameters




+ [` Traversable<string, `](/apis/Interfaces/HH/Traversable/)`` arraykey, mixed> $queries `` - A [` Vector `](/apis/Classes/HH/Vector/) of queries, with each query being a `` string ``
  in the array.
+ ` int $timeout_micros = -1 ` - The maximum time, in microseconds, in which the
  query must be completed; -1 for default, 0 for
  no timeout.
+ ` dict<string> $query_attributes = dict [ ] ` - Query attributes. Empty by default.




## Returns




* [` Awaitable<Vector<AsyncMysqlQueryResult>> `](/apis/Classes/HH/Awaitable/) - an [` Awaitable `](/apis/Classes/HH/Awaitable/) representing the result of your multi-query. Use
  `` await `` or ``` join ``` to get the actual [` Vector `](/apis/Classes/HH/Vector/) of
  [` AsyncMysqlQueryResult `](/apis/Classes/AsyncMysqlQueryResult/) objects.




## Examples




[` AsyncMysqlConnection::multiQuery `](/apis/Classes/AsyncMysqlConnection/multiQuery/) is similar to [` AsyncMysqlConnection::query `](/apis/Classes/AsyncMysqlConnection/query/), except that you can pass an array of queries to run one after the other. Then when you `` await `` on the call, you will get a [` Vector `](/apis/Classes/HH/Vector/) of [` AsyncMysqlQueryResult `](/apis/Classes/AsyncMysqlQueryResult/), one result for each query.




~~~ basic-usage.hack
use \Hack\UserDocumentation\API\Examples\AsyncMysql\ConnectionInfo as CI;

async function connect(
  \AsyncMysqlConnectionPool $pool,
): Awaitable<\AsyncMysqlConnection> {
  return await $pool->connect(
    CI::$host,
    CI::$port,
    CI::$db,
    CI::$user,
    CI::$passwd,
  );
}
async function simple_multi_query(): Awaitable<int> {
  // In our test database, the third query will return an empty result since
  // we do not have a user ID of 3.
  $queries = Vector {
    'SELECT name FROM test_table WHERE userID = 1',
    'SELECT age, email FROM test_table WHERE userID = 2',
    'SELECT name FROM test_table WHERE userID = 3',
  };
  $pool = new \AsyncMysqlConnectionPool(darray[]);
  $conn = await connect($pool);
  $results = await $conn->multiQuery($queries);
  $conn->close();
  $x = 0;
  foreach ($results as $result) {
    $x += $result->numRows();
  }
  return $x;
}

<<__EntryPoint>>
async function run(): Awaitable<void> {
  $r = await simple_multi_query();
  \var_dump($r);
}
```.hhvm.expectf
int(%d)
```.example.hhvm.out
int(1)
```.skipif
await \Hack\UserDocumentation\API\Examples\AsyncMysql\skipif_async();
~~~
<!-- HHAPIDOC -->
