<?hh

class ConcreteClass {
  use BaseTrait;

  public static function m(): void {}

  public static function callBase(): void {
    static::baseMethod();
  }
}
