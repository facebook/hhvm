
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Since this is a failed result wrapper, this always returns the exception
thrown during the [` Awaitable `](/apis/Classes/HH/Awaitable/) operation




``` Hack
public function getException(): Te;
```




[` getException() `](/apis/Classes/HH.Asio/WrappedException/getException/) is the same as `getResult() in this case.




## Returns




+ ` Te ` - The exception thrown during the [` Awaitable `](/apis/Classes/HH/Awaitable/) operation.
<!-- HHAPIDOC -->
