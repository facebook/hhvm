<?hh

function take_ints(array<int> $x) {}

function test(
  (function(int, string): bool) $f,
  Container<int> $x,
  Container<string> $y,
): void {
  take_ints(array_map($f, $x, $y));
}
