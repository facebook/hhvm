<?hh // strict

<<__Rx, __OnlyRxIfArgs>>
function f(<<__OnlyRxIfRxFunc>>(function(): int) $f): int {
  return $f();
}

<<__Rx>>
function g(): int {
  return f(() ==> 1);
}
