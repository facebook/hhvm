<?hh // strict

interface A {
  <<__RxLocal, __OnlyRxIfArgs>>
  public function f(<<__OnlyRxIfRxFunc>>(function(): void) $a): int;
}

interface B extends A {
  // ERROR since reactivity for parameters should be contravariant
  // and RxLocal is not a subtype of Rx
  <<__Rx, __OnlyRxIfArgs>>
  public function f(<<__OnlyRxIfRxFunc>>(function(): void) $a): int;
}
