<?hh
class Foo {}
class Bar extends Foo {
  public function baz($x)  {
    return 1;
  }
}



interface I {}
interface I2 extends I {}

interface I3 extends I, I2 {}
