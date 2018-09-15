<?hh // strict

interface A {
  public function f(): int;
}

interface RxA {
  <<__Rx>>
  public function f(): int;
}

<<__Rx, __OnlyRxIfArgs>>
function a(<<__OnlyRxIfImpl(RxA::class)>>A $a): int {
  return b($a);
}

<<__Rx, __OnlyRxIfArgs>>
function b(<<__OnlyRxIfImpl(RxA::class)>>A $a): int {
  // OK - cond reactive function can call cond reactive method
  return $a->f();
}
