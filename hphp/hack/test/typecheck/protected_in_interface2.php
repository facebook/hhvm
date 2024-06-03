<?hh


interface IInterface {
  protected function foo(): void;
}

abstract class BaseClass implements IInterface {
  public static function doFoo(IInterface $interface): void {
    // This is a typehole because BaseClass does not necessarily
    // have visibility on this protected method.
    $interface->foo();
  }
}

final class FinalClassImplInterface implements IInterface {
  protected function foo(): void {
    echo "Executing foo on FinalClassImplInterface\n";
  }
}

final class FinalClassExtBase extends BaseClass {
  protected function foo(): void {
    echo "Executing foo on FinalClassExtBase\n";
  }
}

<<__EntryPoint>>
function testCausesFatal(): void {
  BaseClass::doFoo(new FinalClassExtBase()); // Runs fine
  BaseClass::doFoo(new FinalClassImplInterface()); // Causes a fatal
}
