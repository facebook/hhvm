<?hh // strict

trait Tr {

  public abstract static function f(): void;

  protected function g(): void {
    self::f();
  }
}
