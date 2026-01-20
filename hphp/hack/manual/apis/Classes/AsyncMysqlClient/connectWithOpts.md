
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Begin an async connection to a MySQL instance




``` Hack
public static function connectWithOpts(
  string $host,
  int $port,
  string $dbname,
  string $user,
  string $password,
  AsyncMysqlConnectionOptions $conn_opts,
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
+ [` AsyncMysqlConnectionOptions `](/apis/Classes/AsyncMysqlConnectionOptions/)`` $conn_opts ``




## Returns




* [` Awaitable<AsyncMysqlConnection> `](/apis/Classes/HH/Awaitable/) - an [` Awaitable `](/apis/Classes/HH/Awaitable/) representing an [` AsyncMysqlConnection `](/apis/Classes/AsyncMysqlConnection/). `` await ``
  or ``` join ``` this result to obtain the actual connection.
<!-- HHAPIDOC -->
