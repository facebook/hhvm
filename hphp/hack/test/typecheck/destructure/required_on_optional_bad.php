<?hh
<<file:__EnableUnstableFeatures('shape_and_tuple_destructuring')>>

// Using a required pattern field on an optional shape field is a type error.
// Use ?'key' for optional fields instead.

function test_required_pattern_optional_rhs_field(
  shape('x' => int, ?'y' => int) $s,
): void {
  // Error: 'y' is optional in the type but required in the pattern
  shape('x' => $x, 'y' => $y, ...) = $s;
}

function test_required_pattern_optional_rhs_nested(
  shape('a' => shape(?'b' => string)) $s,
): void {
  // Error: 'b' is optional in the inner shape but required in the pattern
  shape('a' => shape('b' => $b, ...)) = $s;
}
