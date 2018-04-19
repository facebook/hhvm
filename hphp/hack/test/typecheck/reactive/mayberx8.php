<?hh // strict

<<__Rx, __OnlyRxIfArgs>>
function mayberx(<<__OnlyRxIfRxFunc>>(function(): int) $a): int {
  return $a() + 1;
}

<<__Rx>>
function rx(): int {
  // ERROR
  return mayberx(() ==> {
    print 1;
    return 1;
  });
}
