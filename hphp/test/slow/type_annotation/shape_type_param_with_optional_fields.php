<?hh // strict

class TestClass {

  public static function test(): array<shape(?'foo' => int, 'bar' => int)> {
    return array();
  }
}

TestClass::test();

echo "Done.";
