<?hh
<<file:__EnableUnstableFeatures('shape_and_tuple_destructuring')>>

// Duplicate in same shape --should error
function test_dup_shape(shape('x' => int, 'y' => int) $s): void {
  shape('x' => $a, 'y' => $a) = $s;
}

// Duplicate in same tuple --should error
function test_dup_tuple((int, int) $t): void {
  tuple($a, $a) = $t;
}

// Duplicate across nesting -- should error
function test_dup_nested(shape('x' => int, 'y' => (int, int)) $s): void {
  shape('x' => $a, 'y' => tuple($a, $b)) = $s;
}

// Duplicate $_ -- wildcard should be used instead
function test_dup_dollar_underscore(shape('a' => int, 'b' => int) $s): void {
  shape('a' => $_, 'b' => $_) = $s;
}
