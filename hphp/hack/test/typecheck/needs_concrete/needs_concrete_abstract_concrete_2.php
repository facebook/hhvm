<?hh

<<__ConsistentConstruct>>
abstract class C1 {
  abstract const int X;
  <<__NeedsConcrete>>
  public static function nc(): void {
    new static();
  }

  public static abstract function abs(): void;

}



abstract final class C2 extends C1 {
  const int X = 3;
  public static function m(): void {
    new static(); // pre-existing 4002 error and no redundant warning
    static::nc(); // we generate a warning
  }

  public static function abs(): void {}
}
