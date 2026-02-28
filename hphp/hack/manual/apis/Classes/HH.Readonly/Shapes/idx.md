
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Use [` Readonly\Shapes::idx `](/apis/Classes/HH.Readonly/Shapes/idx/) to retrieve a field value in a shape, when the key may or may not exist




``` Hack
public static function idx(
  shape(...) $shape,
  arraykey $index,
  mixed $default = NULL,
): mixed;
```




If ` $index ` does not exist in the shape, the default value will be returned (`` $default ``), if one has been set.
It behaves similarily to [` idx() `](/apis/Classes/HH.Readonly/Shapes/idx/) for Collections.




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
<!-- HHAPIDOC -->
