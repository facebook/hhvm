
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns whether or not the current connection is reusable




``` Hack
public function isReusable(): bool;
```




By default, the current connection is reusable by the pool. But if you call
` setResuable(false) `, then the current connection will not be reusable by
the connection pool.




## Returns




+ ` bool ` - `` true `` if the connection is reusable; ``` false ``` otherwise.




## Examples




The following example shows how to make a connection not reusable in a connection pool with [` AsyncMysqlConnection::setReusable `](/apis/Classes/AsyncMysqlConnection/setReusable/) and test it with [` AsyncMysqlConnection::isReusable `](/apis/Classes/AsyncMysqlConnection/isReusable/). By default, connections in pools are reusable. So, here we create a pool connection that is assigned to `` $conn ``. When we close ``` $conn ```, that destroys that connection permanently. So when we get ```` $conn2 ````, a whole new connection will need to be created since we can't use the connection that was associated to ````` $conn `````.




~~~ basic-usage.hack
use \Hack\UserDocumentation\API\Examples\AsyncMysql\ConnectionInfo as CI;

async function connect(
  \AsyncMysqlConnectionPool $pool,
): Awaitable<\AsyncMysqlConnection> {
  $conn = await $pool->connect(
    CI::$host,
    CI::$port,
    CI::$db,
    CI::$user,
    CI::$passwd,
  );
  // By default pool connections are automatically set to be reusable
  $conn->setReusable(false);
  return $conn;
}

async function simple_query(\AsyncMysqlConnection $conn): Awaitable<int> {
  $result = await $conn->query('SELECT name FROM test_table WHERE userID = 1');
  return $result->numRows();
}

async function simple_query_2(\AsyncMysqlConnection $conn): Awaitable<int> {
  $result = await $conn->query('SELECT name FROM test_table WHERE userID = 2');
  return $result->numRows();
}

async function get_connection(
  \AsyncMysqlConnectionPool $pool,
): Awaitable<\AsyncMysqlConnection> {
  return await connect($pool);
}

function get_pool(): \AsyncMysqlConnectionPool {
  $options = darray[
    'pool_connection_limit' => 1,
  ];
  return new \AsyncMysqlConnectionPool($options);
}

<<__EntryPoint>>
async function run(): Awaitable<void> {
  $pool = get_pool();

  $conn = await get_connection($pool);
  // This will be false now
  \var_dump($conn->isReusable());
  $r2 = await simple_query($conn);
  $conn->close();

  $conn2 = await get_connection($pool);
  $r2 = await simple_query_2($conn2);
  // You will see one destroyed pool connection since we close $conn above
  // and we didn't set it to be reusable
  \var_dump($pool->getPoolStats());
  $conn2->close();
}
```.hhvm.expectf
bool(false)
darray(5) {
  ["created_pool_connections"]=>
  int(2)
  ["destroyed_pool_connections"]=>
  int(1)
  ["connections_requested"]=>
  int(2)
  ["pool_hits"]=>
  int(%d)
  ["pool_misses"]=>
  int(%d)
}
```.example.hhvm.out
bool(false)
darray(5) {
  ["created_pool_connections"]=>
  int(2)
  ["destroyed_pool_connections"]=>
  int(1)
  ["connections_requested"]=>
  int(2)
  ["pool_hits"]=>
  int(0)
  ["pool_misses"]=>
  int(2)
}
```.skipif
await \Hack\UserDocumentation\API\Examples\AsyncMysql\skipif_async();
~~~
<!-- HHAPIDOC -->
