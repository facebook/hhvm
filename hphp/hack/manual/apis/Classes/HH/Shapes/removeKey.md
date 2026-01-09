
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Removes the $index field from the $shape (passed in as an inout argument)




``` Hack
public static function removeKey<T as shape()>(
  inout darray $shape,
  arraykey $index,
): void;
```




As with all inout arguments, it can only be used with local variables.




## Parameters




+ ` inout darray $shape `
+ ` arraykey $index `




## Returns




* ` void `




## Examples




This example shows that ` removeKey ` directly removes a key from a `` Shape ``:




``` basic-usage.hack
function run(shape('x' => int, 'y' => int) $point): void {
  // Prints the value at key 'y'
  \var_dump($point['y']);

  Shapes::removeKey(inout $point, 'y');

  // Prints NULL because the key 'y' doesn't exist any more
  /* HH_IGNORE_ERROR[4251] typechecker knows the key doesn't exist */
  \var_dump(Shapes::idx($point, 'y'));
}

<<__EntryPoint>>
function basic_usage_main(): void {
  run(shape('x' => 3, 'y' => -1));
}
```
<!-- HHAPIDOC -->
