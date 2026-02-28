
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns the MySQL client statistics at the moment the successful query
ended




``` Hack
public function clientStats(): AsyncMysqlClientStats;
```




This information can be used to know how the performance of the
MySQL client may have affected the query operation.




## Returns




+ [` AsyncMysqlClientStats `](/apis/Classes/AsyncMysqlClientStats/) - an [` AsyncMysqlClientStats `](/apis/Classes/AsyncMysqlClientStats/) object to query about event and
  callback timing to the MySQL client for the query.




## Examples




You can get some statistical information from the MySQL client when you get an [` AsyncMysqlQueryResult `](/apis/Classes/AsyncMysqlQueryResult/) via the [` clientStats() `](/apis/Classes/AsyncMysqlQueryResult/clientStats/) method.




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
  \var_dump($result->clientStats()->callbackDelayMicrosAvg());
  $conn->close();
  return $result->numRows();
}

<<__EntryPoint>>
async function run(): Awaitable<void> {
  $r = await simple_query();
  \var_dump($r);
}
```.hhvm.expectf
float(%f)
int(%d)
```.example.hhvm.out
float(21.760009765625)
int(1)
```.skipif
await \Hack\UserDocumentation\API\Examples\AsyncMysql\skipif_async();
~~~
<!-- HHAPIDOC -->
