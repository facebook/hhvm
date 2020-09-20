<?hh // strict

class :x {
  public static function foo(inout $x) {
    $x++;
  }
}


<<__EntryPoint>>
function main_xhp() {
$x = 42;
:x::foo(inout $x);

var_dump($x);
}
