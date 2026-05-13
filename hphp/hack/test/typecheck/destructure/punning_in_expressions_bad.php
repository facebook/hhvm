<?hh
<<file:__EnableUnstableFeatures('shape_and_tuple_destructuring', 'shape_field_punning')>>

// $_ in expression punning reads void
function test_dollar_underscore_in_expr(): void {
  $_ = 1;
  $s = shape($_);
}
