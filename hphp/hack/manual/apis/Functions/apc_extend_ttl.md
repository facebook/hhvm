
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Extend the TTL of a key in APC to now + a new ttl (or infinite)




``` Hack
function apc_extend_ttl(
  string $key,
  int $new_ttl,
): bool;
```




If the
effective expiration of the key is longer than this, returns FALSE and
does nothing.




## Parameters




+ ` string $key ` - The key to bump. If the key is not stored in APC, then
  FALSE is returned.
+ ` int $new_ttl ` - The new TTL for the key. 0 means infinite TTL.




## Returns




* ` bool ` - - TRUE if the TTL was actually extended, FALSE otherwise.
<!-- HHAPIDOC -->
