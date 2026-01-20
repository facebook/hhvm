
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Sets the connection limit of all connection pools using this client




``` Hack
public static function setPoolsConnectionLimit(
  int $limit,
): void;
```




Use this function to toggle the number of allowed async connections on the
pools connecting to MySQL with this current client. For example, if you
set the limit to 2, and you try a third connection on the same pool, an
[` AsyncMysqlConnectException `](/apis/Classes/AsyncMysqlConnectException/) exception will be thrown.




## Parameters




+ ` int $limit ` - The limit for all pools.




## Returns




* ` void `




## Examples




You can use [` AsyncMysqlClient::setPoolsConnectionLimit() `](/apis/Classes/AsyncMysqlClient/setPoolsConnectionLimit/) to toggle the number of allowed async connections on the client. In this example, we are setting the number of allowed pool connections to be 2, but trying to do 3 connections, and that ends up giving an exception.




~~~ basic-usage.hack
use \Hack\UserDocumentation\API\Examples\AsyncMysql\ConnectionInfo as CI;

function set_connection_pool(): \AsyncMysqlConnectionPool {
  return new \AsyncMysqlConnectionPool(darray[]);
}

async function connect_with_pool(
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

function get_stats(\AsyncMysqlConnectionPool $pool): dict<string, mixed> {
  return dict($pool->getPoolStats());
}

<<__EntryPoint>>
async function run_it(): Awaitable<void> {
  \AsyncMysqlClient::setPoolsConnectionLimit(2); // limit two connections
  $pool = set_connection_pool();
  $conn_awaitables = Vector {};
  try {
    // One of these 3 connections here will throw the exception when we join
    $conn_awaitables[] = connect_with_pool($pool);
    $conn_awaitables[] = connect_with_pool($pool);
    $conn_awaitables[] = connect_with_pool($pool);
    $conns = await \HH\Asio\v($conn_awaitables);
  } catch (\AsyncMysqlConnectException $ex) {
    \var_dump(get_stats($pool));
  }
}
```.hhvm.expectf
dict(5) {
  ["created_pool_connections"]=>
  int(2)
  ["destroyed_pool_connections"]=>
  int(0)
  ["connections_requested"]=>
  int(3)
  ["pool_hits"]=>
  int(%d)
  ["pool_misses"]=>
  int(%d)
}
```.example.hhvm.out
dict(5) {
  ["created_pool_connections"]=>
  int(2)
  ["destroyed_pool_connections"]=>
  int(0)
  ["connections_requested"]=>
  int(3)
  ["pool_hits"]=>
  int(0)
  ["pool_misses"]=>
  int(3)
}
```.skipif
await \Hack\UserDocumentation\API\Examples\AsyncMysql\skipif_async();
~~~
<!-- HHAPIDOC -->
