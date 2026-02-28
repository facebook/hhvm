
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Wait for all polled [` Awaitable `](/apis/Classes/HH/Awaitable/)s, ignoring the results




``` Hack
public function waitUntilEmptyAsync(): Awaitable<void>;
```




This is a convenience function, for when the [` Awaitable `](/apis/Classes/HH/Awaitable/)'s side effects
are needed instead of the result.




## Returns




+ [` Awaitable<void> `](/apis/Classes/HH/Awaitable/)
<!-- HHAPIDOC -->
