<?hh

function incr(int $x): int {
  return $x + 1;
}

function f(): nonnull {
  return incr<>;
}

function cast_function((function(int): int) $f): nonnull {
  return $f;
}
