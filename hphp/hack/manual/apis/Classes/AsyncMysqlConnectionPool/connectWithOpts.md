
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

``` Hack
public function connectWithOpts(
  string $host,
  int $port,
  string $dbname,
  string $user,
  string $password,
  AsyncMysqlConnectionOptions $conn_options,
  string $extra_key = '',
): Awaitable<AsyncMysqlConnection>;
```




## Parameters




+ ` string $host `
+ ` int $port `
+ ` string $dbname `
+ ` string $user `
+ ` string $password `
+ [` AsyncMysqlConnectionOptions `](/apis/Classes/AsyncMysqlConnectionOptions/)`` $conn_options ``
+ ` string $extra_key = '' `




## Returns




* [` Awaitable<AsyncMysqlConnection> `](/apis/Classes/HH/Awaitable/)
<!-- HHAPIDOC -->
