<?hh

function f(?int $i, dict<int, string> $d): void {
  $d[HH\FIXME\UNSAFE_NONNULL_CAST($i)] = "hello";
}
