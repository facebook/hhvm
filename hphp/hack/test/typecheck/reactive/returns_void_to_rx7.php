<?hh // partial

interface Rx {
  <<__Rx>>
  public function foo(): this;
}

abstract class MyParent {
  // OK since it cannot be called in non-rx code
  public function foo(): this {
    return $this;
  }
}

class MyChild extends MyParent {
  <<__Rx, __OnlyRxIfImpl(Rx::class), __Mutable, __ReturnsVoidToRx>>
  public function foo(): this {
    return $this;
  }
}
