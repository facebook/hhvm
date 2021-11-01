<?hh

class C {
  public static function f(): string {
    return "lol";
  }
  static varray<string> $x = varray[C::f()];
}
