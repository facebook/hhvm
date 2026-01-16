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

// C2 is *not* concrete even though it is `abstract final`.
// C2 has an unimplemented member (the constructor)
abstract final class C2 extends C1 {
  const int X = 3;
  <<__NeedsConcrete>>
  public static function m(): void {
    static::nc(); // error
  }

  public static function abs(): void {}
}

function foo(): void {
  C2::m(); // error
}
