<?hh

function get_default<T as nonnull>(?T $val, T $default): T {
  return $val ?? $default;
}

function wrap<T>(T $x): ?T {
  return $x;
}

function f(mixed $x): nonnull {
  return get_default(wrap(wrap($x)), 42);
}
