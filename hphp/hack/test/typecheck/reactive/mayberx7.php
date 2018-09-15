<?hh // strict

// OK
<<__Rx, __OnlyRxIfArgs>>
function mayberx(<<__OnlyRxIfRxFunc>>(function(): int) $a): int {
  return $a() + 1;
}

<<__Rx>>
function rx(): int {
  return mayberx(<<__Rx>> () ==> {
    return 1;
  });
}
