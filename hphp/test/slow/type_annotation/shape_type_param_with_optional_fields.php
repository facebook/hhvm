<?hh

class TestClass {

  public static function test(): varray<shape(?'foo' => int, 'bar' => int)> {
    return vec[];
  }
}


<<__EntryPoint>>
function main_shape_type_param_with_optional_fields() :mixed{
TestClass::test();

echo "Done.";
}
