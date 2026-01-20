
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Retrieve the next pending connection as a disposable




``` Hack
public function nextConnectionAsync(): Awaitable<TSock>;
```




Will wait for new connections if none are pending.




## Returns




+ [` Awaitable<TSock> `](/apis/Classes/HH/Awaitable/)
<!-- HHAPIDOC -->
