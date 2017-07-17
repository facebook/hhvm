<?hh // strict

class TestClass {

  public static function test(): array<shape(...)> {
    return array();
  }
}

TestClass::test();

echo "Done.";
