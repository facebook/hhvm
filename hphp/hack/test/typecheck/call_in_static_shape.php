<?hh

class C {
  public static function f(): string {
    return "lol";
  }
  public static shape('a' => string) $x = shape('a' => C::f());
}
