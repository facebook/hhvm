<?hh // strict

interface A {}
interface RxA {}

class C1 {
  <<__Rx>>
  public function f(Rx<(function(): int)> $a): void {
  }
}

class C2 {
  // OK (though condition is kind of useless)
  <<__Rx, __OnlyRxIfArgs>>
  public function f(<<__OnlyRxIfRxFunc>>(function(): int) $a): void {
  }
}
