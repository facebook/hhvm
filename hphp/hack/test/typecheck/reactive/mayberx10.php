<?hh // strict

<<__RxLocal, __OnlyRxIfArgs>>
function mayberx1(<<__OnlyRxIfRxFunc>>(function(): int) $a): int {
  return $a() + 1;
}

<<__RxLocal, __OnlyRxIfArgs>>
function mayberx2(<<__OnlyRxIfRxFunc>>(function(): int) $a): int {
  // OK to pass mayberx parameter through
  return mayberx1($a);
}
