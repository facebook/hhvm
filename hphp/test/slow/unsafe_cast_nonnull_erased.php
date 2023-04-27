<?hh

function f(?int $i): int {
  $j = HH\FIXME\UNSAFE_NONNULL_CAST($i); // compiles to $j = $i;
  return $j; // TypeHintViolation
}

<<__EntryPoint>>
function main(): void {
  f(null);
}
