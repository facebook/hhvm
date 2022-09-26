<?hh

function f(?int $i, dict<int, string> $d): void {
  $d[HH\FIXME\UNSAFE_NONNULL_CAST($i)] = "hello";
}

<<__EntryPoint>>
function main(): void {
  f(4, dict[]);
  f(null, dict[]); // invalid array key
}
