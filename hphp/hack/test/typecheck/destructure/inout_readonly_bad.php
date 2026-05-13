<?hh
<<file:__EnableUnstableFeatures('shape_and_tuple_destructuring')>>

// Test that destructuring into inout params from readonly values
// is rejected, matching the behavior of direct assignment.

class Foo {
  public int $prop = 0;
}

function test_shape_into_inout(
  inout shape('x' => Foo, 'y' => Foo) $s,
  readonly shape('a' => Foo) $ro,
): void {
  shape('a' => $s) = $ro; // Invalid assignment to an `inout` parameter
}

function test_tuple_into_inout(
  inout (Foo, Foo) $t,
  readonly (Foo,) $ro,
): void {
  tuple($t) = $ro; // Invalid assignment to an `inout` parameter
}

function test_nested_into_inout(
  inout Foo $x,
  readonly shape('inner' => shape('val' => Foo)) $ro,
): void {
  shape('inner' => shape('val' => $x)) = $ro; // Invalid assignment to an `inout` parameter
}
