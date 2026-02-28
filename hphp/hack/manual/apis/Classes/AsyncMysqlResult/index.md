---
title: AsyncMysqlResult
---


:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

A base class for connection, query and error results




This class is ` abstract ` and cannot be instantiated, but provides the methods
that concrete classes must implement, which are timing information methods
regarding a query, connection or a resulting error.




## Guides




+ [Introduction](</hack/asynchronous-operations/introduction>)
+ [Extensions](</hack/asynchronous-operations/extensions>)







## Interface Synopsis




``` Hack
abstract class AsyncMysqlResult {...}
```




### Public Methods




* [` ->clientStats(): AsyncMysqlClientStats `](/apis/Classes/AsyncMysqlResult/clientStats/)\
  Returns the MySQL client statistics at the moment the result was created
* [` ->elapsedMicros(): int `](/apis/Classes/AsyncMysqlResult/elapsedMicros/)\
  The total time for the specific MySQL operation, in microseconds
* [` ->endTime(): float `](/apis/Classes/AsyncMysqlResult/endTime/)\
  The end time for the specific MySQL operation, in seconds since epoch
* [` ->getSslCertCn(): string `](/apis/Classes/AsyncMysqlResult/getSslCertCn/)\
  Returns Common Name attribute of the TLS certificate presented
  by MySQL server
* [` ->getSslCertExtensions(): Vector<string> `](/apis/Classes/AsyncMysqlResult/getSslCertExtensions/)\
  Returns values from the selected cert extensions of the TLS certificate
  presented by MySQL server
* [` ->getSslCertSan(): Vector<string> `](/apis/Classes/AsyncMysqlResult/getSslCertSan/)\
  Returns Server Alternative Names attribute of the TLS certificate
  presented by MySQL server
* [` ->isSslCertValidationEnforced(): bool `](/apis/Classes/AsyncMysqlResult/isSslCertValidationEnforced/)\
  Returns a boolean value indicating if server cert validation was enforced
  for this connection
* [` ->sslSessionReused(): bool `](/apis/Classes/AsyncMysqlResult/sslSessionReused/)\
  Returns whether or not the current connection reused the SSL session
  from another SSL connection
* [` ->startTime(): float `](/apis/Classes/AsyncMysqlResult/startTime/)\
  The start time for the specific MySQL operation, in seconds since epoch
<!-- HHAPIDOC -->
