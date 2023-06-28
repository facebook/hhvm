<?hh
interface I { function foo():mixed; }
abstract class B implements I {}
abstract class C extends B {}
class D extends C {
  public function foo() :mixed{
    echo "Done\n";
  }
}
<<__EntryPoint>> function main(): void {
$obj = new D;
$obj->foo();
}
