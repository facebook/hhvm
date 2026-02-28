<?hh

abstract class C1 {
  abstract const int X;
  <<__NeedsConcrete>>
  public static function nc(): void {}

  public static abstract function abs(): void;

}

// C2 is concrete because it is final+non-__ConsistentConstruct
abstract final class C2 extends C1 {
  const int X = 3;
  public static function m(): void {
    // all the following are ok because C2 is concrete
    static::nc();
    static::abs();
    static::abs<>;
    static::X;
  }

  public static function abs(): void {}
}
