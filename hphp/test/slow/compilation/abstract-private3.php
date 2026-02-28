<?hh

abstract class C1 {
  static private function f1() :mixed{ return 1; }
}
abstract class C2 extends C1 {
  static public function f1() :mixed{ return 2; }
}
abstract class C3 extends C1 {}

<<__EntryPoint>>
function main() :mixed{
  var_dump(C2::f1());
}
