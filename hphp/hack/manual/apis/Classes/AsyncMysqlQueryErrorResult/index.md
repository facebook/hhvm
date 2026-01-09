---
title: AsyncMysqlQueryErrorResult
---


:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Contains the information about results for query statements that ran before
a MySQL error




This class is instantiated when an [` AsyncMysqlQueryException `](/docs/apis/Classes/AsyncMysqlQueryException/) is thrown and
`` AsyncMysqlQueryException::getResult() `` is called.




## Guides




+ [Introduction](</docs/hack/asynchronous-operations/introduction>)
+ [Extensions](</docs/hack/asynchronous-operations/extensions>)







## Interface Synopsis




``` Hack
final class AsyncMysqlQueryErrorResult extends AsyncMysqlErrorResult {...}
```




### Public Methods




* [` ->getSuccessfulResults(): Vector<AsyncMysqlQueryResult> `](/docs/apis/Classes/AsyncMysqlQueryErrorResult/getSuccessfulResults/)\
  Returns the results that were fetched by the successful query statements
* [` ->numSuccessfulQueries(): int `](/docs/apis/Classes/AsyncMysqlQueryErrorResult/numSuccessfulQueries/)\
  Returns the number of successfully executed queries







### Public Methods ([` AsyncMysqlErrorResult `](/docs/apis/Classes/AsyncMysqlErrorResult/))




- [` ->clientStats(): AsyncMysqlClientStats `](/docs/apis/Classes/AsyncMysqlErrorResult/clientStats/)\
  Returns the MySQL client statistics for the events that produced the error
- [` ->elapsedMicros(): int `](/docs/apis/Classes/AsyncMysqlErrorResult/elapsedMicros/)\
  The total time for the MySQL error condition to occur, in microseconds
- [` ->endTime(): float `](/docs/apis/Classes/AsyncMysqlErrorResult/endTime/)\
  The end time when the error was produced, in seconds since epoch
- [` ->failureType(): string `](/docs/apis/Classes/AsyncMysqlErrorResult/failureType/)\
  The type of failure that produced this result
- [` ->mysql_errno(): int `](/docs/apis/Classes/AsyncMysqlErrorResult/mysql_errno/)\
  Returns the MySQL error number for this result
- [` ->mysql_error(): string `](/docs/apis/Classes/AsyncMysqlErrorResult/mysql_error/)\
  Returns a human-readable string for the error encountered in this result
- [` ->mysql_normalize_error(): string `](/docs/apis/Classes/AsyncMysqlErrorResult/mysql_normalize_error/)\
  Returns an alternative, normalized version of the error message provided by
  mysql_error()
- [` ->startTime(): float `](/docs/apis/Classes/AsyncMysqlErrorResult/startTime/)\
  The start time when the error was produced, in seconds since epoch







### Public Methods ([` AsyncMysqlResult `](/docs/apis/Classes/AsyncMysqlResult/))




+ [` ->getSslCertCn(): string `](/docs/apis/Classes/AsyncMysqlResult/getSslCertCn/)\
  Returns Common Name attribute of the TLS certificate presented
  by MySQL server
+ [` ->getSslCertExtensions(): Vector<string> `](/docs/apis/Classes/AsyncMysqlResult/getSslCertExtensions/)\
  Returns values from the selected cert extensions of the TLS certificate
  presented by MySQL server
+ [` ->getSslCertSan(): Vector<string> `](/docs/apis/Classes/AsyncMysqlResult/getSslCertSan/)\
  Returns Server Alternative Names attribute of the TLS certificate
  presented by MySQL server
+ [` ->isSslCertValidationEnforced(): bool `](/docs/apis/Classes/AsyncMysqlResult/isSslCertValidationEnforced/)\
  Returns a boolean value indicating if server cert validation was enforced
  for this connection
+ [` ->sslSessionReused(): bool `](/docs/apis/Classes/AsyncMysqlResult/sslSessionReused/)\
  Returns whether or not the current connection reused the SSL session
  from another SSL connection
<!-- HHAPIDOC -->
