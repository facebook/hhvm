<?hh

class X {
  public static $y = vec[];

  static function z() :mixed{
    self::$y[] = 2;
    return self::$y;
  }
}


<<__EntryPoint>>
function main_public_static_props_016() :mixed{
var_dump(X::z());
var_dump(X::z());
}
