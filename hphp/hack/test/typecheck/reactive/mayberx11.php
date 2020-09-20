<?hh // strict

<<__RxLocal>>
function mayberx1(Rx<(function(): int)> $a): int {
  return $a() + 1;
}

<<__RxLocal, __AtMostRxAsArgs>>
function mayberx2(<<__AtMostRxAsFunc>>(function(): int) $a): int {
  // Error: cannot pass mayberx parameter as rx value
  return mayberx1($a);
}
