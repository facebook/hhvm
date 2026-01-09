---
title: AsyncMysqlQueryResult
---


:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

The result of a successfully executed MySQL query




Not only does this class provide timing information about retrieving the
successful result, it provides the actual result information (e.g., result
rows).




You get an ` AsyncMysqlQueryResult ` through calls to
[` AsyncMysqlConnection::query() `](/docs/apis/Classes/AsyncMysqlConnection/query/), `` AsyncMysqlConection::queryf() `` and
[` AsyncMysqlConnection::multiQuery() `](/docs/apis/Classes/AsyncMysqlConnection/multiQuery/)




## Guides




+ [Introduction](</docs/hack/asynchronous-operations/introduction>)
+ [Extensions](</docs/hack/asynchronous-operations/extensions>)







## Interface Synopsis




``` Hack
final class AsyncMysqlQueryResult extends AsyncMysqlResult {...}
```




### Public Methods




* [` ->clientStats(): AsyncMysqlClientStats `](/docs/apis/Classes/AsyncMysqlQueryResult/clientStats/)\
  Returns the MySQL client statistics at the moment the successful query
  ended

* [` ->dictRowsTyped(): vec<dict<string, mixed, arraykey>> `](/docs/apis/Classes/AsyncMysqlQueryResult/dictRowsTyped/)

* [` ->elapsedMicros(): int `](/docs/apis/Classes/AsyncMysqlQueryResult/elapsedMicros/)\
  The total time for the successful query to occur, in microseconds

* [` ->endTime(): float `](/docs/apis/Classes/AsyncMysqlQueryResult/endTime/)\
  The end time when the successful query began, in seconds since epoch

* [` ->lastInsertId(): int `](/docs/apis/Classes/AsyncMysqlQueryResult/lastInsertId/)\
  The last ID inserted, if one existed, for the query that produced the
  current result

* [` ->mapRows(): Vector<Map<?string>> `](/docs/apis/Classes/AsyncMysqlQueryResult/mapRows/)\
  Returns the actual rows returned by the successful query, each row
  including the name and value for each column

* [` ->mapRowsTyped(): Vector<Map<string, mixed>> `](/docs/apis/Classes/AsyncMysqlQueryResult/mapRowsTyped/)\
  Returns the actual rows returned by the successful query, each row
  including the name and typed-value for each column

* [` ->noIndexUsed(): bool `](/docs/apis/Classes/AsyncMysqlQueryResult/noIndexUsed/)\
  Returns whether or not any of the queries executed did not use an index
  during execution

* [` ->numRows(): int `](/docs/apis/Classes/AsyncMysqlQueryResult/numRows/)\
  The number of rows in the current result

* [` ->numRowsAffected(): int `](/docs/apis/Classes/AsyncMysqlQueryResult/numRowsAffected/)\
  The number of database rows affected in the current result

* [` ->recvGtid(): string `](/docs/apis/Classes/AsyncMysqlQueryResult/recvGtid/)\
  The GTID of database returned for the current commit

* [` ->responseAttributes(): Map<string> `](/docs/apis/Classes/AsyncMysqlQueryResult/responseAttributes/)\
  The response attributes returned for the current query

* [` ->rowBlocks(): Vector<AsyncMysqlRowBlock> `](/docs/apis/Classes/AsyncMysqlQueryResult/rowBlocks/)\
  Returns a [` Vector `](/docs/apis/Classes/HH/Vector/) representing all row blocks returned by the successful
  query

* [` ->startTime(): float `](/docs/apis/Classes/AsyncMysqlQueryResult/startTime/)\
  The start time when the successful query began, in seconds since epoch

* [` ->vectorRows(): Vector<KeyedContainer<int, ?string>> `](/docs/apis/Classes/AsyncMysqlQueryResult/vectorRows/)\
  Returns the actual rows returned by the successful query, each row
  including the values for each column

* [` ->vectorRowsTyped(): Vector<KeyedContainer<int, mixed>> `](/docs/apis/Classes/AsyncMysqlQueryResult/vectorRowsTyped/)\
  Returns the actual rows returned by the successful query, each row
  including the typed values for each column








### Public Methods ([` AsyncMysqlResult `](/docs/apis/Classes/AsyncMysqlResult/))




- [` ->getSslCertCn(): string `](/docs/apis/Classes/AsyncMysqlResult/getSslCertCn/)\
  Returns Common Name attribute of the TLS certificate presented
  by MySQL server
- [` ->getSslCertExtensions(): Vector<string> `](/docs/apis/Classes/AsyncMysqlResult/getSslCertExtensions/)\
  Returns values from the selected cert extensions of the TLS certificate
  presented by MySQL server
- [` ->getSslCertSan(): Vector<string> `](/docs/apis/Classes/AsyncMysqlResult/getSslCertSan/)\
  Returns Server Alternative Names attribute of the TLS certificate
  presented by MySQL server
- [` ->isSslCertValidationEnforced(): bool `](/docs/apis/Classes/AsyncMysqlResult/isSslCertValidationEnforced/)\
  Returns a boolean value indicating if server cert validation was enforced
  for this connection
- [` ->sslSessionReused(): bool `](/docs/apis/Classes/AsyncMysqlResult/sslSessionReused/)\
  Returns whether or not the current connection reused the SSL session
  from another SSL connection
<!-- HHAPIDOC -->
