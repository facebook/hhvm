<?hh
<<file:__EnableUnstableFeatures('shape_and_tuple_destructuring')>>

class Foo { public int $prop = 0; }

function test_readonly_shape(readonly shape('a' => Foo, 'b' => Foo) $s): void {
  shape('a' => $a, 'b' => $b) = $s;
  $a->prop = 4; // should error: $a is readonly
}

function test_readonly_tuple(readonly (Foo, Foo) $t): void {
  tuple($a, $b) = $t;
  $a->prop = 4; // should error: $a is readonly
}
