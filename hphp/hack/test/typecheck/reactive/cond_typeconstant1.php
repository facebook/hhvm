<?hh // partial

interface IRxSomeBaseClass {}
class SomeBaseClass {
  <<__Rx, __OnlyRxIfImpl(IRxSomeBaseClass::class)>>
  public static function someFunction(): void {}
}
class SomeRxChild extends SomeBaseClass implements IRxSomeBaseClass {}

interface IRxTest {
  const type TTest = SomeRxChild;
}
abstract class MyTestClass {
  abstract const type TTest as SomeBaseClass;
  <<__Rx, __OnlyRxIfImpl(IRxTest::class)>>
  public static function doTheThing(): void {
    $cls = static::getType();
    $cls::someFunction();
  }
  <<__Rx>>
  abstract protected static function getType(): classname<this::TTest>;
}
