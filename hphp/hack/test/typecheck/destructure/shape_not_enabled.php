<?hh

function f(shape('a' => int) $x): void {
  shape('a' => $a) = $x;
  tuple($x, tuple (shape('a' => $a), $y), shape('a' => $b)) =
    tuple(1, tuple($x, 2), $x);
  tuple($x, $y) = tuple(1, 2);
}
