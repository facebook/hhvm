
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Begin an async connection to a MySQL instance




``` Hack
public static function connect(
  string $host,
  int $port,
  string $dbname,
  string $user,
  string $password,
  int $timeout_micros = -1,
  ?MySSLContextProvider $ssl_provider = NULL,
  int $tcp_timeout_micros = 0,
  string $sni_server_name = '',
  string $server_cert_extensions = '',
  string $server_cert_values = '',
): Awaitable<AsyncMysqlConnection>;
```




Use this to asynchronously connect to a MySQL instance.




Normally you would use this to make one asynchronous connection to the
MySQL client.




If you want to be able to pool up a bunch of connections, you would call
[` setPoolsConnectionLimit() `](/apis/Classes/AsyncMysqlClient/setPoolsConnectionLimit/), create a default pool of connections with
[` AsyncMysqlConnectionPool()::__construct() `](/apis/Classes/AsyncMysqlConnectionPool/), which now
has that limit set, and then call [` AsyncMysqlConnectionPool()::connect() `](/apis/Classes/AsyncMysqlConnectionPool/).




## Parameters




+ ` string $host ` - The hostname to connect to.
+ ` int $port ` - The port to connect to.
+ ` string $dbname ` - The initial database to use when connecting.
+ ` string $user ` - The user to connect as.
+ ` string $password ` - The password to connect with.
+ ` int $timeout_micros = -1 ` - Timeout, in microseconds, for the connect; -1 for
  default, 0 for no timeout.
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




The following example shows how to use [` AsyncMysqlClient::connect() `](/apis/Classes/AsyncMysqlClient/connect/) to connect to a database asynchronously and get a result from that connection. Notice a couple of things:




- The parameters to [` connect() `](/apis/Classes/AsyncMysqlClient/connect/) are very similar to that of a normal [` mysqli ` connection](<http://php.net/manual/en/mysqli.construct.php>).
- With [` AsyncMysqlClient `](/apis/Classes/AsyncMysqlClient/), we are able to take full advantage of [async](</hack/asynchronous-operations/introduction>) to perform other DB connection or I/O operations while waiting for this connection to return.




~~~ basic-usage.hack
use \Hack\UserDocumentation\API\Examples\AsyncMysql\ConnectionInfo as CI;

async function do_connect(): Awaitable<\AsyncMysqlQueryResult> {
  // Cast because the array from get_connection_info() is a mixed
  $conn = await \AsyncMysqlClient::connect(
    CI::$host,
    CI::$port,
    CI::$db,
    CI::$user,
    CI::$passwd,
  );
  return await $conn->query('SELECT * FROM test_table');
}

<<__EntryPoint>>
async function run_it(): Awaitable<void> {
  $res = await do_connect();
  \var_dump($res->numRows()); // The number of rows from the SELECT statement
}
```.hhvm.expectf
int(%d)
```.example.hhvm.out
int(21)
```.skipif
await \Hack\UserDocumentation\API\Examples\AsyncMysql\skipif_async();
~~~
<!-- HHAPIDOC -->
