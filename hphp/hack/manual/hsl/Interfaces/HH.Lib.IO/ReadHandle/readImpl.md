
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

An immediate, unordered read




``` Hack
public function readImpl(
  ?int $max_bytes = NULL,
): string;
```




## Parameters




+ ` ?int $max_bytes = NULL ` the maximum number of bytes to read
+ if ` null `, an internal default will be used.
+ if 0, ` EINVAL ` will be raised.
+ up to ` $max_bytes ` may be allocated in a buffer; large values may lead
  to unnecessarily hitting the request memory limit.




## Returns




* ` string `
<!-- HHAPIDOC -->
