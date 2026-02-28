
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Unserialize previously fb_serialize()-ed data




``` Hack
function fb_unserialize(
  mixed $thing,
  inout mixed $success,
  int $options = 0,
): mixed;
```




## Parameters




+ ` mixed $thing ` - What to unserialize.
+ ` inout mixed $success ` - Whether it was successful or not.
+ ` int $options = 0 `




## Returns




* ` mixed ` - - Unserialized data.
<!-- HHAPIDOC -->
