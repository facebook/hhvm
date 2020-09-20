<?hh // partial

<<__Rx, __AtMostRxAsArgs>>
function f(<<__AtMostRxAsFunc>>(function(): int) $f): int {
  return $f();
}

<<__Rx>>
function a(): int {
  // ERROR, cannot call rx shallow
  return f(<<__RxShallow>> () ==> 1);
}
