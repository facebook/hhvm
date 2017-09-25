<?hh

class Asd {
  static $heh = Map { 'a' => 'b' };

  function foo() {
    var_dump(self::$heh);
    return self::$heh;
  }
}

(new Asd)->foo();
