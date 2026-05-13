<?hh
<<file:__EnableUnstableFeatures('shape_and_tuple_destructuring')>>

class Foo {
  public int $prop = 0;
}

// Nested readonly: readonly propagates through nesting
function test_readonly_nested(
  readonly shape('inner' => shape('x' => Foo)) $s,
): void {
  shape('inner' => shape('x' => $x)) = $s;
  $x->prop = 4; // error: $x is readonly (propagated through nesting)
}

// Readonly tuple containing shape
function test_readonly_tuple_shape(
  readonly (shape('x' => Foo), Foo) $t,
): void {
  tuple(shape('x' => $x), $y) = $t;
  $x->prop = 4; // error: $x is readonly
  $y->prop = 4; // error: $y is readonly
}
