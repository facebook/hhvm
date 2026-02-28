---
title: AsyncMysqlConnectionPool
---


:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

An asynchronous MySQL connection pool




This class provides a mechanism to create a pool of connections to a MySQL
client that can be utilized and reused as needed.




When a client requests a connection from the pool, it may get one that
already exists; this avoids the overhead of establishing a new connection.




This is the *highly recommended* way to create connections to a MySQL
client, as opposed to using the [` AsyncMysqlClient `](/apis/Classes/AsyncMysqlClient/) class which does not give
you nearly the flexibility. In fact, there is discussion about deprecating
the [` AsyncMysqlClient `](/apis/Classes/AsyncMysqlClient/) class all together.




## Guides




+ [Introduction](</hack/asynchronous-operations/introduction>)
+ [Extensions](</hack/asynchronous-operations/extensions>)







## Interface Synopsis




``` Hack
class AsyncMysqlConnectionPool {...}
```




### Public Methods




* [` ->__construct(darray<string, mixed> $pool_options): void `](/apis/Classes/AsyncMysqlConnectionPool/__construct/)\
  Create a pool of connections to access a MySQL client
* [` ->connect(string $host, int $port, string $dbname, string $user, string $password, int $timeout_micros = -1, string $extra_key = '', ?MySSLContextProvider $ssl_provider = NULL, int $tcp_timeout_micros = 0, string $sni_server_name = '', string $server_cert_extensions = '', string $server_cert_values = ''): Awaitable<AsyncMysqlConnection> `](/apis/Classes/AsyncMysqlConnectionPool/connect/)\
  Begin an async connection to a MySQL instance
* [` ->connectAndQuery(Traversable<string, arraykey> $queries, string $host, int $port, string $dbname, string $user, string $password, AsyncMysqlConnectionOptions $conn_opts, string $extra_key = '', dict<string> $query_attributes = dict [ ]): Awaitable<(AsyncMysqlConnectResult, Vector<AsyncMysqlQueryResult>)> `](/apis/Classes/AsyncMysqlConnectionPool/connectAndQuery/)
* [` ->connectWithOpts(string $host, int $port, string $dbname, string $user, string $password, AsyncMysqlConnectionOptions $conn_options, string $extra_key = ''): Awaitable<AsyncMysqlConnection> `](/apis/Classes/AsyncMysqlConnectionPool/connectWithOpts/)
* [` ->getPoolStats(): darray<string, mixed> `](/apis/Classes/AsyncMysqlConnectionPool/getPoolStats/)\
  Returns statistical information for the current pool
<!-- HHAPIDOC -->
