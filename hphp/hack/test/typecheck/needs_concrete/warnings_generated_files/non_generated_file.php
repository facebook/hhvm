<?hh
// This file is NOT in a "generated" directory, so warnings should appear

abstract class NonGenerated {
  public static function m1(): void {
    // Warning expected: static::abs() when static might not be concrete
    static::abs();
  }
  public static abstract function abs(): void;
}
