
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns the MySQL client statistics at the moment the result was created




``` Hack
abstract public function clientStats(): AsyncMysqlClientStats;
```




This information can be used to know how the performance of the MySQL
client may have affected the result.




## Returns




+ [` AsyncMysqlClientStats `](/apis/Classes/AsyncMysqlClientStats/) - an [` AsyncMysqlClientStats `](/apis/Classes/AsyncMysqlClientStats/) object to query about event and
  callback timing to the MySQL client for the specific result.




## Examples




AsyncMysqlResult is abstract. See specific, concrete classes for examples of [` AsyncMysqlResult::clientStats `](/apis/Classes/AsyncMysqlResult/clientStats/) (e.g., AsyncMysqlConnectResult, AsyncMysqlErrorResult)




~~~ basic-usage.hack
echo "AsyncMysqlResult is abstract. See specific, concrete classes for ".
  "examples of clientStats (e.g., AsyncMysqlConnectResult, ".
  "AsyncMysqlErrorResult)".
  \PHP_EOL;
```.hhvm.expectf
AsyncMysqlResult is abstract. See specific, concrete classes for examples of clientStats (e.g., AsyncMysqlConnectResult, AsyncMysqlErrorResult)
```.example.hhvm.out
AsyncMysqlResult is abstract. See specific, concrete classes for examples of clientStats (e.g., AsyncMysqlConnectResult, AsyncMysqlErrorResult)
```.skipif
await \Hack\UserDocumentation\API\Examples\AsyncMysql\skipif_async();
~~~
<!-- HHAPIDOC -->
