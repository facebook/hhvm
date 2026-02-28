<?hh
interface I {
  public function foo($x):mixed;
}
abstract class B implements I {
  abstract public function foo($x):mixed;
}
class C extends B {
  public function foo($x):mixed{ echo "$x\n";}
}
<<__EntryPoint>> function main(): void {
$obj = new C;
$obj->foo(1);
}
