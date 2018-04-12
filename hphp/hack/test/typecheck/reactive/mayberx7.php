<?hh // strict

// OK
<<__Rx, __OnlyRxIfArgs>>
function mayberx(<<__MaybeRx>>(function(): int) $a): int {
  return $a() + 1;
}

<<__Rx>>
function rx(): int {
  return mayberx(() ==> {
    return 1;
  });
}
