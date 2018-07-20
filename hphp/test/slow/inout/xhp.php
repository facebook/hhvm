<?hh // strict

class :x {
  public static function foo(inout $x) {
    $x++;
  }
}

$x = 42;
:x::foo(&$x);

var_dump($x);
