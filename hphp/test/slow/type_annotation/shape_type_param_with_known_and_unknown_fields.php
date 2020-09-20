<?hh // strict

class TestClass {

  public static function test(): varray<shape('foo' => int, ...)> {
    return varray[];
  }
}


<<__EntryPoint>>
function main_shape_type_param_with_known_and_unknown_fields() {
TestClass::test();

echo "Done.";
}
