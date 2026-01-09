
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

( excerpt* http://php.net/manual/en/reflectionclass.getproperties.php )




``` Hack
public function getProperties(
  int $filter = 65535,
): varray<ReflectionProperty>;
```




Retrieves reflected properties.




## Parameters




+ ` int $filter = 65535 `




## Returns




* ` mixed ` - An array of ReflectionProperty objects.
<!-- HHAPIDOC -->
