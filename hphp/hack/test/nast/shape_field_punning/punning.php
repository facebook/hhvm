<?hh
<<file:__EnableUnstableFeatures('shape_field_punning')>>

function test_punning(): void {
  $foo = 1;
  $bar = 'hello';
  $s = shape($foo, $bar);
}

function test_mixed(): void {
  $field1 = 1;
  $s = shape($field1, 'other' => 42);
}

function test_trailing_comma(): void {
  $x = 1;
  $y = 2;
  $s = shape($x, $y,);
}

function test_single_field(): void {
  $only_field = 'value';
  $s = shape($only_field);
}

function test_multiple_types(): void {
  $int_field = 42;
  $string_field = 'hello';
  $bool_field = true;
  $float_field = 3.14;
  $s = shape($int_field, $string_field, $bool_field, $float_field);
}

function test_punned_at_end(): void {
  $punned = 'last';
  $s = shape('explicit' => 1, $punned);
}

function test_punned_in_middle(): void {
  $middle = 2;
  $s = shape('first' => 1, $middle, 'last' => 3);
}
