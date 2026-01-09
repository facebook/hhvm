
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns statistical information for the current pool




``` Hack
public function getPoolStats(): darray<string, mixed>;
```




Information provided includes the number of pool connections that were
created and destroyed, how many connections were requested, and how many
times the pool was hit or missed when creating the connection. The
returned ` array ` keys are:




+ ` created_pool_connections `
+ ` destroyed_pool_connections `
+ ` connections_requested `
+ ` pool_hits `
+ ` pool_misses `




## Returns




* ` darray<string, mixed> ` - A string-keyed `` array `` with the statistical information above.




## Examples




The following example shows how to gather ` AsyncMySqlConnectionPool ` statistics using its `` getStats() `` method. The statistics that are gathered are connection statistics.




~~~ basic-usage.hack
use \Hack\UserDocumentation\API\Examples\AsyncMysql\ConnectionInfo as CI;


function set_connection_pool(
  darray<string, mixed> $options = darray[],
): \AsyncMysqlConnectionPool {
  return new \AsyncMysqlConnectionPool($options);
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

function get_stats(\AsyncMysqlConnectionPool $pool): mixed {
  return $pool->getPoolStats();
}

<<__EntryPoint>>
async function run_it(): Awaitable<void> {
  $pool = set_connection_pool();
  $conn_awaitables = Vector {};
  $conn_awaitables[] = connect_with_pool($pool);
  $conn_awaitables[] = connect_with_pool($pool);
  $conn_awaitables[] = connect_with_pool($pool);
  $conns = await \HH\Asio\v($conn_awaitables);
  // Get pool connection stats, like pool connections created, how many
  // connections were requested, etc.
  \var_dump(get_stats($pool));
}
```.hhvm.expectf
darray(5) {
  ["created_pool_connections"]=>
  int(3)
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
darray(5) {
  ["created_pool_connections"]=>
  int(3)
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
