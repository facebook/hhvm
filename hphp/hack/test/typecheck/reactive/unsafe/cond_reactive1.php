<?hh // strict

<<__Rx, __AtMostRxAsArgs>>
function f(<<__AtMostRxAsFunc>>(function(): int) $f): int {
  return $f();
}

<<__Rx>>
function g(): int {
  return f(() ==> 1);
}
