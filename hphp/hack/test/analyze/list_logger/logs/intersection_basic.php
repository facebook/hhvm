<?hh

function test_intersection<T>(T $x): void {
  if ($x is Container<_>) {
    list($a, $b) = $x;
  }
}
