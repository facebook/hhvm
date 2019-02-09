<?hh

function foo<T>(vec<T> $v): KeyedContainer<int, T> {
  return $v;
}
