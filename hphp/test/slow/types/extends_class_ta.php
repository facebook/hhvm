<?hh

abstract class A {
  static function foo() :mixed{
    return 24;
  }
}

type T1 = A;

type T2 = T1;

class C extends T2 {}


<<__EntryPoint>>
function main_extends_class_ta() :mixed{
  $h = new C;
  var_dump(C::foo());
}
