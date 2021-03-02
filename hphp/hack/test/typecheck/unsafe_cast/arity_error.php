<?hh

function f(): int {
  return unsafe_cast<string, int>();
}

function g(mixed $m1, mixed $m2): int {
  return unsafe_cast<mixed, int>($m1, $m2);
}
