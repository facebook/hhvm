<?hh

abstract class Abs {
  <<__NeedsConcrete>>
  public static function nc(): void {
    static::abs(); // ok
  }
  public abstract static function abs(): void;
}

<<__EntryPoint>>
function main(): void {
  // cannot call a <<__NeedsConcrete>> method
  // through a class that may be abstract
  Abs::nc(); // error
}
