<?hh

class A {
  private static $x = null;
  private static $y = "string";

  public static function heh(int $i) {
    self::$x[$i] = $i;
  }
  public static function getY() { return self::$y; }
  public static function getX() { return self::$x; }
}

function main() {
  $a = new A;
  var_dump($a->heh(0));
  var_dump($a->getY());
  var_dump($a->getX());
}
main();
