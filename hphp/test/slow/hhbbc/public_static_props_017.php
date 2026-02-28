<?hh

class X {
  public static $y = dict['a' => 17, 'b' => 34];

  static function go() :mixed{
    unset(self::$y['a']);
  }
  static function y() :mixed{
    return self::$y;
  }
}


<<__EntryPoint>>
function main_public_static_props_017() :mixed{
X::go();
var_dump(X::y());
}
