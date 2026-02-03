<?hh
<<file:__EnableUnstableFeatures('shape_field_punning')>>

function test_type_mismatch(): void {
  $a = 'string_value';
  // Type mismatch: field 'a' is string but expected int
  $s = shape($a);
  hh_expect<shape('a' => int)>($s);
}

function test_wrong_field_name(): void {
  $wrong = 42;
  // Using punned shape where different field name is expected
  $s = shape($wrong);
  takes_shape_with_correct($s);
}

function takes_shape_with_correct(shape('correct' => int) $s): void {}

function test_missing_field(): void {
  $a = 1;
  // Shape has only 'a' but expected 'a' and 'b'
  $s = shape($a);
  takes_shape_with_two_fields($s);
}

function takes_shape_with_two_fields(shape('a' => int, 'b' => int) $s): void {}

function test_extra_field(): void {
  $a = 1;
  $b = 2;
  // Shape has 'a' and 'b' but expected only 'a' (closed shape)
  $s = shape($a, $b);
  takes_closed_shape($s);
}

function takes_closed_shape(shape('a' => int) $s): void {}
