<?hh // partial

<<__Rx, __AtMostRxAsArgs>>
function f(<<__AtMostRxAsFunc>>(function(): int) $f): int {
  return $f();
}

<<__Rx>>
function a(): int {
  return
    // OK to call Rx from Rx
    f(<<__Rx>> () ==> 1) +
    // OK - __RxOfScope is __Rx
    f(<<__RxOfScope>> () ==> 1);
}

<<__RxShallow>>
function b(): int {
  return
    // OK to call Rx from RxShallow
    f(<<__Rx>> () ==> 1) +
    // OK to call RxShallow from RxShallow
    f(<<__RxShallow>> () ==> 1) +
    // OK - __RxOfScope is __RxShallow
    f(<<__RxOfScope>> () ==> 1) +
    // OK to call RxLocal from RxShallow
    f(<<__RxLocal>> () ==> 1);
}

<<__RxLocal>>
function c(): int {
  return
    // OK to call Rx from RxLocal
    f(<<__Rx>> () ==> 1) +
    // OK to call RxShallow from RxLocal
    f(<<__RxShallow>> () ==> 1) +
    // OK to call RxLocal from RxLocal
    f(<<__RxLocal>> () ==> 1);
  // OK to call Nonreactive from RxLocal
  f(() ==> 1) +
    // OK - __RxOfScope is __RxLocal
    f(() ==> 1);
}
