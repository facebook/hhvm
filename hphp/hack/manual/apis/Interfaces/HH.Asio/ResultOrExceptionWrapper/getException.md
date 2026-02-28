
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Return the underlying exception, or fail with invariant violation




``` Hack
public function getException(): Exception;
```




+ if the operation succeeded: fails with invariant violation
+ if the operation failed: returns the exception indicating failure




## Returns




* ` Exception ` - The exception that the [` Awaitable `](/apis/Classes/HH/Awaitable/) threw upon failure, or an
  [` InvariantException `](/apis/Classes/HH/InvariantException/) if there is no exception (i.e., because
  the [` Awaitable `](/apis/Classes/HH/Awaitable/) succeeded).
<!-- HHAPIDOC -->
