<?hh // partial

class C {
  public static function f() {
    return "lol";
  }
  static $x = shape('a' => C::f());
}
