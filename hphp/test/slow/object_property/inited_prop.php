<?hh


class Thing {
  private static $thingsArray;

  public static function doStuff($k, $v) {
    self::ensureInit();
    var_dump(self::$thingsArray);
    self::$thingsArray[$k] = $v;
    var_dump(self::$thingsArray);
  }

  private static function ensureInit() {
    if (self::$thingsArray !== null) {
      return;
    }
    self::$thingsArray = array();
    $box = &self::$thingsArray;
    $box[3] = 4;
  }
}

function main() {
  Thing::doStuff(11, 22);
}

main();
