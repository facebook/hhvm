<?hh

class Foo {
  public static $x = 0;
  public static $y = 0;

  function x() :mixed{
    self::$x++;
    self::$y += 12;
  }

  function go() :mixed{
    $this->x();
    var_dump(self::$x);
    var_dump(self::$y);
  }
}


<<__EntryPoint>>
function main_public_static_props_010() :mixed{
(new Foo)->go();
}
