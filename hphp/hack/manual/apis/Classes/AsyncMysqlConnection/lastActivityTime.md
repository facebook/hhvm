
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Last time a successful activity was made in the current connection, in
seconds since epoch




``` Hack
public function lastActivityTime(): float;
```




The first successful activity of the current connection is its creation.




## Returns




+ ` float ` - A `` float `` representing the number of seconds ago since epoch
  that we had successful activity on the current connection.




## Examples




This example shows how to determine the last time a successful call was made using a given connection via [` AsyncMysqlConnection::lastActivityTime `](/apis/Classes/AsyncMysqlConnection/lastActivityTime/). The value returned is seconds since epoch as a `` float ``.




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
async function get_time(): Awaitable<float> {
  $pool = new \AsyncMysqlConnectionPool(darray[]);
  $conn = await connect($pool);
  $t = $conn->lastActivityTime();
  $conn->close();
  return $t;
}

<<__EntryPoint>>
async function run(): Awaitable<void> {
  $t = await get_time();
  \var_dump($t);
}
```.hhvm.expectf
float(%f)
```.example.hhvm.out
float(17272.084797)
```.skipif
await \Hack\UserDocumentation\API\Examples\AsyncMysql\skipif_async();
~~~
<!-- HHAPIDOC -->
