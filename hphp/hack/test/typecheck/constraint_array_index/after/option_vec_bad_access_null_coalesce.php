<?hh
function f(?vec<int> $v): int {
  return $v["a"] ?? 0;
}
