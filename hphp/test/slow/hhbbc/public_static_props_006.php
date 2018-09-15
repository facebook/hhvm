<?hh

class Asd {
  static $heh = Map { 'a' => 'b' };

  function foo() {
    var_dump(self::$heh);
    return self::$heh;
  }
}


<<__EntryPoint>>
function main_public_static_props_006() {
(new Asd)->foo();
}
