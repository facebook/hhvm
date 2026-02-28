<?hh

class MyReifiedGenericClass<<<__Enforceable>> reify T> {}

// T223013561
abstract class Foo {
  <<__Enforceable, __Reifiable>>
  abstract const type TInner;
  const type TOuter = MyReifiedGenericClass<this::TInner>;

  public static abstract function takeOuter(this::TOuter $outer): void;

  public static function test(this::TOuter $outer): void {
    if (
      $outer is MyReifiedGenericClass<int> ||
      $outer is MyReifiedGenericClass<string>
    ) {
    }
    static::takeOuter($outer);
  }
}
