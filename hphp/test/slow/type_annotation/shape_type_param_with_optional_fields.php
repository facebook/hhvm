<?hh // strict

class TestClass {

  public static function test(): varray<shape(?'foo' => int, 'bar' => int)> {
    return varray[];
  }
}


<<__EntryPoint>>
function main_shape_type_param_with_optional_fields() {
TestClass::test();

echo "Done.";
}
