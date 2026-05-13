<?hh
<<file:__EnableUnstableFeatures('shape_and_tuple_destructuring')>>

// Error: missing required field 'y' in closed pattern
function test_missing_field(shape('x' => int, 'y' => int) $s): void {
  shape('x' => $x) = $s;
}

// Error: unknown field 'z' not in shape type
function test_unknown_field(shape('x' => int) $s): void {
  shape('x' => $x, 'z' => $z) = $s;
}

// Error: multiple missing required fields reported in one error
function test_multiple_missing(shape('a' => int, 'b' => int, 'c' => int) $s): void {
  shape('a' => $a) = $s;
}

// Error: tuple arity mismatch
function test_tuple_arity((int, int) $t): void {
  tuple($a) = $t;
}
