<?hh

class Foob {
  private static $heh = 0;
  private static $ok = "string";

  public static function fgh(&$ref, string $x) {
    $ref = $x;
    return self::$heh;
  }

  public static function asd(string $x) {
    return self::fgh(&self::$heh, $x);
  }
  public static function ok() { return self::$ok; }
}

function heh() {
  return Foob::ok();
}


<<__EntryPoint>>
function main_static_props_006() {
var_dump(Foob::asd('asd'));
var_dump(Foob::asd('asd'));
var_dump(Foob::asd('asd'));
var_dump(Foob::ok());
var_dump(heh());
}
