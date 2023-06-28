<?hh

interface I {}

interface I2 extends I {}
interface I3 extends I {}

class A implements I2 {}

class B {
  public function get<
    <<__Enforceable>> reify Ret as I,
  >(): Ret {
    return new A();
  }
}

<<__EntryPoint>> function main() :mixed{
  $x = new B();
  var_dump($x->get<I3>());
}
