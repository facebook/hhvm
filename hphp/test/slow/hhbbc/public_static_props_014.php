<?hh

class X {
  public static $x = null;
  function a() :mixed{
    self::$x = new stdClass();
    self::$x->foo = 2;
    return self::$x;
  }
}


<<__EntryPoint>>
function main_public_static_props_014() :mixed{
var_dump(X::$x);
var_dump((new X)->a());
var_dump(X::$x);
}
