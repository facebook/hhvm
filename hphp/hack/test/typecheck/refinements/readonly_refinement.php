<?hh

class C {
  public static ?dict<string, mixed> $prop;

  public static function f(): void {
    if (readonly self::$prop !== null) {
      self::$prop['key'];
    }
  }
}
