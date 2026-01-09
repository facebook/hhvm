
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns the value of the field $index of a readonly $shape,
throws if the field is missing




``` Hack
public static function at(
  shape() $shape,
  arraykey $index,
): mixed;
```




Use this to access optional fields on readonly shapes.




## Parameters




+ ` shape() $shape `
+ ` arraykey $index `




## Returns




* ` mixed `
<!-- HHAPIDOC -->
