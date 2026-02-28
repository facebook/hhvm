<?hh
function f(?vec<int> $v): int {
  return $v[0] ?? 0;
}
