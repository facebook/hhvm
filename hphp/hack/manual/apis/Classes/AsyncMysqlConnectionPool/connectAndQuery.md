
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

``` Hack
public function connectAndQuery(
  Traversable<string, arraykey> $queries,
  string $host,
  int $port,
  string $dbname,
  string $user,
  string $password,
  AsyncMysqlConnectionOptions $conn_opts,
  string $extra_key = '',
  dict<string> $query_attributes = dict [
],
): Awaitable<(AsyncMysqlConnectResult, Vector<AsyncMysqlQueryResult>)>;
```




## Parameters




+ [` Traversable<string, `](/apis/Interfaces/HH/Traversable/)`` arraykey> $queries ``
+ ` string $host `
+ ` int $port `
+ ` string $dbname `
+ ` string $user `
+ ` string $password `
+ [` AsyncMysqlConnectionOptions `](/apis/Classes/AsyncMysqlConnectionOptions/)`` $conn_opts ``
+ ` string $extra_key = '' `
+ ` dict<string> $query_attributes = dict [ ] `




## Returns




* [` Awaitable<(AsyncMysqlConnectResult, `](/apis/Classes/HH/Awaitable/)`` Vector<AsyncMysqlQueryResult>)> ``
<!-- HHAPIDOC -->
