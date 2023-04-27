<?hh

class A {
  public function foo() { return 1; }
}
abstract class B extends A {}
abstract class C extends A {}

<<__EntryPoint>>
function main() {
  echo "Done\n";
}
