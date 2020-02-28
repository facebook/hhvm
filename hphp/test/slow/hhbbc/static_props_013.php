<?hh

class A {
  private static $x = varray[];
  private static $y = "string";

  public static function heh(int $i) {
    self::$x[$i] = $i;
  }
  public static function getY() { return self::$y; }
  public static function getX() { return self::$x; }
}

<<__EntryPoint>>
function main() {
  var_dump(A::heh(0));
  var_dump(A::getY());
  var_dump(A::getX());
}
