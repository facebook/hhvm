<?hh // strict

interface IRxMyParent {
  <<__Rx, __MutableReturn>>
  public function foo(): Foo;
}

class Foo {}

abstract class MyParent {
  <<__Rx, __MutableReturn, __OnlyRxIfImpl(IRxMyParent::class)>>
  public function foo(): Foo {
    return new Foo();
  }
}

class MyNonRxChild extends MyParent {
  public function foo(): Foo {
    return new Foo();
  }
}

class MyRxChild extends MyParent {
  <<__Rx, __MutableReturn>>
  public function foo(): Foo {
    return new Foo();
  }
}
