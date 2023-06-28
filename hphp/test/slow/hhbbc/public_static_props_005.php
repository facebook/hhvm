<?hh

class Foo {
  public static $heh = varray[];
  function go() :mixed{
    self::$heh = varray[self::$heh];
  }
}


<<__EntryPoint>>
function main_public_static_props_005() :mixed{
(new Foo)->go();
var_dump(Foo::$heh);
}
