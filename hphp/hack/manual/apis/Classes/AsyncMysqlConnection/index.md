---
title: AsyncMysqlConnection
---


:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

An active connection to a MySQL instance




When you call ` connect() ` with a connection provided by the pool established
with [` AsyncMysqlConnectionPool `](/apis/Classes/AsyncMysqlConnectionPool/), you are returned this connection object to
actual do real work with the MySQL database, with the primary work being
querying the database itself.




## Guides




+ [Introduction](</hack/asynchronous-operations/introduction>)
+ [Extensions](</hack/asynchronous-operations/extensions>)







## Interface Synopsis




``` Hack
final class AsyncMysqlConnection {...}
```




### Public Methods




* [` ->close(): void `](/apis/Classes/AsyncMysqlConnection/close/)\
  Close the current connection
* [` ->connectResult(): ?AsyncMysqlConnectResult `](/apis/Classes/AsyncMysqlConnection/connectResult/)\
  Returns the [` AsyncMysqlConnectResult `](/apis/Classes/AsyncMysqlConnectResult/) for the current connection
* [` ->escapeString(string $data): string `](/apis/Classes/AsyncMysqlConnection/escapeString/)\
  Escape a string to be safe to include in a raw query
* [` ->getSslCertCn(): string `](/apis/Classes/AsyncMysqlConnection/getSslCertCn/)\
  Returns Common Name attribute of the TLS certificate presented
  by MySQL server
* [` ->getSslCertExtensions(): Vector<string> `](/apis/Classes/AsyncMysqlConnection/getSslCertExtensions/)\
  Returns values from the selected cert extensions of the TLS certificate
  presented by MySQL server
* [` ->getSslCertSan(): Vector<string> `](/apis/Classes/AsyncMysqlConnection/getSslCertSan/)\
  Returns Server Alternative Names attribute of the TLS certificate
  presented by MySQL server
* [` ->host(): string `](/apis/Classes/AsyncMysqlConnection/host/)\
  The hostname associated with the current connection
* [` ->isReusable(): bool `](/apis/Classes/AsyncMysqlConnection/isReusable/)\
  Returns whether or not the current connection is reusable
* [` ->isSSL(): bool `](/apis/Classes/AsyncMysqlConnection/isSSL/)\
  Returns whether or not the current connection was established as SSL based
  on client flag exchanged during handshake
* [` ->isSslCertValidationEnforced(): bool `](/apis/Classes/AsyncMysqlConnection/isSslCertValidationEnforced/)\
  Returns a boolean value indicating if server cert validation was enforced
  for this connection
* [` ->isValid(): bool `](/apis/Classes/AsyncMysqlConnection/isValid/)\
  Checks if the data inside `` AsyncMysqlConnection `` object is valid
* [` ->lastActivityTime(): float `](/apis/Classes/AsyncMysqlConnection/lastActivityTime/)\
  Last time a successful activity was made in the current connection, in
  seconds since epoch
* [` ->multiQuery(Traversable<string, arraykey, mixed> $queries, int $timeout_micros = -1, dict<string> $query_attributes = dict [ ]): Awaitable<Vector<AsyncMysqlQueryResult>> `](/apis/Classes/AsyncMysqlConnection/multiQuery/)\
  Begin running a query with multiple statements
* [` ->port(): int `](/apis/Classes/AsyncMysqlConnection/port/)\
  The port on which the MySQL instance is running
* [` ->query(string $query, int $timeout_micros = -1, dict<string> $query_attributes = dict [ ]): Awaitable<AsyncMysqlQueryResult> `](/apis/Classes/AsyncMysqlConnection/query/)\
  Begin running an unsafe query on the MySQL database client
* [` ->queryAsync(HH\Lib\SQL\Query $query): Awaitable<AsyncMysqlQueryResult> `](/apis/Classes/AsyncMysqlConnection/queryAsync/)
* [` ->queryf(HH\FormatString<HH\SQLFormatter> $pattern, ...$args): Awaitable<AsyncMysqlQueryResult> `](/apis/Classes/AsyncMysqlConnection/queryf/)\
  Execute a query with placeholders and parameters
* [` ->releaseConnection(): mixed `](/apis/Classes/AsyncMysqlConnection/releaseConnection/)\
  Releases the current connection and returns a synchronous MySQL connection
* [` ->serverInfo(): string `](/apis/Classes/AsyncMysqlConnection/serverInfo/)\
  The MySQL server version associated with the current connection
* [` ->setReusable(bool $reusable): void `](/apis/Classes/AsyncMysqlConnection/setReusable/)\
  Sets if the current connection can be recycled without any clean up
* [` ->sslSessionReused(): bool `](/apis/Classes/AsyncMysqlConnection/sslSessionReused/)\
  Returns whether or not the current connection reused the SSL session
  from another SSL connection
* [` ->warningCount(): int `](/apis/Classes/AsyncMysqlConnection/warningCount/)\
  The number of errors, warnings, and notes returned during execution of
  the previous SQL statement
<!-- HHAPIDOC -->
