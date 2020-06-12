<?hh

class X {
  static $y = darray['a' => 17, 'b' => 34];

  static function go() {
    unset(self::$y['a']);
  }
  static function y() {
    return self::$y;
  }
}


<<__EntryPoint>>
function main_public_static_props_017() {
X::go();
var_dump(X::y());
}
