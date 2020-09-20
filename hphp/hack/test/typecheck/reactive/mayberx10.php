<?hh // strict

<<__RxLocal, __AtMostRxAsArgs>>
function mayberx1(<<__AtMostRxAsFunc>>(function(): int) $a): int {
  return $a() + 1;
}

<<__RxLocal, __AtMostRxAsArgs>>
function mayberx2(<<__AtMostRxAsFunc>>(function(): int) $a): int {
  // OK to pass mayberx parameter through
  return mayberx1($a);
}
