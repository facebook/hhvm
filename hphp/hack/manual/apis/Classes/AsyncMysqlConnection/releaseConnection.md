
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Releases the current connection and returns a synchronous MySQL connection




``` Hack
public function releaseConnection(): mixed;
```




This method will destroy the current [` AsyncMysqlConnection `](/apis/Classes/AsyncMysqlConnection/) object and give
you back a vanilla, synchronous MySQL resource.




## Returns




+ ` mixed ` - A `` resource `` representing a
  [MySQL](<http://php.net/manual/en/book.mysql.php>) resource, or
  ` false ` on failure.




## Examples




If you ever want to get a plain, vanilla synchronous MySQL connection from your async connection, you call [` AsyncMysqlConnection::releaseConnection `](/apis/Classes/AsyncMysqlConnection/releaseConnection/). This examples show how to get such a connection, noting too that your async connection is destroyed.




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
  $sync_connection = $conn->releaseConnection();
  \var_dump($sync_connection);
  try {
    $result2 = await $conn->query(
      'SELECT name FROM test_table WHERE userID = 1',
    );
  } catch (\Exception $ex) { // probably InvalidArgumentException on query
    echo "Connection destroyed when released\n";
  }
  // This call will block since it is not async
  $sync_result = \mysql_query(
    'SELECT name FROM test_table WHERE userID = 1',
    $sync_connection,
  );
  $sync_rows = \mysql_num_rows($sync_result);
  \mysql_close($sync_connection);
  return $result->numRows() + $sync_rows;
}

<<__EntryPoint>>
async function run(): Awaitable<void> {
  $r = await simple_query();
  \var_dump($r);
}
```.hhvm.expectf
resource(%d) of type (mysql link)
Connection destroyed when released
int(%d)
```.example.hhvm.out
resource(4) of type (mysql link)
Connection destroyed when released
int(2)
```.skipif
await \Hack\UserDocumentation\API\Examples\AsyncMysql\skipif_async();
~~~
<!-- HHAPIDOC -->
