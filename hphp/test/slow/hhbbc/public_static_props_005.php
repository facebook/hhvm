<?hh

class Foo {
  static $heh = varray[];
  function go() {
    self::$heh = varray[self::$heh];
  }
}


<<__EntryPoint>>
function main_public_static_props_005() {
(new Foo)->go();
var_dump(Foo::$heh);
}
