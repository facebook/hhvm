<?hh

class Foob {
  private static $heh = 0;
  private static $ok = "string";

  public static function asd(string $x) {
    self::$heh =& $x;
    return self::$heh;
  }
  public static function ok() { return self::$ok; }
}

function heh() {
  return Foob::ok();
}

var_dump(Foob::asd('asd'));
var_dump(Foob::asd('asd'));
var_dump(Foob::asd('asd'));
var_dump(Foob::ok());
var_dump(heh());
