<?hh

class Asd {
  static $heh = Map { 'a' => 'b' };

  function other() { self::$heh = new stdClass; }

  function foo() {
    var_dump(self::$heh);
    return self::$heh;
  }
}


<<__EntryPoint>>
function main_public_static_props_007() {
(new Asd)->foo();
}
