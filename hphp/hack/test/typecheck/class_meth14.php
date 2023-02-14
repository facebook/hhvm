<?hh

// Trait is not final
trait Whoops {
  public static function bar(): void {
    self::bar<>;
  }
}
