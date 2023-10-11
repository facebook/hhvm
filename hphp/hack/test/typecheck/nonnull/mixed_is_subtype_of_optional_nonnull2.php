<?hh

function get_default<T as nonnull>(?T $val, T $default): T {
  return $val ?? $default;
}

function f(mixed $x): nonnull {
  return get_default($x, 42);
}
