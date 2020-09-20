<?hh // strict

// OK
<<__Rx, __AtMostRxAsArgs>>
function mayberx(<<__AtMostRxAsFunc>>(function(): int) $a): int {
  return $a() + 1;
}

<<__Rx>>
function rx(): int {
  return mayberx(<<__Rx>> () ==> {
    return 1;
  });
}
