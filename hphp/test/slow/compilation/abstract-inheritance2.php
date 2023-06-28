<?hh

abstract class C1 {
  public function f1() :mixed{ return 1; }
}
abstract class C2 extends C1 {
  public function f1() :mixed{ return 2; }
}
class C3 extends C1 {}

function func1(): C1 {
  return __hhvm_intrinsics\launder_value(new C3());
}

<<__EntryPoint>>
function main() :mixed{
  $x = func1();
  var_dump($x->f1());
}
