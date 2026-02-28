---
title: AsyncMysqlErrorResult
---


:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Contains error information for a failed operation (e.g., connection, query)




This class is instantiated when an [` AsyncMysqlException `](/apis/Classes/AsyncMysqlException/) is thrown and
[` AsyncMysqlException::getResult() `](/apis/Classes/AsyncMysqlException/getResult/) is called.




## Guides




+ [Introduction](</hack/asynchronous-operations/introduction>)
+ [Extensions](</hack/asynchronous-operations/extensions>)







## Interface Synopsis




``` Hack
class AsyncMysqlErrorResult extends AsyncMysqlResult {...}
```




### Public Methods




* [` ->clientStats(): AsyncMysqlClientStats `](/apis/Classes/AsyncMysqlErrorResult/clientStats/)\
  Returns the MySQL client statistics for the events that produced the error
* [` ->elapsedMicros(): int `](/apis/Classes/AsyncMysqlErrorResult/elapsedMicros/)\
  The total time for the MySQL error condition to occur, in microseconds
* [` ->endTime(): float `](/apis/Classes/AsyncMysqlErrorResult/endTime/)\
  The end time when the error was produced, in seconds since epoch
* [` ->failureType(): string `](/apis/Classes/AsyncMysqlErrorResult/failureType/)\
  The type of failure that produced this result
* [` ->mysql_errno(): int `](/apis/Classes/AsyncMysqlErrorResult/mysql_errno/)\
  Returns the MySQL error number for this result
* [` ->mysql_error(): string `](/apis/Classes/AsyncMysqlErrorResult/mysql_error/)\
  Returns a human-readable string for the error encountered in this result
* [` ->mysql_normalize_error(): string `](/apis/Classes/AsyncMysqlErrorResult/mysql_normalize_error/)\
  Returns an alternative, normalized version of the error message provided by
  mysql_error()
* [` ->startTime(): float `](/apis/Classes/AsyncMysqlErrorResult/startTime/)\
  The start time when the error was produced, in seconds since epoch







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
