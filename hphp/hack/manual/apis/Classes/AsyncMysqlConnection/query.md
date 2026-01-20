
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Begin running an unsafe query on the MySQL database client




``` Hack
public function query(
  string $query,
  int $timeout_micros = -1,
  dict<string> $query_attributes = dict [
],
): Awaitable<AsyncMysqlQueryResult>;
```




If you have a direct query that requires no placeholders, then you can
use this method. It takes a raw string query that will be executed as-is.




You may want to call [` escapeString() `](/apis/Classes/AsyncMysqlConnection/escapeString/) to ensure that any queries out of
your direct control are safe.




We strongly recommend using [` queryf() `](/apis/Classes/AsyncMysqlConnection/queryf/) instead in all cases, which
automatically escapes parameters.




## Parameters




+ ` string $query ` - The query itself.
+ ` int $timeout_micros = -1 ` - The maximum time, in microseconds, in which the
  query must be completed; -1 for default, 0 for
  no timeout.
+ ` dict<string> $query_attributes = dict [ ] ` - Query attributes. Empty by default.




## Returns




* [` Awaitable<AsyncMysqlQueryResult> `](/apis/Classes/HH/Awaitable/) - an [` Awaitable `](/apis/Classes/HH/Awaitable/) representing the result of your query. Use
  `` await `` or ``` join ``` to get the actual [` AsyncMysqlQueryResult `](/apis/Classes/AsyncMysqlQueryResult/)
  object.




## Examples




The following example shows a basic usage of [` AsyncMysqlConnection::query `](/apis/Classes/AsyncMysqlConnection/query/). First you get a connection from an [` AsyncMysqlConnectionPool `](/apis/Classes/AsyncMysqlConnectionPool/), then you can make the query.




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
async function simple_query(): Awaitable<int> {
  $pool = new \AsyncMysqlConnectionPool(darray[]);
  $conn = await connect($pool);
  $result = await $conn->query('SELECT name FROM test_table WHERE userID = 1');
  $conn->close();
  return $result->numRows();
}

<<__EntryPoint>>
async function run(): Awaitable<void> {
  $r = await simple_query();
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
