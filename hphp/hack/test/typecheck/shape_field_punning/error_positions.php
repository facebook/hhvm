<?hh
<<file:__EnableUnstableFeatures('shape_field_punning')>>

function test_error_on_second_occurrence(): void {
  $dup = 1;
  // The error should point to the second occurrence
  $s = shape($dup, $dup);
}

function test_error_on_explicit_after_punned(): void {
  $field = 1;
  // Error should point to 'field' => 2, not $field
  $s = shape($field, 'field' => 2);
}

function test_type_error_with_punning(): void {
  $val = 'string';
  $s = shape($val);
  // Type error should reference correct position
  hh_expect<shape('val' => int)>($s);
}

function test_error_on_punned_after_explicit(): void {
  $name = 'value';
  // Error should point to $name (second occurrence)
  $s = shape('name' => 'other', $name);
}
