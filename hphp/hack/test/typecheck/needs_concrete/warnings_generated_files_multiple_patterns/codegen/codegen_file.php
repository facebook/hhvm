<?hh
// The second configured pattern matches this basename.

abstract class CodegenClass {
  public static function m1(): void {
    static::abs();
  }
  public static abstract function abs(): void;
}
