<?hh // partial

class Foo<-T> {
  public function __construct(T $x) {}
}

class A {}
class B extends A {}

function fa(Foo<A> $x) {}
function fb(Foo<B> $y) {}

function g() {
  // $x has type B and therefore also type A
  $x = new B();
  // $y can be assigned type Foo<A> because B <: A
  $y = new Foo($x);
  // So this is actually legal
  fa($y);
  // By contravariance we can also pass it to something expecting Foo<B>
  fb($y);
}
