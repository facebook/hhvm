
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Read until the specified suffix is seen




``` Hack
public function readUntilAsync(
  string $suffix,
): Awaitable<?string>;
```




The trailing suffix is read (so won't be returned by other calls), but is not
included in the return value.




This call returns null if the suffix is not seen, even if there is other
data.




## Parameters




+ ` string $suffix `




## Returns




* [` Awaitable<?string> `](/apis/Classes/HH/Awaitable/)
<!-- HHAPIDOC -->
