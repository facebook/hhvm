<?hh

function f0(?int $x): int {
  $x ??= 0;
  return $x;
}

function f1(?int $x): int {
  $x ??= 0;
  $y = 42;
  $y ??= vec["hello", 4];
  return $x;
}

function f2(?int $x): nonnull {
  $x ??= 0;
  $x = 42;
  $x ??= vec["hello", 4];
  return $x;
}

function f3(?int $x): int {
  $x ??= 0;
  $x = 42;
  $x ??= vec["hello", 4];
  return $x;
}
