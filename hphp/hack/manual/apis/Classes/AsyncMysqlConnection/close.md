
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Close the current connection




``` Hack
public function close(): void;
```




## Returns




+ ` void `




## Examples




Closing a database connection is usually a good idea. This example shows the closing of a connection using [` AsyncMysqlConnection::close `](/apis/Classes/AsyncMysqlConnection/close/) and then tries to invoke a query on that connection afterwards.




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
async function simple_query(): Awaitable<int> {
  $pool = new \AsyncMysqlConnectionPool(darray[]);
  $conn = await connect($pool);
  $result = await $conn->query('SELECT name FROM test_table WHERE userID = 1');
  $conn->close();
  try {
    $result2 = await $conn->query(
      'SELECT name FROM test_table WHERE userID = 1',
    );
  } catch (\Exception $ex) { // probably InvalidArgumentException on query
    echo "Connection closed\n";
  }
  return $result->numRows();
}

<<__EntryPoint>>
async function run(): Awaitable<void> {
  $r = await simple_query();
  \var_dump($r);
}
```.hhvm.expectf
Connection closed
int(%d)
```.example.hhvm.out
Connection closed
int(1)
```.skipif
await \Hack\UserDocumentation\API\Examples\AsyncMysql\skipif_async();
~~~
<!-- HHAPIDOC -->
