<?hh // strict

class A {
  public static function whoami2(): void {
    echo "I am: ", get_called_class(), "/", static::class, "\n";
  }
  public static function whoami(): void {
    // Should dispatch to A::whoami2 but maintain the current "static" class.
    // This is weird.
    self::whoami2();
  }

}
trait Asub {
  require extends A;
  public static function tfun(): void { self::whoami(); }
}
class B extends A {
  use Asub;
  public static function cfun(): void { parent::whoami(); }
  public static function cfun2(): void { static::whoami(); }
  public static function whoami2(): void { echo "wrong!\n"; }
}
class C extends B {}

function test(): void {
  B::tfun();
  B::cfun();
  B::cfun2();

  C::tfun();
  C::cfun();
  C::cfun2();
}
