<?hh

function test(
  (function(int, string): bool) $f,
  Container<int> $x,
  Container<string> $y,
): void {
  hh_show(array_map($f, $x, $y));
}
