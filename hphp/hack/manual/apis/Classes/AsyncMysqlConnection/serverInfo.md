
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

The MySQL server version associated with the current connection




``` Hack
public function serverInfo(): string;
```




## Returns




+ ` string ` - The server version as a `` string ``.




## Examples




The following example shows how to get the version of the MySQL server that this connection is associated with via [` AsyncMysqlConnection::serverInfo `](/apis/Classes/AsyncMysqlConnection/serverInfo/).




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
async function get_server_info(): Awaitable<string> {
  $pool = new \AsyncMysqlConnectionPool(darray[]);
  $conn = await connect($pool);
  $info = $conn->serverInfo();
  $conn->close();
  return $info;
}

<<__EntryPoint>>
async function run(): Awaitable<void> {
  $info = await get_server_info();
  \var_dump($info);
}
```.hhvm.expectf
string(%d) "%s"
```.example.hhvm.out
string(23) "8.0.22-0ubuntu0.20.04.2"
```.skipif
await \Hack\UserDocumentation\API\Examples\AsyncMysql\skipif_async();
~~~
<!-- HHAPIDOC -->
