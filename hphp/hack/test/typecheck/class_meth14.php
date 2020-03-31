<?hh

// Trait is not final
trait Whoops {
  public static function bar(): void {
    class_meth(self::class, 'bar');
  }
}
