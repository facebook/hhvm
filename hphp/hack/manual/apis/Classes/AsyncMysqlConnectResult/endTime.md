
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

The end time of the connection operation, in seconds since epoch




``` Hack
public function endTime(): float;
```




## Returns




+ ` float ` - the end time as `` float `` seconds since epoch.




## Examples




Every connection has a connection result. You get the connection result from a call to [` AsyncMysqlConnection::connectResult `](/apis/Classes/AsyncMysqlConnection/connectResult/). And one of the methods on an [` AsyncMysqlConnectResult `](/apis/Classes/AsyncMysqlConnectResult/) is [` endTime() `](/apis/Classes/AsyncMysqlConnectResult/endTime/), which tells you when the connection operation completed.




Note that




```
  elapsedMicros() ~== endTime() - startTime()
```




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
async function get_connection_start_time(): Awaitable<?float> {
  $pool = new \AsyncMysqlConnectionPool(darray[]);
  $conn = await connect($pool);
  $et = $conn->connectResult()?->endTime();
  $conn->close();
  return $et;
}

<<__EntryPoint>>
async function run(): Awaitable<void> {
  $et = await get_connection_start_time();
  \var_dump($et);
}
```.hhvm.expectf
float(%f)
```.example.hhvm.out
float(17354.068556)
```.skipif
await \Hack\UserDocumentation\API\Examples\AsyncMysql\skipif_async();
~~~
<!-- HHAPIDOC -->
