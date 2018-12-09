<?hh // strict

<<__Rx, __AtMostRxAsArgs>>
function mayberx(<<__AtMostRxAsFunc>>(function(): int) $a): int {
  return $a() + 1;
}

<<__Rx>>
function rx(): int {
  // ERROR
  return mayberx(<<__NonRx>>() ==> {
    print 1;
    return 1;
  });
}
