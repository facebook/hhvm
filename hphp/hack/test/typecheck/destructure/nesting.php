<?hh

function f(): void {
  list(shape('a' => $a), $b) = tuple(shape('a' => 1), 2);
  shape('a' => list($a, $b)) = shape('a' => tuple(1, 2));
  list(tuple($a), $b) = tuple(tuple(1), 2);
  tuple(list($a, $b)) = tuple(tuple(1, 2));


}
