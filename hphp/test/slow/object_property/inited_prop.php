<?hh


class Thing {
  private static $thingsArray;

  public static function doStuff($k, $v) :mixed{
    self::ensureInit();
    var_dump(self::$thingsArray);
    self::$thingsArray[$k] = $v;
    var_dump(self::$thingsArray);
  }

  private static function ensureInit() :mixed{
    if (self::$thingsArray !== null) {
      return;
    }
    self::$thingsArray = dict[];
    self::$thingsArray[3] = 4;
  }
}

function main() :mixed{
  Thing::doStuff(11, 22);
}


<<__EntryPoint>>
function main_inited_prop() :mixed{
main();
}
