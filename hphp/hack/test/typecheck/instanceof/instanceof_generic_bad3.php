<?hh

function T<T>(mixed $a): bool {
  return $a is T;
}
