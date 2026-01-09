
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

``` Hack
public static function toArray(
  shape() $shape,
): darray<arraykey, mixed>;
```




## Parameters




+ ` shape() $shape `




## Returns




* ` darray<arraykey, mixed> `




## Examples




This example shows that ` toArray ` will return the underlying array of a `` Shape ``. The result will be loosely typed because a single ``` Shape ``` can contain arbitrary different types (e.g. ```` string ````, ````` int `````, `````` float ``````).




``` basic-usage.hack
$point = shape('name' => 'Jane Doe', 'age' => 55, 'points' => 25.30);
\var_dump(Shapes::toArray($point));
```
<!-- HHAPIDOC -->
