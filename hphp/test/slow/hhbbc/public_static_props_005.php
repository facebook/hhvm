<?hh

class Foo {
  public static $heh = vec[];
  function go() :mixed{
    self::$heh = vec[self::$heh];
  }
}


<<__EntryPoint>>
function main_public_static_props_005() :mixed{
(new Foo)->go();
var_dump(Foo::$heh);
}
