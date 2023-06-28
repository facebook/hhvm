<?hh

class A {
  static public $bp = "hello\n";
}
trait T {
  function foo() :mixed{
    echo A::$bp;
  }
}
class C {
 use T;
 }

<<__EntryPoint>>
function main_2122() :mixed{
$o = new C;
$o->foo();
}
