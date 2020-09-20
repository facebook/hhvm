<?hh // strict

<<__RxLocal, __AtMostRxAsArgs>>
function mayberx(<<__AtMostRxAsFunc>>(function(): int) $a): int {
  return $a() + 1;
}

<<__Rx>>
function rx(): int {
  // ERROR: lambda is reactive which makes mayberx locally reactive 
  return mayberx(() ==> {
    return 1;
  });
}
