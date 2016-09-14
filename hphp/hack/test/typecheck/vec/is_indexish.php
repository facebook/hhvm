<?hh

function foo<T>(vec<T> $v): Indexish<int, T> {
  return $v;
}
