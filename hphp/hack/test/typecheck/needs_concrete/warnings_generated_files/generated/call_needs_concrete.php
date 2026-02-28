<?hh
// Test that Call_needs_concrete warning is also suppressed for generated files

abstract class GeneratedCallNeedsConcrete {
  <<__NeedsConcrete>>
  public static function needs_concrete_method(): void {}

  public static function caller(): void {
    // No warning expected because this file matches /generated/ pattern
    // This would normally trigger Call_needs_concrete warning
    static::needs_concrete_method();
  }
}
