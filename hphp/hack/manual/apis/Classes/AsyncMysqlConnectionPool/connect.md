
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Begin an async connection to a MySQL instance




``` Hack
public function connect(
  string $host,
  int $port,
  string $dbname,
  string $user,
  string $password,
  int $timeout_micros = -1,
  string $extra_key = '',
  ?MySSLContextProvider $ssl_provider = NULL,
  int $tcp_timeout_micros = 0,
  string $sni_server_name = '',
  string $server_cert_extensions = '',
  string $server_cert_values = '',
): Awaitable<AsyncMysqlConnection>;
```




Once you have your pool constructed, you use this method to connect to the
MySQL client. The connection pool will either create a new connection or
use one of the recently available connections from the pool itself.




## Parameters




+ ` string $host ` - The hostname to connect to.
+ ` int $port ` - The port to connect to.
+ ` string $dbname ` - The initial database to use when connecting.
+ ` string $user ` - The user to connect as.
+ ` string $password ` - The password to connect with.
+ ` int $timeout_micros = -1 ` - Timeout, in microseconds, for the connect; -1
  for default, 0 for no timeout.
+ ` string $extra_key = '' ` - An extra parameter to help the internal connection
  pool infrastructure separate connections even better.
  Usually, the default `` "" `` is fine.
+ ` ?MySSLContextProvider $ssl_provider = NULL `
+ ` int $tcp_timeout_micros = 0 ` - Timeout, in microseconds, for the tcp phase of
  connect operation; Default: 0 for no timeout.
+ ` string $sni_server_name = '' ` - SNI hostname to use when connecting via SSL.
+ ` string $server_cert_extensions = '' ` - collection of name of TLS cert extension
  names used to validate server cert
+ ` string $server_cert_values = '' ` - collection of accepted values in server cert
  for "server_cert_extension" extension




## Returns




* [` Awaitable<AsyncMysqlConnection> `](/apis/Classes/HH/Awaitable/) - an [` Awaitable `](/apis/Classes/HH/Awaitable/) representing an [` AsyncMysqlConnection `](/apis/Classes/AsyncMysqlConnection/). `` await ``
  or ``` join ``` this result to obtain the actual connection.




## Examples




It is **highly recommended** that you use connection pools when using async MySQL. That way you don't have to create a new connection every time you want to make a query to the database. The following example shows you how to connect to a MySQL database using an ` AsyncMySqlConnectionPool `.




~~~ basic-usage.hack
use \Hack\UserDocumentation\API\Examples\AsyncMysql\ConnectionInfo as CI;

class MyPool {
  private \AsyncMysqlConnectionPool $pool;

  public function __construct() {
    $this->pool = new \AsyncMysqlConnectionPool(darray[]);
  }

  public function getPool(): \AsyncMysqlConnectionPool {
    return $this->pool;
  }

  public async function connect(): Awaitable<\AsyncMysqlConnection> {
    return await $this->pool
      ->connect(CI::$host, CI::$port, CI::$db, CI::$user, CI::$passwd);
  }
}


async function get_num_rows(\AsyncMysqlConnection $conn): Awaitable<int> {
  $result = await $conn->query('SELECT * FROM test_table');
  return $result->numRows();
}

async function get_row_data(
  \AsyncMysqlConnection $conn,
): Awaitable<Vector<KeyedContainer<int, ?string>>> {
  $result = await $conn->query('SELECT * FROM test_table');
  return $result->vectorRows();
}

async function run_it_1(MyPool $pool): Awaitable<void> {
  $conn = await $pool->connect();
  $rows = await get_num_rows($conn);
  \var_dump($rows);
}

async function run_it_2(MyPool $pool): Awaitable<void> {
  $conn = await $pool->connect();
  $data = await get_row_data($conn);
  \var_dump($data->count());
  // Should show only one created pool connection since we are pooling it
  \var_dump($pool->getPool()->getPoolStats());
}

<<__EntryPoint>>
async function run(): Awaitable<void> {
  $pool = new MyPool();
  await run_it_1($pool);
  await run_it_2($pool);
}
```.hhvm.expectf
int(%d)
int(%d)
darray(5) {
  ["created_pool_connections"]=>
  int(1)
  ["destroyed_pool_connections"]=>
  int(0)
  ["connections_requested"]=>
  int(2)
  ["pool_hits"]=>
  int(%d)
  ["pool_misses"]=>
  int(%d)
}
```.example.hhvm.out
int(21)
int(21)
darray(5) {
  ["created_pool_connections"]=>
  int(1)
  ["destroyed_pool_connections"]=>
  int(0)
  ["connections_requested"]=>
  int(2)
  ["pool_hits"]=>
  int(1)
  ["pool_misses"]=>
  int(1)
}
```.skipif
await \Hack\UserDocumentation\API\Examples\AsyncMysql\skipif_async();
~~~
<!-- HHAPIDOC -->
