<?hh

class C {
  public static function f(): string {
    return "lol";
  }
  static darray<string, string> $x = darray['a' => C::f()];
}
