<?hh

class C {
  public static function f() {
    return "lol";
  }
  public static $x = vec[C::f()];
}
