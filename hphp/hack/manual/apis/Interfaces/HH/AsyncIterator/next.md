
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Move the async iterator to the next [` Awaitable `](/apis/Classes/HH/Awaitable/) position




``` Hack
public function next(): Awaitable<?(mixed, Tv)>;
```




```
foreach ($async_iter await $async_iter->next() $value)
```




The above is the longhand syntax for ` await as $value `.




## Returns




+ [` Awaitable<?(mixed, `](/apis/Classes/HH/Awaitable/)`` Tv)> `` - The next [` Awaitable `](/apis/Classes/HH/Awaitable/) in the iterator sequence.
<!-- HHAPIDOC -->
