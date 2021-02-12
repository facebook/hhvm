<?hh // strict
interface IRxMyParent {

  public function foo(): Foo;
}

class Foo {}

abstract class MyParent {

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

  public function foo(): Foo {
    return new Foo();
  }
}
