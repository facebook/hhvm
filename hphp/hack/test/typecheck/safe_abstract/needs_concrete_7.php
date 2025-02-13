<?hh

abstract class A {
  public static function nc(): void {}
  public abstract static function abs(): void;
}

abstract class B extends A {
  // It would be unsound for a <<__NeedsConcrete>> method to override
  // a non-<<__NeedsConcrete>> method
  // (its requirements are more specific)
  <<__NeedsConcrete>> // error
  public static function nc(): void {
    self::abs();
  }
  public static function abs(): void {}
}

function example(classname<A> $klass): void {
  $klass::nc(); // HHVM error
}

<<__EntryPoint>>
function main(): void {
  example(B::class);
}
//
