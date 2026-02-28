<?hh

trait T {
  abstract public function foo($x):mixed;
}
interface I {
  public function foo($x):mixed;
}
abstract class B implements I {
  use T;
}
class C extends B {
  public function foo($x):mixed{
 echo "$x \n";
}
}

<<__EntryPoint>>
function main_2108() :mixed{
$obj = new C;
$obj->foo(1);
}
