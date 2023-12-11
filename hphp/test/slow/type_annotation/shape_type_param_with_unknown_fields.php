<?hh

class TestClass {

  public static function test(): varray<shape(...)> {
    return vec[];
  }
}


<<__EntryPoint>>
function main_shape_type_param_with_unknown_fields() :mixed{
TestClass::test();

echo "Done.";
}
