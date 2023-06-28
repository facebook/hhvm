<?hh

class Asd {
  public static $heh = Map { 'a' => 'b' };

  function other() :mixed{ self::$heh = new stdClass; }

  function foo() :mixed{
    var_dump(self::$heh);
    return self::$heh;
  }
}


<<__EntryPoint>>
function main_public_static_props_007() :mixed{
(new Asd)->foo();
}
