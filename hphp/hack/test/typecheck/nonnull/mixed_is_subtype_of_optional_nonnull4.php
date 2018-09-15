<?hh // strict

function get_default<T as nonnull>(?T $val, T $default): T {
  return $val ?? $default;
}

function f(Map<string, mixed> $x): nonnull {
  return get_default($x->get('foo'), 42);
}
