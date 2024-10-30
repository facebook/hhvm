<?hh

function f(mixed $m): void {
  list(shape('a' => $a), $b) = $m;
}
