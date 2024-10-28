<?hh

function f(shape('a' => int) $x): void {
  shape('a' => $a) = $x;
  list($x, list (shape('a' => $a), $y), shape('a' => $b)) =
    tuple(1, tuple($x, 2), $x);
}
