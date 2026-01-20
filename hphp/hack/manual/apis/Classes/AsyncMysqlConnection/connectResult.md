
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns the [` AsyncMysqlConnectResult `](/apis/Classes/AsyncMysqlConnectResult/) for the current connection




``` Hack
public function connectResult(): ?AsyncMysqlConnectResult;
```




An [` AsyncMysqlConnectResult `](/apis/Classes/AsyncMysqlConnectResult/) provides information about the timing for
creating the current connection.




## Returns




+ ` ? `[` AsyncMysqlConnectResult `](/apis/Classes/AsyncMysqlConnectResult/) - An [` AsyncMysqlConnectResult `](/apis/Classes/AsyncMysqlConnectResult/) object or `` null `` if the
  [` AsyncMysqlConnection `](/apis/Classes/AsyncMysqlConnection/) was not created in the MySQL client.




## Examples




This example shows how to get data about the async MySQL connection you made via a call to [` AsyncMysqlConnection::connectResult `](/apis/Classes/AsyncMysqlConnection/connectResult/). An [` AsyncMysqlConnectResult `](/apis/Classes/AsyncMysqlConnectResult/) is returned and there are various statistical methods you can call. Here, we call `` elapsedTime `` to show the time it took to make the connection.




Interestingly, if you run this example twice or more, you may notice that the second time on will show a lower elapsed time than the first. This could be due to caching mechanisms, etc.




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
async function get_connect_time(): Awaitable<?int> {
  $pool = new \AsyncMysqlConnectionPool(darray[]);
  $conn = await connect($pool);
  $result = $conn->connectResult(); // returns ?\AsyncMysqlConnectResult
  $conn->close();
  return $result?->elapsedMicros();
}

<<__EntryPoint>>
async function run(): Awaitable<void> {
  $time = await get_connect_time();
  \var_dump($time);
}
```.hhvm.expectf
int(%d)
```.example.hhvm.out
int(2984)
```.skipif
await \Hack\UserDocumentation\API\Examples\AsyncMysql\skipif_async();
~~~
<!-- HHAPIDOC -->
