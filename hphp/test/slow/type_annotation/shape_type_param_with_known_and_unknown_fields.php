<?hh // strict

class TestClass {

  public static function test(): array<shape('foo' => int, ...)> {
    return array();
  }
}


<<__EntryPoint>>
function main_shape_type_param_with_known_and_unknown_fields() {
TestClass::test();

echo "Done.";
}
