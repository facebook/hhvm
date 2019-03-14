<?hh // partial

interface A {}
interface Rx {}

<<__Rx, __AtMostRxAsArgs>>
function f(
  <<__AtMostRxAsFunc>>(function(): int) $f,
  <<__OnlyRxIfImpl(Rx::class)>>A $a,
): int {
  return $f();
}

<<__Rx>>
function a<T>(T $a): int where T as A, T as Rx {
  return
    // OK to call Rx from Rx
    f(<<__Rx>> () ==> 1, $a) +
    // OK - __RxOfScope is __Rx
    f(<<__RxOfScope>> () ==> 1, $a);
}

<<__RxShallow>>
function b<T>(T $a): int where T as A, T as Rx {
  return
    // OK to call Rx from RxShallow
    f(<<__Rx>> () ==> 1, $a) +
    // OK to call RxShallow from RxShallow
    f(<<__RxShallow>> () ==> 1, $a) +
    // OK - __RxOfScope is __RxShallow
    f(<<__RxOfScope>> () ==> 1, $a) +
    // OK to call RxLocal from RxShallow
    f(<<__RxLocal>> () ==> 1, $a);
}

<<__RxLocal>>
function c<T>(T $a): int where T as A, T as Rx {
  return
    // OK to call Rx from RxLocal
    f(<<__Rx>> () ==> 1, $a) +
    // OK to call RxShallow from RxLocal
    f(<<__RxShallow>> () ==> 1, $a) +
    // OK to call RxLocal from RxLocal
    f(<<__RxLocal>> () ==> 1, $a);
  // OK to call Nonreactive from RxLocal
  f(() ==> 1, $a) +
    // OK - __RxOfScope is __RxLocal
    f(() ==> 1, $a);
}
