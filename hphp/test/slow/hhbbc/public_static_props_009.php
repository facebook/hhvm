<?hh

class Asd {
  public static $x = 2;
  public static $y = null;
  public static $z = null;
  function heh() :mixed{ self::$y = "asd"; }
  function foo() :mixed{
    return isset(self::$x);
  }
  function bar() :mixed{
    return isset(self::$y);
  }
  function baz() :mixed{
    return isset(self::$z);
  }
}


<<__EntryPoint>>
function main_public_static_props_009() :mixed{
$y = new Asd();
var_dump($y->foo());
var_dump($y->bar());
var_dump($y->baz());
$y->heh();
var_dump($y->foo());
var_dump($y->bar());
var_dump($y->baz());
}
