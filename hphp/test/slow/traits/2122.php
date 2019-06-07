<?hh

class A {
  static public $bp = "hello\n";
}
trait T {
  function foo() {
    echo A::$bp;
  }
}
class C {
 use T;
 }

<<__EntryPoint>>
function main_2122() {
$o = new C;
$o->foo();
}
