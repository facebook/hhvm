
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns the MySQL client statistics at the moment the connection was
established




``` Hack
public function clientStats(): AsyncMysqlClientStats;
```




This information can be used to know how the performance of the
MySQL client may have affected the connecting operation.




## Returns




+ [` AsyncMysqlClientStats `](/apis/Classes/AsyncMysqlClientStats/) - an [` AsyncMysqlClientStats `](/apis/Classes/AsyncMysqlClientStats/) object to query about event and
  callback timing to the MySQL client for the connection.




## Examples




Every connection has a connection result. You get the connection result from a call to [` AsyncMysqlConnection::connectResult `](/apis/Classes/AsyncMysqlConnection/connectResult/). And one of the methods on an [` AsyncMysqlConnectResult `](/apis/Classes/AsyncMysqlConnectResult/) is [` clientStats() `](/apis/Classes/AsyncMysqlConnectResult/clientStats/), which gives you some information about the client you are connecting too.




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
async function get_client_stats(): Awaitable<?\AsyncMysqlClientStats> {
  $pool = new \AsyncMysqlConnectionPool(darray[]);
  $conn = await connect($pool);
  $cstats = $conn->connectResult()?->clientStats();
  \var_dump($cstats?->callbackDelayMicrosAvg());
  $conn->close();
  return $cstats;
}

<<__EntryPoint>>
async function run(): Awaitable<void> {
  $cs = await get_client_stats();
  \var_dump($cs);
}
```.hhvm.expectf
float(%f)
object(AsyncMysqlClientStats) (0) {
}
```.example.hhvm.out
float(20.75)
object(AsyncMysqlClientStats) (0) {
}
```.skipif
await \Hack\UserDocumentation\API\Examples\AsyncMysql\skipif_async();
~~~
<!-- HHAPIDOC -->
