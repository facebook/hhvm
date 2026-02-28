<?hh

enum class E : int {
  int A = 0;
}

<<__Memoize>>
function f(HH\EnumClass\Label<E, int> $x): int {
  return E::valueOf($x);
}
