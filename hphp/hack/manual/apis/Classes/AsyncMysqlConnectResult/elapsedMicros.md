
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

The total time for the establishment of the MySQL connection,
in microseconds




``` Hack
public function elapsedMicros(): int;
```




## Returns




+ ` int ` - the total establishing connection time as `` int `` microseconds.




## Examples




Every connection has a connection result. You get the connection result from a call to [` AsyncMysqlConnection::connectResult `](/apis/Classes/AsyncMysqlConnection/connectResult/). And one of the methods on an [` AsyncMysqlConnectResult `](/apis/Classes/AsyncMysqlConnectResult/) is [` elapsedMicros() `](/apis/Classes/AsyncMysqlConnectResult/elapsedMicros/), which tells you how long it took to make the connection.




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
async function get_connection_time(): Awaitable<?int> {
  $pool = new \AsyncMysqlConnectionPool(darray[]);
  $conn = await connect($pool);
  $em = $conn->connectResult()?->elapsedMicros();
  $conn->close();
  return $em;
}

<<__EntryPoint>>
async function run(): Awaitable<void> {
  $em = await get_connection_time();
  \var_dump($em);
}
```.hhvm.expectf
int(%d)
```.example.hhvm.out
int(3334)
```.skipif
await \Hack\UserDocumentation\API\Examples\AsyncMysql\skipif_async();
~~~
<!-- HHAPIDOC -->
