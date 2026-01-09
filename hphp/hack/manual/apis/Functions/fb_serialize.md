
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Serialize data into a compact format that can be unserialized by
fb_unserialize()




``` Hack
function fb_serialize(
  mixed $thing,
  int $options = 0,
): mixed;
```




## Parameters




+ ` mixed $thing ` - What to serialize. Note that objects are not
  supported.
+ ` int $options = 0 `




## Returns




* ` mixed ` - - Serialized data.
<!-- HHAPIDOC -->
