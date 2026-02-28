<?hh

class Foob {
  private static $heh = 0;

  public static function asd() :mixed{
    self::$heh += 1;
    return self::$heh;
  }
}

<<__EntryPoint>>
function main_static_props_004() :mixed{
var_dump(Foob::asd());
var_dump(Foob::asd());
}
