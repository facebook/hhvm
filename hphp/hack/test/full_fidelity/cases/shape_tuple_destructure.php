<?hh
<<file:__EnableUnstableFeatures('shape_and_tuple_destructuring', 'shape_field_punning')>>

function f(): void {
  // shape with ellipsis
  shape('x' => $x, ...) = $s;
  shape(...) = $s;

  // tuple with ellipsis
  tuple($a, ...) = $t;
  tuple(...) = $t;

  // optional field with ? prefix
  shape('x' => $x, ?'y' => $y) = $s;
  shape(?$y) = $s;

  // basic shape and tuple (no ellipsis)
  shape('x' => $x) = $s;
  tuple($a, $b) = $t;
}
