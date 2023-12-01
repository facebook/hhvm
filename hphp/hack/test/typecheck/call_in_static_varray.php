<?hh

class C {
  public static function f(): string {
    return "lol";
  }
  public static varray<string> $x = vec[C::f()];
}
