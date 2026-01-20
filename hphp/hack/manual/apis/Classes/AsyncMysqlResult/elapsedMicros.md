
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

The total time for the specific MySQL operation, in microseconds




``` Hack
abstract public function elapsedMicros(): int;
```




## Returns




+ ` int ` - the total operation time as `` int `` microseconds.




## Examples




AsyncMysqlResult is abstract. See specific, concrete classes for examples of [` AsyncMysqlResult::elapsedMicros `](/apis/Classes/AsyncMysqlResult/elapsedMicros/) (e.g., AsyncMysqlConnectResult, AsyncMysqlErrorResult)
<!-- HHAPIDOC -->
