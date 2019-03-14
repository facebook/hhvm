<?hh // strict

function f<T>(T $value): vec<T> {
  return vec<T>[$value, $value, null];
}
