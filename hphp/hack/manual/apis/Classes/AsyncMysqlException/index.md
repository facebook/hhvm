---
title: AsyncMysqlException
---


:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

The base exception class for any issues arising from the use of async
MySQL




In general, when trying to connect to a MySQL client (e.g., via
[` AsyncMysqlConnectionPool::connect() `](/apis/Classes/AsyncMysqlConnectionPool/connect/)) or when making a query (e.g., via
[` AsyncMysqlConnection::queryf() `](/apis/Classes/AsyncMysqlConnection/queryf/)), it is good practice to have your code
exception catchable somewhere in the code pipeline (via
[try/catch](<http://php.net/manual/en/language.exceptions.php>)).




e.g.,




```
try {
  // code here
} catch (AsyncMysqlException $ex) {
  $error = $ex->mysqlErrorString();
}
```




## Guides




+ [Intro](</hack/asynchronous-operations/introduction>)
+ [Extensions](</hack/asynchronous-operations/extensions>)







## Interface Synopsis




``` Hack
class AsyncMysqlException extends Exception {...}
```




### Public Methods




* [` ->__construct(AsyncMysqlErrorResult $result) `](/apis/Classes/AsyncMysqlException/__construct/)\
  Explicitly construct an `` AsyncMysqlException ``
* [` ->failed(): bool `](/apis/Classes/AsyncMysqlException/failed/)\
  Returns whether the type of failure that produced the exception was a
  general connection or query failure
* [` ->getResult(): AsyncMysqlErrorResult `](/apis/Classes/AsyncMysqlException/getResult/)\
  Returns the underlying [` AsyncMysqlErrorResult `](/apis/Classes/AsyncMysqlErrorResult/) associated with the current
  exception
* [` ->mysqlErrorCode(): int `](/apis/Classes/AsyncMysqlException/mysqlErrorCode/)\
  Returns the MySQL error number for that caused the current exception
* [` ->mysqlErrorString(): string `](/apis/Classes/AsyncMysqlException/mysqlErrorString/)\
  Returns a human-readable string for the error encountered in the current
  exception
* [` ->timedOut(): bool `](/apis/Classes/AsyncMysqlException/timedOut/)\
  Returns whether the type of failure that produced the exception was a
  timeout
<!-- HHAPIDOC -->
