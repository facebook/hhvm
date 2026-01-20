---
title: AsyncMysqlConnectionOptions
---


:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

This class holds the Connection Options that MySQL client will use to
establish a connection




The ` AsyncMysqlConnectionOptions ` will be passed to
[` AsyncMysqlClient::connectWithOpts() `](/apis/Classes/AsyncMysqlClient/connectWithOpts/).




## Guides




+ [Introduction](</hack/asynchronous-operations/introduction>)
+ [Extensions](</hack/asynchronous-operations/extensions>)







## Interface Synopsis




``` Hack
class AsyncMysqlConnectionOptions {...}
```




### Public Methods




* [` ->enableChangeUser(): void `](/apis/Classes/AsyncMysqlConnectionOptions/enableChangeUser/)
* [` ->enableDelayedResetConn(): void `](/apis/Classes/AsyncMysqlConnectionOptions/enableDelayedResetConn/)
* [` ->enableResetConnBeforeClose(): void `](/apis/Classes/AsyncMysqlConnectionOptions/enableResetConnBeforeClose/)
* [` ->setConnectAttempts(int $attempts): void `](/apis/Classes/AsyncMysqlConnectionOptions/setConnectAttempts/)
* [` ->setConnectTcpTimeout(int $timeout): void `](/apis/Classes/AsyncMysqlConnectionOptions/setConnectTcpTimeout/)
* [` ->setConnectTimeout(int $timeout): void `](/apis/Classes/AsyncMysqlConnectionOptions/setConnectTimeout/)
* [` ->setConnectionAttributes(darray<string> $attrs): void `](/apis/Classes/AsyncMysqlConnectionOptions/setConnectionAttributes/)
* [` ->setQueryTimeout(int $timeout): void `](/apis/Classes/AsyncMysqlConnectionOptions/setQueryTimeout/)
* [` ->setSSLOptionsProvider(?MySSLContextProvider $ssl_context): void `](/apis/Classes/AsyncMysqlConnectionOptions/setSSLOptionsProvider/)
* [` ->setServerCertValidation(string $extensions = '', string $values = ''): void `](/apis/Classes/AsyncMysqlConnectionOptions/setServerCertValidation/)
* [` ->setSniServerName(string $sni_server_name): void `](/apis/Classes/AsyncMysqlConnectionOptions/setSniServerName/)
* [` ->setTotalTimeout(int $timeout): void `](/apis/Classes/AsyncMysqlConnectionOptions/setTotalTimeout/)
<!-- HHAPIDOC -->
