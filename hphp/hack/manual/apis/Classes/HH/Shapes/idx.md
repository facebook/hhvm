
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Use [` Shapes::idx `](/apis/Classes/HH/Shapes/idx/) to retrieve a field value in a shape, when the key may or may not exist




``` Hack
public static function idx(
  shape(...) $shape,
  arraykey $index,
  mixed $default = NULL,
);
```




If ` $index ` does not exist in the shape, the default value will be returned (`` $default ``), if one has been set.
It behaves similarily to [` idx() `](/apis/Classes/HH/Shapes/idx/) for Collections.




A few examples:

+ [` Shapes::idx(shape('x' `](/apis/Classes/HH/Shapes/idx/)`` => 123), 'x') // 123 ``

+ [` Shapes::idx(shape('x' `](/apis/Classes/HH/Shapes/idx/)`` => 123), 'y') // null ``

+ [` Shapes::idx(shape('x' `](/apis/Classes/HH/Shapes/idx/)`` => 123), 'y', 456) // 456 ``

+ [` Shapes::idx(null, `](/apis/Classes/HH/Shapes/idx/)`` 'y', 456) // 456 ``





Use [` Shapes::idx `](/apis/Classes/HH/Shapes/idx/) when the key in your shape is optional (e.g., `` ?x ``, in ``` shape(?'x' => int ```).
If the key in your shape is always present, access the value directly: ```` $my_shape['x'] ````.




The second argument, ` $index ` must always be a literal.




## Parameters




* ` shape(...) $shape ` - shape to search for $index.
* ` arraykey $index ` - Key ($index) to search. Must be a literal!
* ` mixed $default = NULL ` - Default value to return if $index does not exist. By default, returns `` null ``.




## Returns




- ` $value ` - - Value at $index, if it exists, or $default.




## Examples




This example shows how to use [` Shapes::idx `](/apis/Classes/HH/Shapes/idx/) for keys that may or may not exist in a `` Shape ``:




``` basic-usage.hack
function run(shape('x' => int, 'y' => int, ?'z' => int) $point): void {
  // The key 'x' exists in the Shape $point so it's returned
  \var_dump(Shapes::idx($point, 'x'));

  // The key 'z' doesn't exist in $point so the default NULL is returned
  \var_dump(Shapes::idx($point, 'z'));

  // The key 'z' doesn't exist in $point so our explicit default 0 is returned
  \var_dump(Shapes::idx($point, 'z', 0));
}

<<__EntryPoint>>
function basic_usage_main(): void {
  run(shape('x' => 3, 'y' => -1));
}
```




This example shows that [` Shapes::idx `](/apis/Classes/HH/Shapes/idx/) will only return the default value if the key doesn't exist in the `` Shape ``. If the key exists but is ``` NULL ``` then ```` NULL ```` will be returned.




``` nullable-values.hack
function runNullable(shape('x' => ?int, 'y' => ?int, ...) $point): void {
  // The key 'x' exists, so its value (3) is returned, not our explicit default 0
  \var_dump(Shapes::idx($point, 'x', 0));

  // The key 'y' exists, so its value (NULL) is returned, not our explicit default 0
  \var_dump(Shapes::idx($point, 'y', 0));
}

<<__EntryPoint>>
function runNullableMain(): void {
  runNullable(shape('x' => 3, 'y' => null));
}
```
<!-- HHAPIDOC -->
