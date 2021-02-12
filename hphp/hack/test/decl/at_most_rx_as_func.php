<?hh

<<__Pure, __AtMostRxAsArgs>>
function f<T>(
  <<__AtMostRxAsFunc>> (function(T): bool) $g,
  T $x,
): bool {
  return $g($x);
}
