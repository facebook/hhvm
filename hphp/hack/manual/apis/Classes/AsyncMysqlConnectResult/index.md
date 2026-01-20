---
title: AsyncMysqlConnectResult
---


:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Provides the result information for when the connection to the MySQL
client is made successfully




This class is instantiated through a call from the connection object
via [` AsyncMysqlConnection::connectResult() `](/apis/Classes/AsyncMysqlConnection/connectResult/).




## Guides




+ [Introduction](</hack/asynchronous-operations/introduction>)
+ [Extensions](</hack/asynchronous-operations/extensions>)







## Interface Synopsis




``` Hack
final class AsyncMysqlConnectResult extends AsyncMysqlResult {...}
```




### Public Methods




* [` ->clientStats(): AsyncMysqlClientStats `](/apis/Classes/AsyncMysqlConnectResult/clientStats/)\
  Returns the MySQL client statistics at the moment the connection was
  established
* [` ->elapsedMicros(): int `](/apis/Classes/AsyncMysqlConnectResult/elapsedMicros/)\
  The total time for the establishment of the MySQL connection,
  in microseconds
* [` ->endTime(): float `](/apis/Classes/AsyncMysqlConnectResult/endTime/)\
  The end time of the connection operation, in seconds since epoch
* [` ->startTime(): float `](/apis/Classes/AsyncMysqlConnectResult/startTime/)\
  The start time for the connection operation, in seconds since epoch







### Public Methods ([` AsyncMysqlResult `](/apis/Classes/AsyncMysqlResult/))




- [` ->getSslCertCn(): string `](/apis/Classes/AsyncMysqlResult/getSslCertCn/)\
  Returns Common Name attribute of the TLS certificate presented
  by MySQL server
- [` ->getSslCertExtensions(): Vector<string> `](/apis/Classes/AsyncMysqlResult/getSslCertExtensions/)\
  Returns values from the selected cert extensions of the TLS certificate
  presented by MySQL server
- [` ->getSslCertSan(): Vector<string> `](/apis/Classes/AsyncMysqlResult/getSslCertSan/)\
  Returns Server Alternative Names attribute of the TLS certificate
  presented by MySQL server
- [` ->isSslCertValidationEnforced(): bool `](/apis/Classes/AsyncMysqlResult/isSslCertValidationEnforced/)\
  Returns a boolean value indicating if server cert validation was enforced
  for this connection
- [` ->sslSessionReused(): bool `](/apis/Classes/AsyncMysqlResult/sslSessionReused/)\
  Returns whether or not the current connection reused the SSL session
  from another SSL connection
<!-- HHAPIDOC -->
