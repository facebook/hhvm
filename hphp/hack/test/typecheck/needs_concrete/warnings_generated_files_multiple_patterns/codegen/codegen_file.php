<?hh
// Test with multiple patterns: matches /codegen/ pattern

abstract class CodegenClass {
  public static function m1(): void {
    // No warning expected because this file matches /codegen/ pattern
    static::abs();
  }
  public static abstract function abs(): void;
}
