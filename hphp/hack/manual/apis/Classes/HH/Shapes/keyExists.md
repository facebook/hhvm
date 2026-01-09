
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Check if a field in shape exists




``` Hack
public static function keyExists(
  shape() $shape,
  arraykey $index,
): bool;
```




Similar to array_key_exists, but for shapes.




## Parameters




+ ` shape() $shape `
+ ` arraykey $index `




## Returns




* ` bool `




## Examples




This example shows that ` keyExists ` returns true if a key exists in the `` Shape `` (even if the corresponding value is ``` NULL ```):




``` basic-usage.hack
function run(shape(?'x' => ?int, ?'y' => ?int, ?'z' => ?int) $point): void {
  // The key 'x' exists in Shape $point
  \var_dump(Shapes::keyExists($point, 'x'));

  // The key 'z' doesn't exist in $point
  \var_dump(Shapes::keyExists($point, 'z'));

  // The key 'y' exists in $point, even though its value is NULL
  \var_dump(Shapes::keyExists($point, 'y'));
}

<<__EntryPoint>>
function basic_usage_main(): void {
  run(shape('x' => 3, 'y' => null));
}
```
<!-- HHAPIDOC -->
