<?hh

abstract class C {

  public abstract static function f(): void;

  protected function g(): void {
    self::f();
  }
}
