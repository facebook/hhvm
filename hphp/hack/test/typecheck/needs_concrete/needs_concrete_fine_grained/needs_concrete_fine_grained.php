<?hh

function classCallCheck(): void {
  Base::concreteOnly();
}

<<__ConsistentConstruct>>
abstract class Base {
  public static abstract function abstractMethod(): void;

  <<__NeedsConcrete>>
  public static function concreteOnly(): void {}

  public static function definitionChecks(): void {
    static::abstractMethod();
    new static();
  }

}

abstract class Child extends Base {
  public static function forwardingCallChecks(): void {
    self::concreteOnly();
    parent::concreteOnly();
    static::concreteOnly();
  }
}
