<?hh // partial

function test(
  (function(int): bool) $f,
  (function(int, string): bool) $g,
  (function(int, string, num): Container<int>) $h,
  (function(num): bool) $i,
  Container<int> $ci,
  Container<string> $cs,
  KeyedContainer<int, num> $cn,
  bool $b,
): void {
  $unresolved = $b ? $ci : $cn;

  array_map($f, $ci);
  array_map($g, $ci, $cs);
  array_map($h, $ci, $cs, $cn);
  array_map($i, $unresolved);
}
