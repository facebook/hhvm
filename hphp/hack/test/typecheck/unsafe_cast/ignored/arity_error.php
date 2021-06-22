<?hh

function f(): int {
  return \HH_FIXME\UNSAFE_CAST<string, int>();
}

function g(mixed $m1, mixed $m2): int {
  return \HH_FIXME\UNSAFE_CAST<mixed, int>($m1, $m2);
}
