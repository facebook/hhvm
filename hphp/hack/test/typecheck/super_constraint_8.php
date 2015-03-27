<?hh

class Foo<+T> {
  public function bar<Tu super T>(Tu $x): void {}
}

class A {}
class B extends A {}

function f(Foo<A> $x) {
  $x->bar(new B());
}
