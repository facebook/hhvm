<?hh // strict

interface A {
  <<__RxLocal, __OnlyRxIfArgs>>
  public function f(<<__OnlyRxIfRxFunc>>(function(): void) $a): int;
}

interface B extends A {
  // OK
  <<__Rx, __OnlyRxIfArgs>>
  public function f(<<__OnlyRxIfRxFunc>>(function(): void) $a): int;
}
