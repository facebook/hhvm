
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Serialize data into a compact format that can be unserialized by
fb_compact_unserialize()




``` Hack
function fb_compact_serialize(
  mixed $thing,
  int $options = 0,
): mixed;
```




In general produces smaller output compared to
fb_serialize(). Largest savings are on arrays with sequential (or almost
sequential) indexes, i.e. simple arrays like array($a, $b, $c). NOTE:
unlike serialize(), does not preserve internal references, i.e. array(&$a,
&$a) will become array($a, $a).




## Parameters




+ ` mixed $thing ` - What to serialize. Note that objects are not
  supported.
+ ` int $options = 0 `




## Returns




* ` mixed ` - - Serialized data.
<!-- HHAPIDOC -->
