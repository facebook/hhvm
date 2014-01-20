<?hh

class monomorphic_dynamic {
  private static $x = 0;
  private static $y = 1;
  private static $z = 2;

  public static function heh(int $val, string $name) {
    self::${$name} = $val;
  }

  public static function get() {
    var_dump(is_int(self::$x));
    var_dump(is_int(self::$y));
    var_dump(is_int(self::$z));
    return self::$x;
  }
}

function main() {
  monomorphic_dynamic::get();
  monomorphic_dynamic::heh(3, 'x');
  monomorphic_dynamic::heh(3, 'z');
  monomorphic_dynamic::heh(3, 'y');
  monomorphic_dynamic::get();
}

main();
