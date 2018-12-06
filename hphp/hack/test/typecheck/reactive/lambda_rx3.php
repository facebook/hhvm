<?hh

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
    print(1);
    return 1;
  });
}

<<__RxShallow>>
function s(): void {
  // OK - lambda is shallow
  g(() ==> l());
}
