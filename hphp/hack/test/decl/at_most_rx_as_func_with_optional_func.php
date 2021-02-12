<?hh

<<__Pure, __AtMostRxAsArgs>>
function f<T>(
  <<__AtMostRxAsFunc>>
  ?(function(T): bool) $predicate = null,
): bool {
  return true;
}
