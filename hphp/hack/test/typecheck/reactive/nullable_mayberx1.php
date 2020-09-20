<?hh // strict

<<__Rx, __AtMostRxAsArgs>>
function f(<<__AtMostRxAsFunc>>?(function(): int) $f): void {
}

<<__Rx>>
function g1(): void {
  // OK
  f(null);
}
