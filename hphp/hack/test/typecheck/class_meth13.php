<?hh

// No error because class is final
final class Foo {
  public static function bar(): void {
    self::bar<>;
  }
}

// Error because the class is not final and could be extended
class Whoops {
  public static function bar(): void {
    self::bar<>;
  }
}
