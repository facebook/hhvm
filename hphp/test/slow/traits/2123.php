<?hh

class B {
  static public $bp = "B::hello\n";
}
trait T {
  function foo() {
    echo self::$bp;
  }
}
class C extends B {
  static public $bp = "C::hello\n";
  use T;
}

<<__EntryPoint>>
function main_2123() {
$o = new C;
$o->foo();
}
