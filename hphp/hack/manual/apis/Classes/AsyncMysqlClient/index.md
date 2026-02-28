---
title: AsyncMysqlClient
---


:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

An asynchronous MySQL client




This class allows you to asynchronously connect to a MySQL client. You
can directly connect to the MySQL client with the [` connect() `](/apis/Classes/AsyncMysqlClient/connect/) method; in
addition you can use this class in conjunction with
[` AsyncMysqlConnectionPool `](/apis/Classes/AsyncMysqlConnectionPool/) pools by setting the limit of connections on
any given pool, and using [` AsyncMysqlConnectionPool::connect() `](/apis/Classes/AsyncMysqlConnectionPool/connect/).




There is some duplication with this class. If possible, you should directly
construct connection pools via ` new AsyncMysqlConnectionPool() ` and then
call [` AsyncMysqlConnectionPool::connect() `](/apis/Classes/AsyncMysqlConnectionPool/connect/) to connect to the MySQL client
using those pools. Here we optionally set pool limits and call [` connect() `](/apis/Classes/AsyncMysqlClient/connect/)
on this class. [` AsyncMysqlConnectionPool `](/apis/Classes/AsyncMysqlConnectionPool/) provides more flexibility with
other available options, etc.




In fact, there is discussion about deprecating ` AsyncMysqlClient ` all
together to avoid having this choice. But, for now, you can use this class
for asynchronous connection(s) to a MySQL database.




## Guides




+ [Extensions](</hack/asynchronous-operations/extensions>)
+ [Introduction](</hack/asynchronous-operations/introduction>)







## Interface Synopsis




``` Hack
final class AsyncMysqlClient {...}
```




### Public Methods




* [` ::connect(string $host, int $port, string $dbname, string $user, string $password, int $timeout_micros = -1, ?MySSLContextProvider $ssl_provider = NULL, int $tcp_timeout_micros = 0, string $sni_server_name = '', string $server_cert_extensions = '', string $server_cert_values = ''): Awaitable<AsyncMysqlConnection> `](/apis/Classes/AsyncMysqlClient/connect/)\
  Begin an async connection to a MySQL instance
* [` ::connectAndQuery(Traversable<string, arraykey> $queries, string $host, int $port, string $dbname, string $user, string $password, AsyncMysqlConnectionOptions $conn_opts, dict<string> $query_attributes = dict [ ]): Awaitable<(AsyncMysqlConnectResult, Vector<AsyncMysqlQueryResult>)> `](/apis/Classes/AsyncMysqlClient/connectAndQuery/)\
  Begin an async connection and query  to a MySQL instance
* [` ::connectWithOpts(string $host, int $port, string $dbname, string $user, string $password, AsyncMysqlConnectionOptions $conn_opts): Awaitable<AsyncMysqlConnection> `](/apis/Classes/AsyncMysqlClient/connectWithOpts/)\
  Begin an async connection to a MySQL instance
* [` ::setPoolsConnectionLimit(int $limit): void `](/apis/Classes/AsyncMysqlClient/setPoolsConnectionLimit/)\
  Sets the connection limit of all connection pools using this client
<!-- HHAPIDOC -->
