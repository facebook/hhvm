<?hh // strict

class TestClass {

  public static function test(): array<shape('foo' => int, ...)> {
    return array();
  }
}

TestClass::test();

echo "Done.";
