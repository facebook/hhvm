<?hh

function test(
  (function(int): bool) $f,
  Container<int> $x,
  Container<string> $y,
): void {
  array_map($f, $x, $y);
}
