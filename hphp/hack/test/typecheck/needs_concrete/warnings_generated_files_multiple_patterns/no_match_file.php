<?hh
// Test with multiple patterns: this file does NOT match any pattern

abstract class NoMatchClass {
  public static function m1(): void {
    // Warning expected because this file doesn't match any pattern
    static::abs();
  }
  public static abstract function abs(): void;
}
