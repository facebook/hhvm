
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

``` Hack
function thrift_protocol_write_binary(
  object $transportobj,
  string $method_name,
  int $msgtype,
  object $request_struct,
  int $seqid,
  bool $strict_write,
  bool $oneway = false,
): void;
```




## Parameters




+ ` object $transportobj `
+ ` string $method_name `
+ ` int $msgtype `
+ ` object $request_struct `
+ ` int $seqid `
+ ` bool $strict_write `
+ ` bool $oneway = false `




## Returns




* ` void `
<!-- HHAPIDOC -->
