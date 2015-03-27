<?hh

class Foo<-T> {
  public function __construct(T $x) {}
}

class A {}
class B extends A {}

function f(Foo<A> $x) {}

function g() {
  f(new Foo(new B()));
}
