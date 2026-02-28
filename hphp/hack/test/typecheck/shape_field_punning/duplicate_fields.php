<?hh
<<file:__EnableUnstableFeatures('shape_field_punning')>>

function test_duplicate_punned_and_explicit(): void {
  $foo = 1;
  // Error: field 'foo' already bound
  $s = shape('foo' => 2, $foo);
}

function test_duplicate_same_var_twice(): void {
  $bar = 1;
  // Error: field 'bar' already bound
  $s = shape($bar, $bar);
}

function test_duplicate_explicit_then_punned(): void {
  $baz = 1;
  // Error: field 'baz' already bound
  $s = shape($baz, 'baz' => 2);
}

function test_three_way_collision(): void {
  $x = 1;
  // Multiple duplicates
  $s = shape($x, 'x' => 2, $x);
}
