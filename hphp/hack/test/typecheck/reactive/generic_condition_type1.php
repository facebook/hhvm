<?hh // strict

interface IRxMyParent<T> {
  <<__Rx>>
  public function foo(T $a): void;
}

abstract class MyParent {
  <<__Rx, __OnlyRxIfImpl(IRxMyParent::class)>>
  public function foo(string $s): void {}
}

// OK
class MyChildRx extends MyParent implements IRxMyParent<string> {}
