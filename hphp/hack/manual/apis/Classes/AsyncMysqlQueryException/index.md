---
title: AsyncMysqlQueryException
---


:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

The exception associated with a MySQL query failure




All methods are the same as the base [` AsyncMysqlException `](/apis/Classes/AsyncMysqlException/).




## Interface Synopsis




``` Hack
class AsyncMysqlQueryException extends AsyncMysqlException {...}
```




### Public Methods ([` AsyncMysqlException `](/apis/Classes/AsyncMysqlException/))




+ [` ->__construct(AsyncMysqlErrorResult $result) `](/apis/Classes/AsyncMysqlException/__construct/)\
  Explicitly construct an [` AsyncMysqlException `](/apis/Classes/AsyncMysqlException/)
+ [` ->failed(): bool `](/apis/Classes/AsyncMysqlException/failed/)\
  Returns whether the type of failure that produced the exception was a
  general connection or query failure
+ [` ->getResult(): AsyncMysqlErrorResult `](/apis/Classes/AsyncMysqlException/getResult/)\
  Returns the underlying [` AsyncMysqlErrorResult `](/apis/Classes/AsyncMysqlErrorResult/) associated with the current
  exception
+ [` ->mysqlErrorCode(): int `](/apis/Classes/AsyncMysqlException/mysqlErrorCode/)\
  Returns the MySQL error number for that caused the current exception
+ [` ->mysqlErrorString(): string `](/apis/Classes/AsyncMysqlException/mysqlErrorString/)\
  Returns a human-readable string for the error encountered in the current
  exception
+ [` ->timedOut(): bool `](/apis/Classes/AsyncMysqlException/timedOut/)\
  Returns whether the type of failure that produced the exception was a
  timeout
<!-- HHAPIDOC -->
