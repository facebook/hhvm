
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Return the result of the operation, or throw underlying exception




``` Hack
public function getResult(): T;
```




+ if the operation succeeded: return its result
+ if the operation failed: throw the exception incating failure




## Returns




* ` T ` - the result of the [` Awaitable `](/apis/Classes/HH/Awaitable/) operation upon success or the
  exception that was thrown upon failed.
<!-- HHAPIDOC -->
