<?hh // strict

class TestClass {

  public static function test(): array<shape(...)> {
    return array();
  }
}


<<__EntryPoint>>
function main_shape_type_param_with_unknown_fields() {
TestClass::test();

echo "Done.";
}
