<?hh

class Asd {
  static $heh = Map { 'a' => 'b' };

  function other() { self::$heh = new stdclass; }

  function foo() {
    var_dump(self::$heh);
    return self::$heh;
  }
}

(new Asd)->foo();
