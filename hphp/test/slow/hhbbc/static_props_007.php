<?hh

class Foob {
  private static $heh = 0;
  private static $ok = "string";

  public static function asd(string $x) {
    self::$heh =& $x;
    return self::$heh;
  }
  public static function ok() { return self::$ok; }

  public static function breaker($x) {
    self::${$x} =& $x;
  }
}

function heh() {
  return Foob::ok();
}

var_dump(Foob::asd('asd'));
var_dump(Foob::asd('asd'));
var_dump(Foob::asd('asd'));
var_dump(heh());
var_dump(Foob::ok());
var_dump(Foob::breaker('ok'));
var_dump(Foob::ok());
var_dump(heh());
