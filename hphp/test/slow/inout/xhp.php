<?hh

class :x {
  public static function foo(inout $x) :mixed{
    $x++;
  }
}


<<__EntryPoint>>
function main_xhp() :mixed{
$x = 42;
:x::foo(inout $x);

var_dump($x);
}
