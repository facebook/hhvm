<?hh

abstract class C1 {
  static private function f1() { return 1; }
}
abstract class C2 extends C1 {
  static public function f1() { return 2; }
}
abstract class C3 extends C1 {}

<<__EntryPoint>>
function main() {
  var_dump(C2::f1());
}
