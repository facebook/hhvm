<?hh

trait T {
  abstract public function foo($x);
}
interface I {
  public function foo($x);
}
abstract class B implements I {
  use T;
}
class C extends B {
  public function foo($x){
 echo "$x \n";
}
}

<<__EntryPoint>>
function main_2108() {
$obj = new C;
$obj->foo(1);
}
