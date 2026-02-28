<?hh

trait T1 {
 abstract function foo():mixed;
 }
trait T2 {
 abstract function foo():mixed;
 }
class B {
  function foo() :mixed{
 return "hello\n";
 }
}
class C extends B {
  use T1, T2;
}

<<__EntryPoint>>
function main_2120() :mixed{
$o = new C;
echo $o->foo();
}
