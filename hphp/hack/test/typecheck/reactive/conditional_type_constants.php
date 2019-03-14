<?hh // partial
class MyConditionalClassBase {}
class MyConditionalClass extends MyConditionalClassBase {}
class MyRxConditionalClass extends MyConditionalClassBase {}

interface IRxMyTestClass {
  const type TTest = MyRxConditionalClass;
}

abstract class MyTestClass {
  abstract const type TTest as MyConditionalClassBase;
  <<__Rx, __OnlyRxIfImpl(IRxMyTestClass::class)>>
  public static function foo(this::TTest $x): void {
    self::bar($x);
  }

  <<__Rx, __AtMostRxAsArgs>>
  public static function bar(
    <<__OnlyRxIfImpl(MyRxConditionalClass::class)>>MyConditionalClassBase $x,
  ): void {}
}
