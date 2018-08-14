<?hh

<<__Rx, __OnlyRxIfArgs>>
function f(<<__AtMostRxAsFunc>>(function(): int) $f): int {
  // ERROR: canot call rxlocal from rx
  return rxlocal();
}

<<__RxLocal>>
function rxlocal(): int {
  return 1;
}
