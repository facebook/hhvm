
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Starts a namespaced attribute




``` Hack
public function startAttributeNS(
  string $prefix,
  string $name,
  string $uri,
): bool;
```




## Parameters




+ ` string $prefix ` - The namespace prefix.
+ ` string $name ` - The attribute name.
+ ` string $uri ` - The namespace URI.




## Returns




* ` bool ` - - Returns TRUE on success or FALSE on failure.
<!-- HHAPIDOC -->
