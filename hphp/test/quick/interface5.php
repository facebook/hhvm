<?hh
interface I { function foo(); }
abstract class B implements I {}
abstract class C extends B {}
class D extends C {
  public function foo() {
    echo "Done\n";
  }
}
<<__EntryPoint>> function main(): void {
$obj = new D;
$obj->foo();
}
