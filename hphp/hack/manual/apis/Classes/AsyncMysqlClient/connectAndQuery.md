
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Begin an async connection and query  to a MySQL instance




``` Hack
public static function connectAndQuery(
  Traversable<string, arraykey> $queries,
  string $host,
  int $port,
  string $dbname,
  string $user,
  string $password,
  AsyncMysqlConnectionOptions $conn_opts,
  dict<string> $query_attributes = dict [
],
): Awaitable<(AsyncMysqlConnectResult, Vector<AsyncMysqlQueryResult>)>;
```




Use this to asynchronously connect and query a MySQL instance.




Normally you would use this to make one query to the
MySQL client.




If you want to be able to reuse the connection use connect or
connectWithOpts




## Parameters




+ [` Traversable<string, `](/apis/Interfaces/HH/Traversable/)`` arraykey> $queries ``
+ ` string $host ` - The hostname to connect to.
+ ` int $port ` - The port to connect to.
+ ` string $dbname ` - The initial database to use when connecting.
+ ` string $user ` - The user to connect as.
+ ` string $password ` - The password to connect with.
+ [` AsyncMysqlConnectionOptions `](/apis/Classes/AsyncMysqlConnectionOptions/)`` $conn_opts ``
+ ` dict<string> $query_attributes = dict [ ] ` - Query attributes. Empty by default.




## Returns




* [` Awaitable<(AsyncMysqlConnectResult, `](/apis/Classes/HH/Awaitable/)`` Vector<AsyncMysqlQueryResult>)> `` - an [` Awaitable `](/apis/Classes/HH/Awaitable/) representing the result of your connect and query
  This is a tuple where the latter contains information about the connection
  retrieval, and the former has the query results
<!-- HHAPIDOC -->
