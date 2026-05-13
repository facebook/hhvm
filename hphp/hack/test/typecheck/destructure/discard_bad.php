<?hh
<<file:__EnableUnstableFeatures('shape_and_tuple_destructuring', 'shape_field_punning')>>

// $_ in destructuring is a discard -- reading it gives void
function test_punning_discard(shape('_' => int) $s): void {
  shape($_) = $s;
  $_ + 1; // $_ is a placeholder variable not meant to be used
}

function test_explicit_discard(shape('x' => int) $s): void {
  shape('x' => $_) = $s;
  $_ + 1; // $_ is a placeholder variable not meant to be used
}
