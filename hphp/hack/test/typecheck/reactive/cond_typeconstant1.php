<?hh
interface IRxSomeBaseClass {}
class SomeBaseClass {

  public static function someFunction(): void {}
}
class SomeRxChild extends SomeBaseClass implements IRxSomeBaseClass {}

interface IRxTest {
  const type TTest = SomeRxChild;
}
abstract class MyTestClass {
  abstract const type TTest as SomeBaseClass;

  public static function doTheThing(): void {
    $cls = static::getType();
    $cls::someFunction();
  }

  abstract protected static function getType(): classname<this::TTest>;
}
