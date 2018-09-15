<?hh // strict

class TestClass {

  public static function test(): array<shape(?'foo' => int, 'bar' => int)> {
    return array();
  }
}


<<__EntryPoint>>
function main_shape_type_param_with_optional_fields() {
TestClass::test();

echo "Done.";
}
