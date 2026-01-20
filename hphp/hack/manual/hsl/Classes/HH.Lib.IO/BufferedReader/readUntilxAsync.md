
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Read until the suffix, or raise EPIPE if the separator is not seen




``` Hack
public function readUntilxAsync(
  string $suffix,
): Awaitable<string>;
```




This is similar to [` readUntilAsync() `](/hsl/Classes/HH.Lib.IO/BufferedReader/readUntilAsync/), however it raises EPIPE instead
of returning null.




## Parameters




+ ` string $suffix `




## Returns




* [` Awaitable<string> `](/apis/Classes/HH/Awaitable/)
<!-- HHAPIDOC -->
