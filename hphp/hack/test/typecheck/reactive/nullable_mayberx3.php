<?hh // strict

<<__Rx, __OnlyRxIfArgs>>
function f(<<__OnlyRxIfRxFunc>>?(function(): int) $f): void {
}

<<__Rx>>
function g1(): void {
  // OK
  f(() ==> {
    return 1;
  });
}
