<?hh // strict

interface A {
  <<__Rx>>
  public function f((function(): int) $f): int;
}

interface B extends A {
  // ERROR, will allow to run non-reactive code in rx context
  // class CB implements B {
  //   <<__Rx, __OnlyRxIfArgs>>
  //   public function f(<<__OnlyRxIfRxFunc>> (function(): int) $f): int {
  //     return $f();
  //   }
  // }

  <<__Rx, __OnlyRxIfArgs>>
  public function f(<<__OnlyRxIfRxFunc>>(function(): int) $f): int;
}
