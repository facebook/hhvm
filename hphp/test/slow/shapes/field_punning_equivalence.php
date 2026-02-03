<?hh
// Verify that punned syntax produces identical results to explicit syntax

<<file:__EnableUnstableFeatures('shape_field_punning')>>

<<__EntryPoint>>
function main(): void {
  $foo = 42;
  $bar = 'hello';
  $baz = true;

  // These two shapes should be identical
  $punned = shape($foo, $bar, $baz);
  $explicit = shape('foo' => $foo, 'bar' => $bar, 'baz' => $baz);

  echo "Punned and explicit are equal: ";
  var_dump($punned === $explicit);

  // Mixed forms also produce correct results
  $mixed1 = shape($foo, 'bar' => $bar, $baz);
  $mixed2 = shape('foo' => $foo, $bar, 'baz' => $baz);

  echo "Mixed form 1 equals explicit: ";
  var_dump($mixed1 === $explicit);

  echo "Mixed form 2 equals explicit: ";
  var_dump($mixed2 === $explicit);

  // Verify key access works the same way
  echo "Keys match:\n";
  echo "  foo: ";
  var_dump($punned['foo'] === $explicit['foo']);
  echo "  bar: ";
  var_dump($punned['bar'] === $explicit['bar']);
  echo "  baz: ";
  var_dump($punned['baz'] === $explicit['baz']);
}
