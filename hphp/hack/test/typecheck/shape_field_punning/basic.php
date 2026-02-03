<?hh
<<file:__EnableUnstableFeatures('shape_field_punning')>>

function basic_punning(): void {
  $foo = 1;
  $bar = 'hello';
  // Basic shorthand syntax
  $s = shape($foo, $bar);
  hh_expect<shape('foo' => int, 'bar' => string)>($s);
}

function mixed_syntax(): void {
  $field1 = 1;
  // Mix shorthand and explicit syntax
  $s = shape($field1, 'other' => 42, 'third' => 'hello');
  hh_expect<shape('field1' => int, 'other' => int, 'third' => string)>($s);
}

function empty_shape_still_works(): void {
  $s = shape();
  hh_expect<shape()>($s);
}

function explicit_syntax_still_works(): void {
  $x = 1;
  // Explicit syntax should still work
  $s = shape('x' => $x, 'y' => 2);
  hh_expect<shape('x' => int, 'y' => int)>($s);
}
