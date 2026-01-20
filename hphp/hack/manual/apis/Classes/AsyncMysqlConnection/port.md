
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

The port on which the MySQL instance is running




``` Hack
public function port(): int;
```




## Returns




+ ` int ` - The port as an `` int ``.




## Examples




The following example shows how to get the port of the MySQL server that this connection is associated with via [` AsyncMysqlConnection::port `](/apis/Classes/AsyncMysqlConnection/port/).




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
async function get_port(): Awaitable<int> {
  $pool = new \AsyncMysqlConnectionPool(darray[]);
  $conn = await connect($pool);
  $port = $conn->port();
  $conn->close();
  return $port;
}

<<__EntryPoint>>
async function run(): Awaitable<void> {
  $port = await get_port();
  \var_dump($port);
}
```.hhvm.expectf
int(%d)
```.example.hhvm.out
int(3306)
```.skipif
await \Hack\UserDocumentation\API\Examples\AsyncMysql\skipif_async();
~~~
<!-- HHAPIDOC -->
