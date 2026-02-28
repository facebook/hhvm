<?hh

interface I {
  public function foo($x):mixed;
}
abstract class B implements I {
  abstract public function foo($x):mixed;
}
class C extends B {
  public function foo($x):mixed{
 echo "$x \n";
}
}

<<__EntryPoint>>
function main_744() :mixed{
$obj = new C;
$obj->foo(1);
}
