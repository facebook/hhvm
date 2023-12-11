<?hh

class A {
  private static $x = dict[];
  private static $y = "string";

  public static function heh(int $i) :mixed{
    self::$x[$i] = $i;
  }
  public static function getY() :mixed{ return self::$y; }
  public static function getX() :mixed{ return self::$x; }
}

<<__EntryPoint>>
function main() :mixed{
  var_dump(A::heh(0));
  var_dump(A::getY());
  var_dump(A::getX());
}
