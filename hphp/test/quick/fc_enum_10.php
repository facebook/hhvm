<?hh // strict

enum Foo : int as int {
  FOO = 1;
  BAR = 2;
  BAZ = 3;
}

class C extends Foo {}
class D extends C {}

var_dump(C::FOO);
var_dump(D::BAR);
