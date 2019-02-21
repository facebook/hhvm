<?hh // partial

class C {
  public static function f() {
    return "lol";
  }
  static $x = darray['a' => C::f()];
}
