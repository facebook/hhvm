<?hh // partial

<<__Rx, __AtMostRxAsArgs>>
function g(<<__AtMostRxAsFunc>> (function(): int) $a): int {
  return $a();
}

<<__Rx>>
function f(): void {
  // OK - lambda is rx
  g(() ==> 1);
}

<<__RxLocal>>
function l(): int {
  // OK - lambda is local
  return g(() ==> {
    nonrx();
    return 1;
  });
}

function nonrx(): void {
}

<<__RxShallow>>
function s(): void {
  // OK - lambda is shallow
  g(() ==> l());
}
