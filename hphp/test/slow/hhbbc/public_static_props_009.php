<?hh

class Asd {
  static $x = 2;
  static $y = null;
  static $z = null;
  function heh() { self::$y = "asd"; }
  function foo() {
    return isset(self::$x);
  }
  function bar() {
    return isset(self::$y);
  }
  function baz() {
    return isset(self::$z);
  }
}


<<__EntryPoint>>
function main_public_static_props_009() {
$y = new Asd();
var_dump($y->foo());
var_dump($y->bar());
var_dump($y->baz());
$y->heh();
var_dump($y->foo());
var_dump($y->bar());
var_dump($y->baz());
}
