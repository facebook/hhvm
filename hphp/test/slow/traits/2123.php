<?hh

class B {
  static public $bp = "B::hello\n";
}
trait T {
  function foo() :mixed{
    echo self::$bp;
  }
}
class C extends B {
  static public $bp = "C::hello\n";
  use T;
}

<<__EntryPoint>>
function main_2123() :mixed{
$o = new C;
$o->foo();
}
