
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Pure variant of unserialize




``` Hack
function unserialize_pure(
  string $str,
  darray<string, mixed> $options = dict [
],
): mixed;
```




Unserializing objects with impure implementations of the ` __wakeup ` method
will result in coeffect violations.




## Parameters




+ ` string $str `
+ ` darray<string, mixed> $options = dict [ ] `




## Returns




* ` mixed `
<!-- HHAPIDOC -->
