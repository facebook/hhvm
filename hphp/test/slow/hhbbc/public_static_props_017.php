<?hh

class X {
  static $y = varray[1,2,3];

  static function go() {
    unset(self::$y[0]);
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
