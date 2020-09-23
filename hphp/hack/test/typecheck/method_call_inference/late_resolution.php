<?hh

class Box<T> {
  public function get() : T {
    throw new Exception();
  }
}

class Inv<T> {
  public function __construct(public T $item){}
}

class A {
  public function foo() : bool {
    return true;
  }
}

class B {}

class C extends B {
  public function foo() : void {}
}

function test1() : Inv<A> {
  $box = new Box();
  $unresolved = $box->get();
  $unresolved->foo();
  return new Inv($unresolved); // OK - A implements method foo
}

function test2() : Inv<B> {
  $box = new Box();
  $unresolved = $box->get();
  $unresolved->foo();
  return new Inv($unresolved);
  // OK - $unresolved type can subtype B and implement method foo, e.g. C
  // e.g. function bar(C $c) : Inv<B> { return new Inv<$c>; } is OK
}

function test3(C $c) : Inv<B> {
  $inv_c = new Inv($c); // $inv_c : Inv<#1>, C <: #1
  $inv_c->item->foo(); // #1 <: has-member(foo, ...)
  return $inv_c; // Inv<#1> <: Inv<B> i.e. #1 = B
  // Error, B does not implement method foo
}

function test4(C $c) : Inv<C> {
  $inv_c = new Inv($c); // $inv_c : Inv<#1>, C <: #1
  $inv_c->item->foo(); // #1 <: has-member(foo, ...)
  return $inv_c; // Inv<#1> <: Inv<C> i.e. #1 = C
  // OK, C implements method foo
}
