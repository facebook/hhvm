<?hh

trait T1 {
 abstract function foo():mixed;
 }
trait T2 {
 abstract function foo():mixed;
 }
abstract class B {
  use T1, T2;
}
class C extends B {
  function foo() :mixed{
 return "hello\n";
 }
}

<<__EntryPoint>>
function main_2121() :mixed{
$o = new C;
echo $o->foo();
}
