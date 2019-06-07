<?hh

class X {
  static $x = null;
  function a() {
    self::$x = new stdClass();
    self::$x->foo = 2;
    return self::$x;
  }
}


<<__EntryPoint>>
function main_public_static_props_014() {
var_dump(X::$x);
var_dump((new X)->a());
var_dump(X::$x);
}
