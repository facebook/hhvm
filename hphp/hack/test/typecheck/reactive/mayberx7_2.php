<?hh // strict

<<__Rx, __AtMostRxAsArgs>>
function mayberx(<<__AtMostRxAsFunc>>(function(): int) $a): int {
  return $a() + 1;
}

<<__Rx>>
function rx(): int {
  // error, non-rx lambda
  return mayberx(<<__NonRx>>function() {
    return 1;
  });
}
