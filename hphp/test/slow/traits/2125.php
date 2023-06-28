<?hh

class A {
  function foo() :mixed{
 var_dump('fail');
 }
  function bar() :mixed{
 $this->foo();
 }
}
trait BT {
  function foo() :mixed{
 var_dump('ok');
 }
}
class B extends A {
  use BT;
}
function test() :mixed{
  $b = new B;
  $b->bar();
}

<<__EntryPoint>>
function main_2125() :mixed{
test();
}
