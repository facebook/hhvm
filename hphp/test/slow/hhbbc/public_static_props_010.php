<?hh

class Foo {
  static $x = 0;
  static $y = 0;

  function x() {
    self::$x++;
    self::$y += 12;
  }

  function go() {
    $this->x();
    var_dump(self::$x);
    var_dump(self::$y);
  }
}


<<__EntryPoint>>
function main_public_static_props_010() {
(new Foo)->go();
}
